
// Ensure the header file is included only once in multi-file projects
#ifndef SO_MODEL_NORMALS_H
#define SO_MODEL_NORMALS_H

// Include needed header files
#include <vector>
#include <gsim/gs_color.h>
#include <gsim/gs_vec.h>

#include "ogl_tools.h"
#include "armmodel/model.h"

// Scene objects should be implemented in their own classes; and
// here is an example of how to organize a scene object in a class.
// Scene object axis:
class SoModelNormals : public GlObjects
{
    // saves the number of elements
    std::vector<GsVec>::size_type _smoothStart;
    std::vector<GsVec>::size_type _smoothCount;
    
    std::vector<GsVec>::size_type _flatStart;
    std::vector<GsVec>::size_type _flatCount;

public :
    SoModelNormals ();
    
    void init ( const GlProgram& prog );
    void build ( const Model& model );
    void draw ( GsMat& tr, GsMat& pr, bool showFlatNormals );
};

#endif // SO_AXIS_H
