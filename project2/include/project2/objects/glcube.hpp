#ifndef __GLSKYBOX_HPP__
#define __GLSKYBOX_HPP__

#include <project2/common.hpp>
#include <project2/objects/globject.hpp>

class GLCube : public GLObject
{
public:
    GLCube();
    virtual ~GLCube();

    void Draw();
};

#endif
