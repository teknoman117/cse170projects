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
#include <limits>

#include <project2/application.hpp>
#include <project2/directories.hpp>
#include <project2/shader.hpp>

Application::Application(SDL_Window *_window, SDL_GLContext& _context)
    : window(_window), context(_context)
{
    SDL_GetWindowSize(window, &width, &height);
    
    // Shaders
    std::shared_ptr<Shader>  gbuffer_vs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/gbuffer_vs.glsl", GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  diffuse_fs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/diffuse_fs.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Shader>  terrain_vs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/terrain_vs.glsl", GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  terrain_fs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/terrain_fs.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Shader>  skybox_vs  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/skybox_vs.glsl",  GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  skybox_fs  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/skybox_fs.glsl",  GL_FRAGMENT_SHADER);
 
    programs["diffuse"] = std::make_shared<Program>();
    programs["terrain"] = std::make_shared<Program>();
    programs["skybox"] = std::make_shared<Program>();

    programs["diffuse"]->Attach(gbuffer_vs).Attach(diffuse_fs).Link();
    programs["terrain"]->Attach(terrain_vs).Attach(terrain_fs).Link();
    programs["skybox"]->Attach(skybox_vs).Attach(skybox_fs).Link();

    // Lighting shaders
    programs["directionalLight"] = renderer.CompileLightingProgram(GetApplicationResourcesDirectory() + "/content/shaders/directionallight_fs.glsl");

    // Grass texture
    textures["grass_diffuse"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_diffuse.png", GL_SRGB8_ALPHA8);
    textures["grass_diffuse"]->SetWrapMode(GL_REPEAT);
    textures["grass_normals"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_normals.png", GL_RGB8);
    textures["grass_normals"]->SetWrapMode(GL_REPEAT);
    textures["cliff_diffuse"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/cliff_07_diffuse.png", GL_SRGB8_ALPHA8);
    textures["cliff_diffuse"]->SetWrapMode(GL_REPEAT);
    textures["cliff_normals"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/cliff_07_normal.png", GL_RGB8);
    textures["cliff_normals"]->SetWrapMode(GL_REPEAT);

    std::unique_ptr<Cubemap> cubemap(new Cubemap(GetApplicationResourcesDirectory() + "/content/skybox/skybox"));
    skyboxTexture = std::move(cubemap);

    std::unique_ptr<GLCube> cube(new GLCube());
    skybox = std::move(cube);

    /*std::unique_ptr<ChunkedTerrain> t(
        new ChunkedTerrain(GetApplicationResourcesDirectory() + "/content/terrain_2705_2705.raw", 
                           2705, 
                           2705, 
                           glm::dvec2((1.0/10800.0) * (glm::pi<double>()/180.0), (1.0/10800.0) * (glm::pi<double>()/180.0)),
                           glm::dvec2(glm::pi<double>()*(-121.5/180.0), glm::pi<double>()*(40.65/180.0))
        )
    );
    chunkedTerrain = std::move(t);

    // Initialize view position
    glm::vec3 p = chunkedTerrain->GetLocationOfCoordinate(glm::dvec2(
        glm::pi<double>()*(-121.4044/180.0), 
        glm::pi<double>()*(40.5824/180.0))
    );*/

    std::unique_ptr<ChunkedTerrain> t(
        new ChunkedTerrain(GetApplicationResourcesDirectory() + "/content/bigazzfile.raw", 
                           5009, 
                           5009, 
                           glm::dvec2((1.0/10800.0) * (glm::pi<double>()/180.0), (1.0/10800.0) * (glm::pi<double>()/180.0)),
                           glm::dvec2(glm::pi<double>()*(-122.0/180.0), glm::pi<double>()*(41.0/180.0))
        )
    );
    chunkedTerrain = std::move(t);

    // Initialize view position
    glm::vec3 p = chunkedTerrain->GetLocationOfCoordinate(glm::dvec2(
        glm::pi<double>()*(-121.9/180.0), 
        glm::pi<double>()*(40.9/180.0))
    );

    viewPosition = glm::vec3(p.x, chunkedTerrain->GetElevationAt(p), p.z);
    viewRotation = glm::vec2(0,glm::pi<float>());

    timeOfDay = 2;

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

    // Compute view stuff
    glm::quat r   = glm::angleAxis(viewRotation.y, glm::vec3(0,1,0)) * glm::angleAxis(viewRotation.x, glm::vec3(1,0,0));
    glm::vec3 up  = glm::vec3(0,1,0);

    // Compute base movement vector
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    glm::vec3 moveDirection;

    if(state[SDL_SCANCODE_W])
        moveDirection.z = -1.f;
    else if(state[SDL_SCANCODE_S])
        moveDirection.z = 1.f;

    if(state[SDL_SCANCODE_A])
        moveDirection.x = -1.0f;
    else if(state[SDL_SCANCODE_D])
        moveDirection.x = 1.f;

    if(state[SDL_SCANCODE_R])
        moveDirection.y = 1.0f;
    else if(state[SDL_SCANCODE_F])
        moveDirection.y = -1.f;

    if(state[SDL_SCANCODE_PAGEUP])
        timeOfDay += 1.0 * frameDelta;
    else if(state[SDL_SCANCODE_PAGEDOWN])
        timeOfDay -= 1.0 * frameDelta;

    // jogging or sprinting
    if(!flying)
    {
        moveDirection *= (state[SDL_SCANCODE_LSHIFT] ? 6.7056f : 2.68224f);

        // Update position in the current view direction
        viewPosition += (glm::angleAxis(viewRotation.y, glm::vec3(0.f,1.f,0.f)) * moveDirection) * frameDelta;
        viewPosition.y = chunkedTerrain->GetElevationAt(glm::vec3(viewPosition.x, 0, viewPosition.z)) + 1.8f;
    }

    // flying camera
    else
    {
        moveDirection *= (state[SDL_SCANCODE_LSHIFT] ? 240.0f : 30.f);

        glm::vec3 forward = r*glm::vec3(0,0,1);
        glm::vec3 right   = cross(forward, up);    
        glm::vec3 lup     = cross(right, forward);

        viewPosition += moveDirection.z*forward*frameDelta + 
                        -moveDirection.x*right*frameDelta + 
                        moveDirection.y*lup*frameDelta;

        float h = chunkedTerrain->GetElevationAt(viewPosition);
        if(viewPosition.y < (h + 1.8f) && h != std::numeric_limits<float>::infinity()) 
            viewPosition.y = h + 1.8f;
    }

    // Compute camera parameters
    glm::vec3 center = viewPosition + r*glm::vec3(0,0,-1);
    const float yFov = 75.f * glm::pi<float>() / 180.f;

    // Build frustum for culling the terrain
    Frustum cameraFrustum;
    cameraFrustum.SetCameraProperties(100.f * glm::pi<float>() / 180.f, aspect, 0.1f, 70000.f);
    cameraFrustum.SetCameraOrientation(viewPosition, center, up);

    // Compute OpenGL matrices
    glm::mat4 P = glm::perspective(yFov, aspect, 0.01f, 20000.f);
    glm::mat4 V = glm::lookAt(viewPosition, center, up);
    glm::mat4 VP = P*V;

    // Start rendering
    renderer.BeginRendering();
    {
        // --------------- OBJECT DRAWING PHASE ----------------------------
        renderer.BeginGBufferPass();
        {
            // Skybox
            skyboxTexture->Bind(GL_TEXTURE0);
            programs["skybox"]->Bind();

            glUniformMatrix4fv(programs["skybox"]->GetUniform("uProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(P));
            glUniformMatrix4fv(programs["skybox"]->GetUniform("uWorldToCameraMatrix"), 1, GL_FALSE, glm::value_ptr(V));
            glUniform1i(programs["skybox"]->GetUniform("uTexture"), 0);

            glDepthRangef(0.9999999f,1);
            directionalLight.Draw();
            glDepthRangef(0,1);

            // Terrain
            glm::mat4 M = glm::translate(glm::vec3(0.f,0.f,0.f));

            textures["grass_diffuse"]->Bind(GL_TEXTURE1);
            textures["cliff_diffuse"]->Bind(GL_TEXTURE2);
            textures["grass_normals"]->Bind(GL_TEXTURE3);
            textures["cliff_normals"]->Bind(GL_TEXTURE4);

            programs["terrain"]->Bind();
            glUniformMatrix4fv(programs["terrain"]->GetUniform("M"),  1, GL_FALSE, glm::value_ptr(M));
            glUniformMatrix4fv(programs["terrain"]->GetUniform("VP"), 1, GL_FALSE, glm::value_ptr(VP));

            glUniform3f(programs["terrain"]->GetUniform("DiffuseColor"), float(0x7c)/float(0xff), float(0xfc)/float(0xff), 0);
            glUniform1f(programs["terrain"]->GetUniform("SpecularExponent"),         1.0f);
            glUniform1i(programs["terrain"]->GetUniform("horizontalTexture"),        1);
            glUniform1i(programs["terrain"]->GetUniform("verticalTexture"),          2);
            glUniform1i(programs["terrain"]->GetUniform("horizontalTextureNormals"), 3);
            glUniform1i(programs["terrain"]->GetUniform("verticalTextureNormals"),   4);
            glUniform3fv(programs["terrain"]->GetUniform("CameraPosition"),  1, glm::value_ptr(viewPosition));

            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
            glFrontFace(GL_CW);
            
            chunkedTerrain->Draw(cameraFrustum, programs["terrain"]);

            glFrontFace(GL_CCW);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        renderer.EndGBufferPass();


        // --------------- LIGHT DRAWING PHASE -----------------------------
        renderer.BeginLightPass();
        {
            // Light direction in view space
            // Compute the position of a light
            glm::vec3 ld = glm::angleAxis(-glm::pi<float>()*35.0f / 180.0f, glm::vec3(1,0,0)) * 
                           glm::vec3(sin(timeOfDay),
                                     cos(timeOfDay),
                                     0);

            // Directional Lights
            programs["directionalLight"]->Bind();

            const glm::mat4 nullTransform;
            glUniformMatrix4fv(programs["directionalLight"]->GetUniform("MVP"), 1, GL_FALSE, glm::value_ptr(nullTransform));

            glUniform3f(programs["directionalLight"]->GetUniform("LightColor"),      1.0f, 1.0f, 1.0f);
            glUniform3f(programs["directionalLight"]->GetUniform("LightIntensity"),  0.2f, 6.0f, 0.0f);
            glUniform3fv(programs["directionalLight"]->GetUniform("LightDirection"), 1, glm::value_ptr(ld));
            glUniform3fv(programs["directionalLight"]->GetUniform("CameraPosition"),  1, glm::value_ptr(viewPosition));

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
        else if(event.key.keysym.sym == SDLK_p)
        {
            flying = !flying;
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
