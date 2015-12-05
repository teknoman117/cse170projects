//
//  Application.cpp
//  Application
//
//  Created by Nathaniel Lewis on 11/21/14.
//  Copyright (c) 2014 HoodooNet. All rights reserved.
//

#include <sstream>
#include <iomanip>

#include <project2/application.hpp>
#include <project2/directories.hpp>
#include <project2/shader.hpp>

#include <glm/glm.hpp>

float randomColor()
{
    return float(rand()) / float(RAND_MAX);
}

struct vertex
{
    glm::vec3 position;
    glm::vec4 color;
};

Application::Application(SDL_Window *_window, SDL_GLContext& _context)
    : window(_window), context(_context)
{
    SDL_GetWindowSize(window, &width, &height);
    
    std::shared_ptr<Shader>  vs1  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/vs1.glsl", GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  fs1  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/fs1.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Program> test = (programs["test"] = std::make_shared<Program>());
    test->Attach(vs1)
         .Attach(fs1)
         .Link();

    // Create a triangle on the GPU
    const struct vertex triangle[] = 
    {
        {glm::vec3(-0.5f,-0.5,0.f), glm::vec4(1,0,0,1)},
        {glm::vec3( 0.0f, 0.5,0.f), glm::vec4(0,1,0,1)},
        {glm::vec3( 0.5f,-0.5,0.f), glm::vec4(0,0,1,1)},
    };

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

    // Prepare framebuffer
    glGetError();
    //glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    programs["test"]->Bind();

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::OnResize(GLint _width, GLint _height)
{
    width  = _width;
    height = _height;
    aspect = float(height) / float(width);
    
    glViewport(0, 0 , width, height);
}

void Application::OnEvent(SDL_Event event)
{
    
}
