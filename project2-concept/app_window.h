
// Ensure the header file is included only once in multi-file projects
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

# include <gsim/gs_color.h>
# include <gsim/gs_vec.h>
# include "ogl_tools.h"
# include "glut_window.h"

# include "so_axis.h"
# include "so_terrain.h"
# include "so_texture.h"

# include <map>

// The functionality of your application should be implemented inside AppWindow
class AppWindow : public GlutWindow
{
    // OpenGL shaders and programs:
    GlShader _vertexsh, _fragsh;
    GlShader _terrainvsh, _terrainfsh;
    
    GlProgram _prog;
    GlProgram _terrainprog;

    // My scene objects:
    SoAxis _axis;
    SoTerrain _terrain;
    
    // App data:
    enum MenuEv { evOption0, evOption1 };
    float  _rotx, _roty, _fovy, _lightpos;
    bool   _viewaxis;
    int    _w, _h;
    
    // App options:
    double timeWarp;
    bool   wireframe;
    GsVec2 location;
    
public :
    AppWindow ( const char* label, int x, int y, int w, int h );
    void initPrograms ();
    GsVec2 windowToScene ( const GsVec2& v );

    private : // functions derived from the base class
    virtual void glutMenu ( int m );
    virtual void glutKeyboard ( unsigned char key, int x, int y );
    virtual void glutSpecial ( int key, int x, int y );
    virtual void glutMouse ( int b, int s, int x, int y );
    virtual void glutMotion ( int x, int y );
    virtual void glutDisplay ();
    virtual void glutReshape ( int w, int h );
    virtual void glutIdle ();
};

#endif // APP_WINDOW_H
