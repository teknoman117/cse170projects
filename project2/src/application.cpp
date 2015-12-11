//
//  Application.cpp
//  Application
//
//  Created by Nathaniel Lewis on 11/21/14.
//  Copyright (c) 2014 HoodooNet. All rights reserved.
//

#include <sstream>
#include <iomanip>
#include <cmath>

#include <project2/application.hpp>
#include <project2/directories.hpp>
#include <project2/shader.hpp>

Application::Application(SDL_Window *_window, SDL_GLContext& _context)
    : window(_window), context(_context)
{
    SDL_GetWindowSize(window, &width, &height);
    
    // Diffuse color object shader
    std::shared_ptr<Shader>  diffuse_vs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/diffuse_vs.glsl", GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  diffuse_fs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/diffuse_fs.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Program> diffuse = (programs["diffuse"] = std::make_shared<Program>());
    diffuse->Attach(diffuse_vs).Attach(diffuse_fs).Link();

    // Lighting shaders
    programs["directionalLight"] = renderer.CompileLightingProgram(GetApplicationResourcesDirectory() + "/content/shaders/directionallight_fs.glsl");

    // Grass texture
    textures["grass_diffuse"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_diffuse.png", GL_SRGB8_ALPHA8);
    textures["grass_diffuse"]->SetWrapMode(GL_REPEAT);
    textures["grass_normals"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_normals.png", GL_RGB8);
    textures["grass_normals"]->SetWrapMode(GL_REPEAT);

    // Test objects
    std::unique_ptr<GLSphere> sphere(new GLSphere(.33f,4));
    testSphere = std::move(sphere);

    std::unique_ptr<ChunkedTerrain> t(
        new ChunkedTerrain(GetApplicationResourcesDirectory() + "/content/terrain.raw", 
                           1024, 
                           1024, 
                           glm::dvec2((1.0/10800.0) * (glm::pi<double>()/180.0), (1.0/10800.0) * (glm::pi<double>()/180.0)),
                           glm::dvec2(glm::pi<double>()*(-121.3697/180.0), glm::pi<double>()*(40.5848/180.0))
        )
    );
    chunkedTerrain = std::move(t);

    // Initialize view position
    glm::vec3 p = chunkedTerrain->GetLocationOfCoordinate(glm::dvec2(
        glm::pi<double>()*(-121.3697/180.0) + (512.5/10800.0)*(glm::pi<double>()/180.0), 
        glm::pi<double>()*(40.5848/180.0) - (512.5/10800.0)*(glm::pi<double>()/180.0))
    );
    viewPosition = glm::vec4(p.x, 0, p.z, 0);
    viewRotation = glm::vec2(0,glm::pi<float>());

    // allocate pipeline buffers
    OnResize(width, height);
}

Application::~Application()
{
}

bool Application::OnDisplay(float frameTime, float frameDelta)
{
    // Update window title with fps
    std::ostringstream windowTitle;
    windowTitle << std::setprecision(2) << "ENGR 180 Final Project - FPS = " << (1.f / frameDelta);
    SDL_SetWindowTitle(window, windowTitle.str().c_str());

    // Compute base movement vector
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    glm::vec4 moveDirection;

    if(state[SDL_SCANCODE_W])
        moveDirection.z = -1.f;
    else if(state[SDL_SCANCODE_S])
        moveDirection.z = 1.f;

    if(state[SDL_SCANCODE_A])
        moveDirection.x = -1.0f;
    else if(state[SDL_SCANCODE_D])
        moveDirection.x = 1.f;

    // jogging or sprinting
    moveDirection *= (state[SDL_SCANCODE_LSHIFT] ? 6.7056f : 2.68224f);

    // Update position in the current view direction
    viewPosition += (glm::rotate(viewRotation.y, glm::vec3(0.f,1.f,0.f)) * moveDirection) * frameDelta;
    float h = chunkedTerrain->GetElevationAt(glm::vec3(viewPosition.x, 0, viewPosition.z)) + 1.8f;

    // Compute camera parameters
    glm::quat r      = glm::angleAxis(viewRotation.y, glm::vec3(0,1,0)) * glm::angleAxis(viewRotation.x, glm::vec3(1,0,0));
    glm::vec3 eye    = glm::vec3(viewPosition.x, h, viewPosition.z);
    glm::vec3 center = eye + r*glm::vec3(0,0,-1);
    glm::vec3 up     = glm::vec3(0,1,0);
    const float yFov = 75.f * glm::pi<float>() / 180.f;

    // Build frustum for culling the terrain
    Frustum cameraFrustum;
    cameraFrustum.SetCameraProperties(100.f * glm::pi<float>() / 180.f, aspect, 0.01f, 20000.f);
    cameraFrustum.SetCameraOrientation(eye, center, up);

    // Compute OpenGL matrices
    glm::mat4 P = glm::perspective(yFov, aspect, 0.01f, 20000.f);
    glm::mat4 V = glm::lookAt(eye, center, up);

    // Start rendering
    renderer.BeginRendering();
    {
        // --------------- OBJECT DRAWING PHASE ----------------------------
        
        renderer.BeginGBufferPass();
        {
            glm::mat4 MV = V * glm::translate(glm::vec3(0.f,0.f,0.f));

            programs["diffuse"]->Bind();
            glUniformMatrix4fv(programs["diffuse"]->GetUniform("P"),  1, GL_FALSE, glm::value_ptr(P));
            glUniformMatrix4fv(programs["diffuse"]->GetUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV));

            glUniform3f(programs["diffuse"]->GetUniform("DiffuseColor"), float(0x7c)/float(0xff), float(0xfc)/float(0xff), 0);
            glUniform1f(programs["diffuse"]->GetUniform("SpecularExponent"), 32.0f);

            //testSphere->Draw();
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
            glFrontFace(GL_CW);
            
            chunkedTerrain->Draw(cameraFrustum);

            glFrontFace(GL_CCW);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        renderer.EndGBufferPass();


        // --------------- LIGHT DRAWING PHASE -----------------------------
        renderer.BeginLightPass();
        {
            // Light direction in view space
            glm::vec4 ld = V * glm::normalize(glm::vec4(cos(frameTime),-0.5,sin(frameTime),0));

            // Directional Lights
            programs["directionalLight"]->Bind();

            const glm::mat4 nullTransform;
            glUniformMatrix4fv(programs["directionalLight"]->GetUniform("MVP"), 1, GL_FALSE, glm::value_ptr(nullTransform));

            glUniform3f(programs["directionalLight"]->GetUniform("LightColor"),     1.0f, 1.0f, 1.0f);
            glUniform3f(programs["directionalLight"]->GetUniform("LightIntensity"), 0.1f, 0.7f, 1.0f);
            glUniform3f(programs["directionalLight"]->GetUniform("LightDirection"), ld.x, ld.y, ld.z);

            glUniform1i(programs["directionalLight"]->GetUniform("gBufferDiffuseSpecular"), 0);
            glUniform1i(programs["directionalLight"]->GetUniform("gBufferNormals"), 1);
            glUniform1i(programs["directionalLight"]->GetUniform("gBufferPosition"), 2);

            renderer.BeginGlobalLightRender();
            directionalLight.Draw();
            renderer.EndGlobalLightRender();
        }
        renderer.EndLightPass();
    }
    renderer.EndRendering();

    // Continue application
    return true;
}

bool Application::OnResize(GLint _width, GLint _height)
{
    std::cout << "[INFO] window resized " << _width << ", " << _height << std::endl;

    width  = _width;
    height = _height;
    aspect = float(width) / float(height);

    // Reallocate framebuffer storage
    renderer.ResizeRenderPipeline(width,height);

    return true;
}

bool Application::OnEvent(SDL_Event event)
{
    if(event.type == SDL_KEYDOWN)
    {
        if(event.key.keysym.sym == SDLK_ESCAPE)
        {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        else if(event.key.keysym.sym == SDLK_SPACE)
        {
            wireframe = !wireframe;
        }
    }

    else if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
    {
        if(SDL_GetRelativeMouseMode() == SDL_FALSE)
        {
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        else
        {
            // Do something cool otherwise
        }
    }

    // If the mouse is captured and it moves, move the camera
    else if(event.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE)
    {
        viewRotation.y += -0.025f * float(event.motion.xrel) * glm::pi<float>() / 180.f;
        viewRotation.x += -0.025f * float(event.motion.yrel) * glm::pi<float>() / 180.f;

        viewRotation.x = glm::clamp(viewRotation.x, -85.0f * glm::pi<float>() / 180.f, 85.0f * glm::pi<float>() / 180.f);
    }

    return true;
}
