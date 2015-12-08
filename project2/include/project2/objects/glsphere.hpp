#ifndef SO_SPHERE_H
#define SO_SPHERE_H

// Include needed header files
#include <project2/common.hpp>
#include <project2/program.hpp>
#include <project2/objects/globject.hpp>

class GLSphere : public GLObject
{
    size_t numPoints;
    
public:
    GLSphere(float radius, unsigned short resolution);

    void Draw();
};

#endif 
