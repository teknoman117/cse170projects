#ifndef __TEXTURE_HPP__
#define __TEXTURE_HPP__

#include <project2/common.hpp>

class Texture
{
    GLuint handle;

    GLuint width;
    GLuint height;
    GLuint levels;
    GLint  format;

public:
    Texture(GLuint width, GLuint height, GLuint levels = 1, GLenum format = GL_RGBA8);
    Texture(const std::string& path, GLint format, bool mipmapped = true);
    ~Texture();

    void Bind(GLenum target, GLenum unit);

    void SetWrapMode(GLint mode);

    GLuint GetHandle() const;
    GLuint GetWidth()  const;
    GLuint GetHeight() const;
    GLenum GetFormat() const;
};

#endif
