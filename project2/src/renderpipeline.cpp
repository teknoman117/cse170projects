#include <project2/renderpipeline.hpp>
#include <project2/directories.hpp>

#include <cmath>
#include <tuple>

using glm::mat4;

namespace
{
    GLuint numberOfMipLevels(GLuint w, GLuint h)
    {
        GLuint maxDim = w > h ? w : h;
        return (GLuint) floorf(log2f(float(maxDim))) + 1;
    }

    // Function given an image computes the size of the luminosity image (1->2)x(1->2)
    std::tuple<GLuint,GLuint> luminositySize(GLuint w, GLuint h)
    {
        GLuint maxDimension = (w>h)?w:h;

        // 65536 texture is huge, move by 4 because thats what our shader does
        for(GLuint i = 0; i <= 16; i+=2)
        {
            if((maxDimension / (1 << i)) <= 256)
            {
                GLuint lw = (GLuint) ceilf(float(w / (1<<i)) / 128.f);
                GLuint lh = (GLuint) ceilf(float(h / (1<<i)) / 128.f);

                return std::make_tuple(lw,lh);
            }
        }

        return std::make_tuple<GLuint,GLuint>(0,0);
    }
}

RenderPipeline::RenderPipeline()
{
    // Lighting shader
    lighting_vs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/lighting_vs.glsl", GL_VERTEX_SHADER);

    // Tonemapping shader
    auto tonemap_fs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/tonemap_fs.glsl", GL_FRAGMENT_SHADER);
    tonemap = std::make_shared<Program>();
    tonemap->Attach(lighting_vs).Attach(tonemap_fs).Link();

    // Downsampling shaders
    auto downsample4x_cs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/downsample4x.glsl", GL_COMPUTE_SHADER);
    downsample4x = std::make_shared<Program>();
    downsample4x->Attach(downsample4x_cs).Link();

    auto downsample4x_reduce_cs = std::make_shared<Shader>(GetApplicationResourcesDirectory() + "/content/shaders/downsample4x_reduce.glsl", GL_COMPUTE_SHADER);
    downsample4x_reduce = std::make_shared<Program>();
    downsample4x_reduce->Attach(downsample4x_reduce_cs).Link();

    // Generate our framebuffer objects
    glGenFramebuffers(1, &gBufferFramebuffer);
    glGenFramebuffers(1, &LightFramebuffer);
}

RenderPipeline::~RenderPipeline()
{

}

void RenderPipeline::BeginGBufferPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFramebuffer);

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void RenderPipeline::EndGBufferPass()
{

}

void RenderPipeline::BeginLightPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, LightFramebuffer);

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE,GL_ONE);

    glClear(GL_COLOR_BUFFER_BIT);

    // Bind gbuffer into texture units
    gBufferDiffuseSpecular->Bind(GL_TEXTURE_2D, GL_TEXTURE0);
    gBufferNormals->Bind(GL_TEXTURE_2D, GL_TEXTURE1);
    gBufferPosition->Bind(GL_TEXTURE_2D, GL_TEXTURE2);
}

void RenderPipeline::EndLightPass()
{
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
}

void RenderPipeline::BeginLocalLightMask()
{

}

void RenderPipeline::EndLocalLightMask()
{

}

void RenderPipeline::BeginLocalLightRender()
{

}

void RenderPipeline::EndLocalLightRender()
{

}

void RenderPipeline::BeginGlobalLightRender()
{

}

void RenderPipeline::EndGlobalLightRender()
{

}

void RenderPipeline::PerformFinalRender()
{
    // --------------------- SCENE LUMINOSITY --------------------------
    // downsample image until we can do parallel reduction
    downsample4x->Bind();
    gBufferLightAccumulation->Bind(GL_TEXTURE_2D, GL_TEXTURE0);
    glUniform1i(downsample4x->GetUniform("sourceImage"), 0);

    GLuint currentLevel = 0;
    while( float(width>height ? width : height) / float(1<<currentLevel) > 256 )
    {
        glUniform1i(downsample4x->GetUniform("sourceLevel"), currentLevel);
        glBindImageTexture(1, gBufferLightAccumulation->GetHandle(), currentLevel+2, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

        // compute dispatch size
        GLuint x = ceilf(float(width / (1<<currentLevel)) / 128.f);
        GLuint y = ceilf(float(height / (1<<currentLevel)) / 128.f);

        glDispatchCompute(x, y, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        currentLevel+=2;
    }

    // parallel reduction
    downsample4x_reduce->Bind();
    glUniform1i(downsample4x_reduce->GetUniform("sourceImage"), 0);
    glUniform1i(downsample4x_reduce->GetUniform("sourceLevel"), currentLevel);
    glBindImageTexture(1, sceneLuminosity->GetHandle(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute(sceneLuminosity->GetWidth(), sceneLuminosity->GetHeight(), 1);

    // need to figure out how to control running average from a shader perspective

    // ---------------- BLOOM ----------------------------



    // ---------------- TONE MAPPING, GAMMA CORRECTION -------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    tonemap->Bind();
    sceneLuminosity->Bind(GL_TEXTURE_2D, GL_TEXTURE1);

    const float gammaCorrectionFactor = 1.f/ 2.2f;
    const mat4  nullTransform;
    
    glUniformMatrix4fv(tonemap->GetUniform("MVP"), 1, GL_FALSE, glm::value_ptr(nullTransform));

    glUniform1f(tonemap->GetUniform("correctionFactor"), gammaCorrectionFactor);
    glUniform1i(tonemap->GetUniform("renderHDR"), 0);
    glUniform1i(tonemap->GetUniform("luminosity"), 1);
    
    fullscreenQuad.Draw();
}

void RenderPipeline::ResizeRenderPipeline(GLuint width, GLuint height)
{
    this->width = width;
    this->height = height;

    // Reallocate framebuffer storage
    GLuint gBufferMipLevels = numberOfMipLevels(width, height);

    gBufferDiffuseSpecular   = std::make_shared<Texture>(width, height, 1,                GL_RGBA16F);
    gBufferNormals           = std::make_shared<Texture>(width, height, 1,                GL_RGB16F);
    gBufferPosition          = std::make_shared<Texture>(width, height, 1,                GL_RGB16F);
    gBufferLightAccumulation = std::make_shared<Texture>(width, height, gBufferMipLevels, GL_RGBA16F);
    gBufferDepthStencil      = std::make_shared<Texture>(width, height, 1,                GL_DEPTH24_STENCIL8);

    // Reallocate luminosity texture
    auto size = luminositySize(width, height);
    sceneLuminosity = std::make_shared<Texture>(std::get<0>(size), std::get<1>(size), 1, GL_RGBA16F);

    // Rebuild gbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,        GL_TEXTURE_2D, gBufferDiffuseSpecular->GetHandle(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,        GL_TEXTURE_2D, gBufferNormals->GetHandle(),         0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,        GL_TEXTURE_2D, gBufferPosition->GetHandle(),        0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, gBufferDepthStencil->GetHandle(),    0);

    GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);

    // Rebuild light accumulation buffer
    glBindFramebuffer(GL_FRAMEBUFFER, LightFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,        GL_TEXTURE_2D, gBufferLightAccumulation->GetHandle(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, gBufferDepthStencil->GetHandle(),      0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0 , width, height);
}

