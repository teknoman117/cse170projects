#ifndef __RENDER_PIPELINE_HPP__
#define __RENDER_PIPELINE_HPP__

#include <project2/common.hpp>
#include <project2/program.hpp>
#include <project2/texture.hpp>
#include <project2/objects/glfullscreenquad.hpp>

class RenderPipeline
{
    GLuint width;
    GLuint height;

    std::shared_ptr<Shader>  lighting_vs;

    std::shared_ptr<Program> tonemap;
    std::shared_ptr<Program> downsample4x;
    std::shared_ptr<Program> downsample4x_reduce;

    std::shared_ptr<Texture> gBufferDepthStencil;
    std::shared_ptr<Texture> gBufferDiffuseSpecular;
    std::shared_ptr<Texture> gBufferNormals;
    std::shared_ptr<Texture> gBufferPosition;
    std::shared_ptr<Texture> gBufferLightAccumulation;

    std::shared_ptr<Texture> sceneLuminosity;

    GLuint gBufferFramebuffer;
    GLuint LightFramebuffer;

    GLFullscreenQuad fullscreenQuad;

public:
    RenderPipeline();
    ~RenderPipeline();

    void BeginGBufferPass();
    void EndGBufferPass();
    void BeginLightPass();
    void EndLightPass();

    // Point lights, spot lights, etc.
    void BeginLocalLightMask();
    void EndLocalLightMask();
    void BeginLocalLightRender();
    void EndLocalLightRender();

    // Directional lights
    void BeginGlobalLightRender();
    void EndGlobalLightRender();

    void PerformFinalRender();

    void ResizeRenderPipeline(GLuint width, GLuint height);

    std::shared_ptr<Program> CompileLightingProgram(const std::string& path)
    {
        std::shared_ptr<Program> p = std::make_shared<Program>();
        p->Attach(path, GL_FRAGMENT_SHADER).Attach(lighting_vs).Link();
        return p;
    }
};

#endif
