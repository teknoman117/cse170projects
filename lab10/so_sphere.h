
// Ensure the header file is included only once in multi-file projects
#ifndef SO_SPHERE_H
#define SO_SPHERE_H

// Include needed header files
# include <vector>
# include <gsim/gs_color.h>
# include <gsim/gs_vec.h>
# include "ogl_tools.h"

# include "light.h"
# include "material.h"

// Scene objects should be implemented in their own classes; and
// here is an example of how to organize a scene object in a class.
// Scene object axis:
class SoSphere
{
public:
    class Renderer : public GlObjects
    {
        const SoSphere *sphere;
        
    public:
        Renderer () : sphere(NULL) {}
        void init ( const GlProgram& prog );
        void draw ( GsMat& tr, GsMat& pr, const Light& l, const Material& m );
        
        const SoSphere* GetSphere() const;
        void            SetSphere( const SoSphere *sphere );
    };
    
private:
    size_t _numpoints;
    GLuint _buffers[2];
    
    friend Renderer;
    
public:
    void init ();
    void build ( float radius, unsigned short resolution );
};

#endif // SO_AXIS_H
