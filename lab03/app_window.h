
// Ensure the header file is included only once in multi-file projects
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <gsim/gs_color.h>
#include <gsim/gs_vec.h>
#include "ogl_tools.h"
#include "glut_window.h"

#include "so_axis.h"
#include "so_capsule.h"
#include "so_tube.h"

#include <chrono>

// The functionality of your application should be implemented inside AppWindow
class AppWindow : public GlutWindow
{
    // OpenGL shaders and programs:
    GlShader _vertexsh, _fragsh;
    GlProgram _prog;
    
    // My scene objects:
    SoAxis _axis;
    SoTube _border;
    
    SoCapsule _subseconds;
    SoCapsule _seconds;
    
    // App data:
    enum MenuEv { evOption0, evOption1 };
    float _rotx, _roty, _fovy;
    bool  _viewaxis;
    int _w, _h;
    
    std::chrono::high_resolution_clock::time_point previousTime;
    double frameTime;
    bool   paused;
    
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
