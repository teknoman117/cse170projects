
// Ensure the header file is included only once in multi-file projects
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

# include <gsim/gs_color.h>
# include <gsim/gs_array.h>
# include <gsim/gs_light.h>
# include <gsim/gs_vec.h>

# include "ogl_tools.h"
# include "glut_window.h"

# include "so_axis.h"
# include "so_lines.h"
# include "so_superquadric.h"
# include "so_cylinder.h"

# include "gs_image.h"

// The functionality of your application should be implemented inside AppWindow
class AppWindow : public GlutWindow
{
    // My scene objects:
    SoAxis _axis;
    SoLines _lines; // used to draw the object normals
    SoTexturedCylinder _texturedCylinder;

    // Parameters
    float rt;
    float rb;
    int   numfaces;
    bool  textureChoice;
    float blendFactor;
    
    // Scene data:
    bool    _viewaxis;
    GsLight _light;
    GLuint  _textures[2];

    // App data:
    enum MenuEv { evOption0, evOption1 };
    float _rotx, _roty, _fovy;
    int _w, _h;
    double _lasttime; // last time update during animation
    float _animateinc;
    bool _animate;
    bool _normals;
    bool _flatn;
    bool _phong;

public :
    AppWindow ( const char* label, int x, int y, int w, int h );
    void initPrograms ();
    void initTextures ();
    GsVec2 windowToScene ( const GsVec2& v );
    void printInfo ();

private : // functions derived from the base class
    virtual void glutMenu ( int m );
    virtual void glutKeyboard ( unsigned char key, int x, int y );
    virtual void glutSpecial ( int key, int x, int y );
    virtual void glutMouse ( int b, int s, int x, int y );
    virtual void glutMotion ( int x, int y );
    virtual void glutDisplay ();
    virtual void glutReshape ( int w, int h );
};

#endif // APP_WINDOW_H
