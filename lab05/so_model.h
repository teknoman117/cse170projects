
// Ensure the header file is included only once in multi-file projects
#ifndef SO_MODEL_H
#define SO_MODEL_H

// Include needed header files
#include <vector>
#include <gsim/gs_color.h>
#include <gsim/gs_vec.h>

#include "ogl_tools.h"
#include "model.h"

#include "light.h"
#include "material.h"

// Scene objects should be implemented in their own classes; and
// here is an example of how to organize a scene object in a class.
// Scene object axis:
class SoModel : public GlObjects
{
    // Represents a vertex of the model
    struct Vertex
    {
        GsVec   vertex;
        GsVec   normal;
        GsColor color;
    };
    
    // saves the number of elements
    std::vector<struct Vertex>::size_type _numpoints;

public :
    SoModel ();
    
    void init ( const GlProgram& prog );
    void build ( const Model& model );
    void draw ( GsMat& tr, GsMat& pr, const Light& l, const Material& m );
};

#endif // SO_AXIS_H
