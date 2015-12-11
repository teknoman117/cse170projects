#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include <project2/common.hpp>
#include <project2/program.hpp>
#include <project2/texture.hpp>
#include <project2/renderpipeline.hpp>

#include <project2/objects/glfullscreenquad.hpp>
#include <project2/objects/glsphere.hpp>

#include <project2/chunkedterrain.hpp>

class Application
{
    SDL_Window     *window;
    SDL_GLContext&  context;
    
    GLint           width;
    GLint           height;
    float           aspect;

    glm::vec2       viewRotation;
    glm::vec4       viewPosition;

    bool            wireframe;
    
    std::map<std::string, std::shared_ptr<Program>> programs;
    std::map<std::string, std::shared_ptr<Texture>> textures;

    RenderPipeline    renderer;
    GLFullscreenQuad  directionalLight;

    std::unique_ptr<GLSphere> testSphere;
    std::unique_ptr<ChunkedTerrain> chunkedTerrain;
    
public:
    Application(SDL_Window *window, SDL_GLContext& context);
    ~Application();
    
    bool OnDisplay(float frameTime, float frameDelta);
    bool OnResize(GLint width, GLint height);
    bool OnEvent(SDL_Event event);
};

#endif
