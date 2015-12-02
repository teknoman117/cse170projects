#include "so_texture.h"

SoTexture::SoTexture()
{
    glGenTextures(1, &textureHandle);
    
    static GLubyte whiteTexture[4] = {255, 255, 255, 255};
    
    glBindTexture ( GL_TEXTURE_2D, textureHandle );
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteTexture );
    glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glGenerateMipmap ( GL_TEXTURE_2D );
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool SoTexture::Load(std::string path)
{
    GsImage I;
    if(!I.load(path.c_str()))
        return false;
    
    glBindTexture ( GL_TEXTURE_2D, textureHandle );
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, I.w(), I.h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, I.data() );
    glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glGenerateMipmap ( GL_TEXTURE_2D );
    glBindTexture(GL_TEXTURE_2D, 0);
    I.init(0,0);
    
    return true;
}

void SoTexture::Bind(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
}
