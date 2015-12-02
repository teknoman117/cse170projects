#ifndef __SO_TEXTURE_H__
#define __SO_TEXTURE_H__

#include "gs_image.h"
#include "ogl_tools.h"

class SoTexture
{
    GLuint textureHandle;
    
public:
    SoTexture();
    bool Load(std::string path);
    void Bind(GLenum textureUnit);
};

#endif
