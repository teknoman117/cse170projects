
// Ensure the header file is included only once in multi-file projects
#ifndef SO_SPHERE_H
#define SO_SPHERE_H

// Include needed header files
# include <vector>
# include <gsim/gs_color.h>
# include <gsim/gs_vec.h>
# include "ogl_tools.h"

// Scene objects should be implemented in their own classes; and
// here is an example of how to organize a scene object in a class.
// Scene object axis:
class SoTerrain : public GlObjects
{
    std::vector<GsVec> V;
    
    size_t width;
    size_t height;
    size_t _numpoints;
    
    GsVec convertToMeters;
    
public:
    void  init ( const GlProgram& prog );
    void  build ( std::string filename, size_t width, size_t height, GsVec convertToMeters = GsVec(1,1,1) );
    void  draw (const GsMat& tr, const GsMat& pr);
    
    float sampleHeightAtPoint ( GsVec2 point );
};

#endif // SO_AXIS_H
