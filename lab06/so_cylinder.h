
// Ensure the header file is included only once in multi-file projects
#ifndef SO_HUR_H
#define SO_HUR_H

// Include needed header files
# include <gsim/gs_mat.h>
# include <gsim/gs_light.h>
# include <gsim/gs_color.h>
# include <gsim/gs_material.h>
# include <gsim/gs_array.h>
# include "ogl_tools.h"

// Scene objects should be implemented in their own classes; and
// here is an example of how to organize a scene object in a class.
// Scene object axis:
class SoTexturedCylinder : public GlObjects
{
    GlShader _vshgou, _fshgou, _vshphong, _fshphong;
    GlProgram _proggouraud, _progphong;

    GsMaterial _mtl;
    int _numpoints;     // just saves the number of points
    bool _phong;
    bool _flatn;

public :
    GsArray<GsVec> NL; // normal lines computed in last build

public :
    SoTexturedCylinder ();
    void init ();
    void flat ( bool b ) { _flatn=b; }
    void phong ( bool b ) { _phong=b; }
    void build ( float rt, float rb, float len, int numfaces );
    void draw ( const GsMat& tr, const GsMat& pr, const GsLight& l, int textureId, float blendFactor );
};

#endif // SO_MODEL_H
