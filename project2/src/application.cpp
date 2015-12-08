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

namespace
{
    struct vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
    };
}

Application::Application(SDL_Window *_window, SDL_GLContext& _context)
    : window(_window), context(_context)
{
    SDL_GetWindowSize(window, &width, &height);
    
    // Diffuse color object shader
    std::shared_ptr<Shader>  diffuse_vs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/diffuse_vs.glsl", GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  diffuse_fs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/diffuse_fs.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Program> diffuse = (programs["diffuse"] = std::make_shared<Program>());
    diffuse->Attach(diffuse_vs).Attach(diffuse_fs).Link();

    // Test triangle shader
    std::shared_ptr<Shader>  vs1  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/vs1.glsl", GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  fs1  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/fs1.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Program> test = (programs["test"] = std::make_shared<Program>());
    test->Attach(vs1).Attach(fs1).Link();

    // Lighting shaders
    programs["directionalLight"] = renderer.CompileLightingProgram(GetApplicationResourcesDirectory() + "/content/shaders/directionallight_fs.glsl");

    textures["grass_diffuse"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_diffuse.png", GL_SRGB8_ALPHA8);
    textures["grass_diffuse"]->SetWrapMode(GL_REPEAT);
    textures["grass_normals"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_normals.png", GL_RGB8);
    textures["grass_normals"]->SetWrapMode(GL_REPEAT);

    std::unique_ptr<GLSphere> sphere(new GLSphere(.33f,4));
    testSphere = std::move(sphere);

    // runtime queries for mips
    glGenQueries(2, queries);
    frontBuffer = 0;
    backBuffer = 1;

    // allocate pipeline buffers
    OnResize(width, height);
}

Application::~Application()
{
}

void Application::OnDisplay(float frameTime, float frameDelta)
{
    // Update window title with fps
    std::ostringstream windowTitle;
    windowTitle << std::setprecision(2) << "ENGR 180 Final Project - FPS = " << (1.f / frameDelta);
    SDL_SetWindowTitle(window, windowTitle.str().c_str());

    glGetError();
    glBeginQuery(GL_TIME_ELAPSED, queries[frontBuffer]);

    glm::mat4 P = glm::perspective(75.f * glm::pi<float>() / 180.f, aspect, 0.01f, 10000.f);
    glm::mat4 V = glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,0,1), glm::vec3(0,1,0));

    // --------------- OBJECT DRAWING PHASE ----------------------------
    renderer.BeginGBufferPass();
    {
        glm::mat4 MV = V * glm::translate(glm::vec3(0,0,1.f));

        programs["diffuse"]->Bind();
        glUniformMatrix4fv(programs["diffuse"]->GetUniform("P"),  1, GL_FALSE, glm::value_ptr(P));
        glUniformMatrix4fv(programs["diffuse"]->GetUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV));

        glUniform3f(programs["diffuse"]->GetUniform("DiffuseColor"), 0.0f, 0.0f, 1.0f);
        glUniform1f(programs["diffuse"]->GetUniform("SpecularExponent"), 32.0f);

        testSphere->Draw();

        MV = V * glm::translate(glm::vec3(1.0f,0,1.f));

        programs["diffuse"]->Bind();
        glUniformMatrix4fv(programs["diffuse"]->GetUniform("P"),  1, GL_FALSE, glm::value_ptr(P));
        glUniformMatrix4fv(programs["diffuse"]->GetUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV));

        glUniform3f(programs["diffuse"]->GetUniform("DiffuseColor"), 0.0f, 0.0f, 1.0f);
        glUniform1f(programs["diffuse"]->GetUniform("SpecularExponent"), 32.0f);

        testSphere->Draw();

        MV = V * glm::translate(glm::vec3(-1.f,0,1.f));

        programs["diffuse"]->Bind();
        glUniformMatrix4fv(programs["diffuse"]->GetUniform("P"),  1, GL_FALSE, glm::value_ptr(P));
        glUniformMatrix4fv(programs["diffuse"]->GetUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV));

        glUniform3f(programs["diffuse"]->GetUniform("DiffuseColor"), 0.0f, 0.0f, 1.0f);
        glUniform1f(programs["diffuse"]->GetUniform("SpecularExponent"), 32.0f);

        testSphere->Draw();
    }
    renderer.EndGBufferPass();


    // --------------- LIGHT DRAWING PHASE -----------------------------
    renderer.BeginLightPass();
    {
        // Light direction in view space
        glm::vec4 ld = V * glm::normalize(glm::vec4(cos(frameTime),-0.1,sin(frameTime),0));

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

    // --------------- FINAL RENDER ------------------------------------
    renderer.PerformFinalRender();

    glEndQuery(GL_TIME_ELAPSED);
    std::swap(frontBuffer, backBuffer);

    // Get previous query result
    GLuint elapsed;
    glGetQueryObjectuiv(queries[frontBuffer], GL_QUERY_RESULT, &elapsed);
    std::cout << "[INFO] Scene Render Took: " << float(elapsed)/1000.f << " us" << std::endl;
}

void Application::OnResize(GLint _width, GLint _height)
{
    std::cout << "[INFO] window resized " << _width << ", " << _height << std::endl;

    width  = _width;
    height = _height;
    aspect = float(width) / float(height);

    // Reallocate framebuffer storage
    renderer.ResizeRenderPipeline(width,height);
}

void Application::OnEvent(SDL_Event event)
{
    
}
