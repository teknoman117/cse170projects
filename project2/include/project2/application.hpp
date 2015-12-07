#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include <map>

#include <glm/glm.hpp>

#include <project2/common.hpp>
#include <project2/program.hpp>
#include <project2/texture.hpp>
#include <project2/renderpipeline.hpp>

class Application
{
    SDL_Window     *window;
    SDL_GLContext&  context;
    
    GLint           width;
    GLint           height;
    float           aspect;

    GLuint          vao;
    GLuint          buf;

    GLuint          queries[2];
    GLuint          frontBuffer;
    GLuint          backBuffer;
    
    std::map<std::string, std::shared_ptr<Program>> programs;
    std::map<std::string, std::shared_ptr<Texture>> textures;

    RenderPipeline  renderer;
    
public:
    Application(SDL_Window *window, SDL_GLContext& context);
    ~Application();
    
    void OnDisplay(float frameTime, float frameDelta);
    void OnResize(GLint width, GLint height);
    void OnEvent(SDL_Event event);
};

#endif
