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
#include <project2/polyline.hpp>
#include <project2/curve_eval.hpp>

Application::Application(SDL_Window *_window, SDL_GLContext& _context, const Application::Options& options)
    : window(_window), context(_context)
{
    SDL_GetWindowSize(window, &camera.width, &camera.height);
    
    // Shaders
    std::shared_ptr<Shader>  gbuffer_vs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/gbuffer_vs.glsl", GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  diffuse_fs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/diffuse_fs.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Shader>  skybox_vs  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/skybox_vs.glsl",  GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  skybox_fs  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/skybox_fs.glsl",  GL_FRAGMENT_SHADER);

    std::shared_ptr<Shader>  terrain_vs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/terrain_vs.glsl", GL_VERTEX_SHADER );
    std::shared_ptr<Shader>  terrain_fs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/terrain_fs.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Shader>  terrain_tcs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/terrain_tcs.glsl", GL_TESS_CONTROL_SHADER  );
    std::shared_ptr<Shader>  terrain_tes = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/terrain_tes.glsl", GL_TESS_EVALUATION_SHADER);
 
    programs["diffuse"] = std::make_shared<Program>();
    programs["terrain"] = std::make_shared<Program>();
    programs["skybox"] = std::make_shared<Program>();

    programs["diffuse"]->Attach(gbuffer_vs).Attach(diffuse_fs).Link();
    programs["terrain"]->Attach(terrain_vs).Attach(terrain_fs).Attach(terrain_tcs).Attach(terrain_tes).Link();
    programs["skybox"]->Attach(skybox_vs).Attach(skybox_fs).Link();

    // Lighting shaders
    programs["directionalLight"] = renderer.CompileLightingProgram(GetApplicationResourcesDirectory() + "/content/shaders/directionallight_fs.glsl");

    // Grass texture
    textures["grass_diffuse"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_diffuse.png", GL_SRGB8_ALPHA8);
    textures["grass_diffuse"]->SetWrapMode(GL_REPEAT);
    textures["grass_normals"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_normals.png", GL_RGB8);
    textures["grass_normals"]->SetWrapMode(GL_REPEAT);
    textures["cliff_diffuse"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/cliff_diffuse.png", GL_SRGB8_ALPHA8);
    textures["cliff_diffuse"]->SetWrapMode(GL_REPEAT);
    textures["cliff_normals"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/cliff_normal.png", GL_RGB8);
    textures["cliff_normals"]->SetWrapMode(GL_REPEAT);

    std::unique_ptr<Cubemap> cubemap(new Cubemap(GetApplicationResourcesDirectory() + "/content/skybox/skybox"));
    skyboxTexture = std::move(cubemap);

    // Load terrain
    std::unique_ptr<ChunkedTerrain> t(
        new ChunkedTerrain(options.rasterFilename, options.rasterSize,  options.chunkSize, options.resolution, options.corner)
    );
    chunkedTerrain = std::move(t);

    // Initialize view position
    glm::vec3 p = chunkedTerrain->GetLocationOfWGS84Coordinate(options.starting);
    camera.position = glm::vec3(p.x, chunkedTerrain->GetElevationAt(p), p.z);
    camera.rotation = glm::vec2(0,glm::pi<float>());

    timeOfDay = 2;
    animationSpeed = 10;

    // If a camera path was provided
    if(options.pathFilename != "")
    {
        std::vector<glm::dvec2> coordinates;
        std::vector<glm::vec3>  vertices;

        size_t count = DecodePolylineFromFile(coordinates, options.pathFilename);

        for(std::vector<glm::dvec2>::iterator coord = coordinates.begin(); coord != coordinates.end() /*&& coord != coordinates.begin()+100*/; coord++)
        {
            glm::vec3 p = chunkedTerrain->GetLocationOfWGS84Coordinate(*coord * (glm::pi<double>() / 180.0));
            p.y = chunkedTerrain->GetElevationAt(p) + 5.f;
            vertices.push_back(p);
        }

        std::cout << "processed " << vertices.size() << " vertices" << std::endl;

        cameraPath = std::move(vertices);
        //evaluate_bezier(vertices.size()*6, cameraPath, vertices);

        std::unique_ptr<GLLine> line(new GLLine(cameraPath));
        pathLine = std::move(line);
    }

    // allocate pipeline buffers
    OnResize(camera.width, camera.height);
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

    if(state[SDL_SCANCODE_HOME])
        animationSpeed += 50.0f * frameDelta;
    else if(state[SDL_SCANCODE_END])
        animationSpeed = glm::max<float>(0.0f, animationSpeed - 50.f * frameDelta);

    
    // spline following camera
    if(following)
    {
        animationTime += frameDelta * (animationSpeed / distance(cameraPath[currentPosition], cameraPath[currentPosition+1]));

        // Handle next frame / end logic
        if(animationTime > 1.0)
        {
            int overage = floor(animationTime);
            currentPosition += overage;
            animationTime -= overage;

            if(currentPosition >= cameraPath.size()-1)
            {
                following = false;
                animationTime = 0.f;
                currentPosition = 0;
            }
        }
        
        // If we are still following (didn't bail)
        if(following)
        {
            camera.position = mix(cameraPath[currentPosition], cameraPath[currentPosition+1], animationTime);
        }
    }

    // jogging or sprinting
    else if(!flying)
    {
        moveDirection *= (state[SDL_SCANCODE_LSHIFT] ? 6.7056f : 2.68224f);

        // Update position in the current view direction
        camera.position += (glm::angleAxis(camera.rotation.y, glm::vec3(0.f,1.f,0.f)) * moveDirection) * frameDelta;
        camera.position.y = chunkedTerrain->GetElevationAt(glm::vec3(camera.position.x, 0, camera.position.z)) + 1.8f;
    }

    // flying camera
    else
    {
        moveDirection *= (state[SDL_SCANCODE_LSHIFT] ? 240.0f : 30.f);

        glm::vec3 forward = camera.RotationAsQuaternion()*glm::vec3(0,0,1);
        glm::vec3 right   = cross(forward, camera.up);    
        glm::vec3 lup     = cross(right, forward);

        camera.position += -moveDirection.x*right*frameDelta + 
                            moveDirection.y*lup*frameDelta +
                            moveDirection.z*forward*frameDelta;

        float h = chunkedTerrain->GetElevationAt(camera.position);
        if(camera.position.y < (h + 1.8f) && h != std::numeric_limits<float>::infinity()) 
            camera.position.y = h + 1.8f;
    }

    // Update camera uniform buffer
    camera.Update();

    // Start rendering
    renderer.BeginRendering();
    {
        // --------------- OBJECT DRAWING PHASE ----------------------------
        renderer.BeginGBufferPass();
        {
            // Skybox
            skyboxTexture->Bind(GL_TEXTURE0);
            programs["skybox"]->Bind();
            glUniform1i(programs["skybox"]->GetUniform("uTexture"), 0);

            glDepthRangef(0.9999999f,1);
            directionalLight.Draw();
            glDepthRangef(0,1);

            // Terrain
            textures["grass_diffuse"]->Bind(GL_TEXTURE1);
            textures["cliff_diffuse"]->Bind(GL_TEXTURE2);
            textures["grass_normals"]->Bind(GL_TEXTURE3);
            textures["cliff_normals"]->Bind(GL_TEXTURE4);

            programs["terrain"]->Bind();

            glUniform1i(programs["terrain"]->GetUniform("horizontalTexture"),        1);
            glUniform1i(programs["terrain"]->GetUniform("verticalTexture"),          2);
            glUniform1i(programs["terrain"]->GetUniform("horizontalTextureNormals"), 3);
            glUniform1i(programs["terrain"]->GetUniform("verticalTextureNormals"),   4);

            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
            chunkedTerrain->Draw(programs["terrain"]);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // Path line
            if(pathLine && following)
            {
                programs["diffuse"]->Bind();
                glUniform3f(programs["diffuse"]->GetUniform("DiffuseColor"), 10.f, 10.f, 10.f);
                glUniform1f(programs["diffuse"]->GetUniform("SpecularExponent"), 32.f);
                glm::mat4 nulltransform;
                glUniformMatrix4fv(programs["diffuse"]->GetUniform("M"), 1, GL_FALSE, glm::value_ptr(nulltransform));
                pathLine->Draw();
            }
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

            glUniform3f (programs["directionalLight"]->GetUniform("LightColor"),     1.0f, 1.0f, 1.0f);
            glUniform3f (programs["directionalLight"]->GetUniform("LightIntensity"), 0.2f, 6.0f, 0.0f);
            glUniform3fv(programs["directionalLight"]->GetUniform("LightDirection"), 1, glm::value_ptr(ld));

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

    camera.width  = _width;
    camera.height = _height;
    camera.aspect = float(_width) / float(_height);
    camera.viewport = glm::vec4(0, 0, _width, _height);

    // Reallocate framebuffer storage
    renderer.ResizeRenderPipeline(camera.width, camera.height);

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
        else if(event.key.keysym.sym == SDLK_i)
        {
            following = false;
            flying =  false;
        }
        else if(event.key.keysym.sym == SDLK_o)
        {
            following = false;
            flying = true;
        }
        else if(event.key.keysym.sym == SDLK_p && cameraPath.size())
        {
            following = true;
            currentPosition = 0;
            animationTime = 0.f;
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
        camera.rotation.y += -0.025f * float(event.motion.xrel) * glm::pi<float>() / 180.f;
        camera.rotation.x += -0.025f * float(event.motion.yrel) * glm::pi<float>() / 180.f;

        camera.rotation.x = glm::clamp(camera.rotation.x, -85.0f * glm::pi<float>() / 180.f, 85.0f * glm::pi<float>() / 180.f);
    }

    return true;
}
