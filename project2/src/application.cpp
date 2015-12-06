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
    std::shared_ptr<Shader>  vs1  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/vs1.glsl", GL_VERTEX_SHADER  );
    std::shared_ptr<Shader>  fs1  = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/fs1.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Program> test = (programs["test"] = std::make_shared<Program>());
    test->Attach(vs1)
         .Attach(fs1)
         .Link();

    // Fullscreen quad shader
    std::shared_ptr<Shader>  tonemap_vs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/tonemap_vs.glsl", GL_VERTEX_SHADER);
    std::shared_ptr<Shader>  tonemap_fs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/tonemap_fs.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Program> tonemap    = (programs["tonemap"] = std::make_shared<Program>());
    tonemap->Attach(tonemap_vs)
            .Attach(tonemap_fs)
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



    // setup a frame buffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    textures["hdr-color"] = std::make_shared<Texture>(width, height, 1, GL_RGBA16F);
    textures["hdr-depth"] = std::make_shared<Texture>(width, height, 1, GL_DEPTH24_STENCIL8);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures["hdr-color"]->GetHandle(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textures["hdr-depth"]->GetHandle(), 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "[FATAL] Unable to construct HDR framebuffer" << std::endl;
        SDL_Quit();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    programs["test"]->Bind();

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    programs["tonemap"]->Bind();
    textures["hdr-color"]->Bind(GL_TEXTURE_2D);

    const float gammaCorrectionFactor = 1.f/ 2.2f;
    glUniform1f(programs["tonemap"]->GetUniform("correctionFactor"), gammaCorrectionFactor);

    fullscreenQuad.Draw();
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
