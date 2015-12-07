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

#include <glm/glm.hpp>

namespace
{
    struct vertex
    {
        glm::vec3 position;
        glm::vec4 color;
    };
}

Application::Application(SDL_Window *_window, SDL_GLContext& _context)
    : window(_window), context(_context)
{
    SDL_GetWindowSize(window, &width, &height);
    
    // Test triangle shader
    std::shared_ptr<Shader>  vs1  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/vs1.glsl", GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  fs1  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/fs1.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Program> test = (programs["test"] = std::make_shared<Program>());
    test->Attach(vs1)
         .Attach(fs1)
         .Link();

    textures["grass_diffuse"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_diffuse.png", GL_SRGB8_ALPHA8);
    textures["grass_diffuse"]->SetWrapMode(GL_REPEAT);
    textures["grass_normals"] = std::make_shared<Texture>(GetApplicationResourcesDirectory() + "/content/textures/grass_normals.png", GL_RGB8);
    textures["grass_normals"]->SetWrapMode(GL_REPEAT);

    // Create a triangle on the GPU
    const struct vertex triangle[] = 
    {
        {glm::vec3(-0.5f,-0.5,0.f), glm::vec4(15,0,0,1)},
        {glm::vec3( 0.0f, 0.5,0.f), glm::vec4(0,15,0,1)},
        {glm::vec3( 0.5f,-0.5,0.f), glm::vec4(0,0,15,1)},
    };

    // Setup a test object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

    glEnableVertexAttribArray ( 0 );
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (GLvoid *) offsetof(struct vertex, position));

    glEnableVertexAttribArray ( 1 );
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glVertexAttribPointer ( 1, 4, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (GLvoid *) offsetof(struct vertex, color));

    glBindVertexArray(0);

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

    // --------------- OBJECT DRAWING PHASE ----------------------------
    renderer.BeginGBufferPass();
    
    programs["test"]->Bind();
    
    glUniform1f(programs["test"]->GetUniform("amplification"), 0.1f + (1.f + sinf(frameTime*5.f))*1.5f);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    renderer.EndGBufferPass();

    // --------------- LIGHT DRAWING PHASE -----------------------------
    renderer.BeginLightPass();

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
    aspect = float(height) / float(width);

    // Reallocate framebuffer storage
    renderer.ResizeRenderPipeline(width,height);
}

void Application::OnEvent(SDL_Event event)
{
    
}
