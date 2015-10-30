
// Ensure the header file is included only once in multi-file projects
#ifndef SO_CYLINDER_H
#define SO_CYLINDER_H

// Include needed header files
# include <vector>
# include <gsim/gs_color.h>
# include <gsim/gs_vec.h>
# include "ogl_tools.h"

# include "light.h"
# include "material.h"
# include "so_segments.h"

// Scene objects should be implemented in their own classes; and
// here is an example of how to organize a scene object in a class.
// Scene object axis:
class SoCylinder : public GlObjects
{
    size_t elementCount;
    
    SoSegments normals;
    bool smooth;
    
public:
    
    void init ( const GlProgram& prog, const GlProgram& nprog, bool smooth = false );
    void build ( float length, float radius, unsigned short nfaces );
    
    void draw ( GsMat& tr, GsMat& pr );
    void draw ( GsMat& tr, GsMat& pr, const Light& l, const Material& m );
    
    void drawNormals ( GsMat& tr, GsMat& pr );
};

#endif // SO_AXIS_H
