#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include <project2/common.hpp>
#include <project2/program.hpp>
#include <project2/texture.hpp>
#include <project2/cubemap.hpp>
#include <project2/renderpipeline.hpp>
#include <project2/cameraparameters.hpp>

#include <project2/objects/glfullscreenquad.hpp>
#include <project2/objects/glsphere.hpp>
#include <project2/objects/glcube.hpp>
#include <project2/objects/glline.hpp>

#include <project2/chunkedterrain.hpp>

#include <vector>

class Application
{
public:
    struct Options
    {
        std::string rasterFilename;
        std::string pathFilename;
        glm::ivec2  rasterSize;
        glm::ivec2  chunkSize;
        glm::dvec2  resolution;
        glm::dvec2  corner;
        glm::dvec2  starting;
    };

private:
    SDL_Window      *window;
    SDL_GLContext&   context;

    CameraParameters camera;

    float            timeOfDay;
    bool             wireframe;
    bool             flying;

    bool             following;
    size_t           currentPosition;
    float            animationTime;

    std::map<std::string, std::shared_ptr<Program>> programs;
    std::map<std::string, std::shared_ptr<Texture>> textures;

    RenderPipeline    renderer;
    GLFullscreenQuad  directionalLight;

    std::unique_ptr<GLSphere>       testSphere;
    std::unique_ptr<ChunkedTerrain> chunkedTerrain;
    std::unique_ptr<GLCube>         skybox;
    std::unique_ptr<Cubemap>        skyboxTexture;
    std::unique_ptr<GLLine>         pathLine;
    std::vector<glm::vec3>          cameraPath;
    
public:
    Application(SDL_Window *window, SDL_GLContext& context, const Application::Options& options);
    ~Application();
    
    bool OnDisplay(float frameTime, float frameDelta);
    bool OnResize(GLint width, GLint height);
    bool OnEvent(SDL_Event event);
};

#endif
