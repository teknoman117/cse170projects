#ifndef __TEXTURE_HPP__
#define __TEXTURE_HPP__

#include <project2/common.hpp>

class Texture
{
	GLuint handle;

	size_t width;
	size_t height;
	GLint  format;

public:
	Texture(size_t width, size_t height, size_t levels = 1, GLenum format = GL_RGBA8);
	Texture(const std::string& path, GLint format, bool mipmapped = true);
	~Texture();

	void Bind(GLenum target);
	void Resize(size_t width, size_t height, size_t levels);

	void SetWrapMode(GLint mode);

	GLuint GetHandle() const;
	size_t GetWidth()  const;
	size_t GetHeight() const;
	GLenum GetFormat() const;
};

#endif
