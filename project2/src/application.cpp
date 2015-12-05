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

float randomColor()
{
    return float(rand()) / float(RAND_MAX);
}

Application::Application(SDL_Window *_window, SDL_GLContext& _context)
    : window(_window), context(_context)
{
    SDL_GetWindowSize(window, &width, &height);
    
    std::shared_ptr<Shader> vs1 = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/vs1.glsl", GL_VERTEX_SHADER);
    std::shared_ptr<Shader> fs1 = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/fs1.glsl", GL_FRAGMENT_SHADER);
    
    std::shared_ptr<Program> test = std::make_shared<Program>();
    test->Attach(vs1);
    test->Attach(fs1);
    test->Link();
    programs["test"] = test;
}

Application::~Application()
{
}

void Application::OnDisplay(float frameTime, float frameDelta)
{
    std::ostringstream windowTitle;
   windowTitle << std::setprecision(2) << "ENGR 180 Final Project - FPS = " << (1.f / frameDelta);
    SDL_SetWindowTitle(window, windowTitle.str().c_str());
    
    glClearColor(randomColor(), randomColor(), randomColor(), 1.0f);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
