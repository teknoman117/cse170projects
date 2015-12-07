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

    size_t numberOfMipLevels(size_t w, size_t h)
    {
        size_t maxDim = w > h ? w : h;
        return (size_t) floorf(::log2f(float(maxDim))) + 1;
    }

    // Function given an image computes the size of the luminosity image (1->2)x(1->2)
    void luminositySize(GLuint w, GLuint h, GLuint& lw, GLuint& lh)
    {
        // find the base decomposition level
        GLuint maxDimension = (w>h)?w:h;

        // 65536 texture is huge, move by 4 because thats what our shader does
        for(GLuint i = 0; i <= 16; i+=2)
        {
            // is this the luminosity level
            if((maxDimension / (1 << i)) <= 256)
            {
                lw = (GLuint) ceilf(float(w / (1<<i)) / 128.f);
                lh = (GLuint) ceilf(float(h / (1<<i)) / 128.f);

                return;
            }
        }

        lw=0;
        lh=0;
    }
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

    // Fullscreen quad shader
    std::shared_ptr<Shader>  tonemap_vs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/tonemap_vs.glsl", GL_VERTEX_SHADER);
    std::shared_ptr<Shader>  tonemap_fs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/tonemap_fs.glsl", GL_FRAGMENT_SHADER);
    std::shared_ptr<Program> tonemap    = (programs["tonemap"] = std::make_shared<Program>());
    tonemap->Attach(tonemap_vs)
            .Attach(tonemap_fs)
            .Link();

    // Averaging shaders
    std::shared_ptr<Program> downsample4x = (programs["downsample4x"] = std::make_shared<Program>());
    downsample4x->Attach(GetApplicationResourcesDirectory() + "/content/shaders/downsample4x.glsl", GL_COMPUTE_SHADER)
                 .Link();

    std::shared_ptr<Program> downsample4x_reduce = (programs["downsample4x_reduce"] = std::make_shared<Program>());
    downsample4x_reduce->Attach(GetApplicationResourcesDirectory() + "/content/shaders/downsample4x_reduce.glsl", GL_COMPUTE_SHADER)
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

    // Bind the HDR framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0 , width, height);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    programs["test"]->Bind();
    
    glUniform1f(programs["test"]->GetUniform("amplification"), 0.1f + (1.f + sinf(frameTime*5.f))*1.5f);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // ---------------- LUMINOSITY CALCULATION, COMPUTE AVERAGE PIXEL VALUE -------------

    // downsample image until we can do parallel reduction
    programs["downsample4x"]->Bind();
    textures["hdr-color"]->Bind(GL_TEXTURE_2D, GL_TEXTURE0);
    glUniform1i(programs["downsample4x"]->GetUniform("sourceImage"), 0);

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint currentLevel = 0;

    while( float(width>height ? width : height) / float(1<<currentLevel) > 256 )
    {
        glUniform1i(programs["downsample4x"]->GetUniform("sourceLevel"), currentLevel);
        glBindImageTexture(1, textures["hdr-color"]->GetHandle(), currentLevel+2, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

        // compute dispatch size
        GLuint x = ceilf(float(width / (1<<currentLevel)) / 128.f);
        GLuint y = ceilf(float(height / (1<<currentLevel)) / 128.f);

        glDispatchCompute(x, y, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        currentLevel+=2;
    }

    // parallel reduction
    programs["downsample4x_reduce"]->Bind();
    textures["hdr-color"]->Bind(GL_TEXTURE_2D, GL_TEXTURE0);
    glUniform1i(programs["downsample4x_reduce"]->GetUniform("sourceImage"), 0);
    glUniform1i(programs["downsample4x_reduce"]->GetUniform("sourceLevel"), currentLevel);
    glBindImageTexture(1, textures["luminosity"]->GetHandle(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute(textures["luminosity"]->GetWidth(), textures["luminosity"]->GetHeight(), 1);


    // ---------------- TONE MAPPING, GAMMA CORRECTION -------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0 , width, height);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    programs["tonemap"]->Bind();
    textures["hdr-color"]->Bind(GL_TEXTURE_2D, GL_TEXTURE0);
    textures["luminosity"]->Bind(GL_TEXTURE_2D, GL_TEXTURE1);

    const float gammaCorrectionFactor = 1.f/ 2.2f;
    glUniform1f(programs["tonemap"]->GetUniform("correctionFactor"), gammaCorrectionFactor);
    glUniform1i(programs["tonemap"]->GetUniform("renderHDR"), 0);
    glUniform1i(programs["tonemap"]->GetUniform("luminosity"), 1);
    fullscreenQuad.Draw();

    glEndQuery(GL_TIME_ELAPSED);
    std::swap(frontBuffer, backBuffer);

    // Get previous query result
    GLuint elapsed;
    glGetQueryObjectuiv(queries[frontBuffer], GL_QUERY_RESULT, &elapsed);
    //std::cout << "[INFO] Scene Render Took: " << float(elapsed)/1000.f << " us" << std::endl;
}

void Application::OnResize(GLint _width, GLint _height)
{
    std::cout << "[INFO] window resized " << _width << ", " << _height << std::endl;

    width  = _width;
    height = _height;
    aspect = float(height) / float(width);

    // Reallocate framebuffer storage
    textures["hdr-color"] = std::make_shared<Texture>(width, height, numberOfMipLevels(width,height), GL_RGBA16F);
    textures["hdr-depth"] = std::make_shared<Texture>(width, height, 1, GL_DEPTH24_STENCIL8);

    GLuint luminosityWidth, luminosityHeight;
    luminositySize(width, height, luminosityWidth, luminosityHeight);
    textures["luminosity"] = std::make_shared<Texture>(luminosityWidth, luminosityHeight, 1, GL_RGBA16F);
    std::cout << "[INFO] New luminosity texture size = " << luminosityWidth << "," << luminosityHeight << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures["hdr-color"]->GetHandle(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textures["hdr-depth"]->GetHandle(), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::OnEvent(SDL_Event event)
{
    
}
