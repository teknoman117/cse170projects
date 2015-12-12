#ifndef __CUBEMAP_HPP__
#define __CUBEMAP_HPP__

#include <project2/common.hpp>

class Cubemap
{
    GLuint handle;

public:
    Cubemap(const std::string& path);
    ~Cubemap();

    void Bind(GLenum unit);
};

#endif
