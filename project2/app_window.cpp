
# include <iostream>
# include <gsim/gs.h>
# include "app_window.h"
# include <cmath>
# include <chrono>

#ifndef M_PI
#define M_PI 3.14159
#endif

using namespace std::chrono;

AppWindow::AppWindow ( const char* label, int x, int y, int w, int h )
    : GlutWindow ( label, x, y, w, h )
{
    initPrograms ();
    addMenuEntry ( "Option 0", evOption0 );
    addMenuEntry ( "Option 1", evOption1 );
    
    _viewaxis = true;
    _fovy = GS_TORAD(60.0f);
    _rotx = _roty = 0;
    _w = w;
    _h = h;
    _lightpos = 0.0f;
    
    timeWarp = 1.0;
    wireframe = false;
    location = GsVec2((5120 - 2740) * 0.76f, (5120 - 2640));
}

void AppWindow::initPrograms ()
{
    // Load your shaders and link your programs here:
#ifdef WIN32
    _vertexsh.load_and_compile ( GL_VERTEX_SHADER, "../vsh_mcol_flat.glsl" );
    _fragsh.load_and_compile ( GL_FRAGMENT_SHADER, "../fsh_flat.glsl" );
    _terrainvsh.load_and_compile (GL_VERTEX_SHADER, "../vsh_terrain.glsl");
    _terrainfsh.load_and_compile (GL_FRAGMENT_SHADER, "../fsh_terrain.glsl");
#else
	_vertexsh.load_and_compile(GL_VERTEX_SHADER, "vsh_mcol_flat.glsl");
    _fragsh.load_and_compile(GL_FRAGMENT_SHADER, "fsh_flat.glsl");
    _terrainvsh.load_and_compile (GL_VERTEX_SHADER, "vsh_terrain.glsl");
    _terrainfsh.load_and_compile (GL_FRAGMENT_SHADER, "fsh_terrain.glsl");
#endif

    _prog.init_and_link ( _vertexsh, _fragsh );
    _terrainprog.init_and_link(_terrainvsh, _terrainfsh);
    
    // Init my scene objects:
    _axis.init ( _prog );
    _terrain.init ( _terrainprog );
    _terrain.build("terrain.raw", 1024, 1024, GsVec(10.f * 0.76f, 1.f, 10.f));
}

// mouse events are in window coordinates, but your 2D scene is in [0,1]x[0,1],
// so make here the conversion when needed
GsVec2 AppWindow::windowToScene ( const GsVec2& v )
 {
   // GsVec2 is a lighteweight class suitable to return by value:
   return GsVec2 ( (2.0f*(v.x/float(_w))) - 1.0f,
                    1.0f - (2.0f*(v.y/float(_h))) );
 }

// Called every time there is a window event
void AppWindow::glutKeyboard ( unsigned char key, int x, int y )
{
    float angle = 0;
    
    switch ( key )
    {
        case ' ': wireframe = !wireframe; break;
        case 'w':
            angle = _roty + M_PI;
            location += GsVec2(sinf(angle), cosf(angle) );
            break;
        case 's':
            angle = _roty;
            location += GsVec2(sinf(angle), cosf(angle) );
            break;
        case 'a':
            angle = _roty + M_PI*(3.f/2.f);
            location += GsVec2(sinf(angle), cosf(angle) );
            break;
        case 'd':
            angle = _roty + M_PI/2.f;
            location += GsVec2(sinf(angle), cosf(angle) );
            break;
      case 27 : exit(1); // Esc was pressed
    }
}

void AppWindow::glutSpecial ( int key, int x, int y )
{
    const float incr=GS_TORAD(2.5f);
    const float incf=0.05f;
    switch ( key )
    { 
        case GLUT_KEY_LEFT:      _roty+=incr; break;
        case GLUT_KEY_RIGHT:     _roty-=incr; break;
        case GLUT_KEY_UP:        _rotx-=incr; break;
        case GLUT_KEY_DOWN:      _rotx+=incr; break;
        case GLUT_KEY_PAGE_UP:   _fovy-=incf; break;
        case GLUT_KEY_PAGE_DOWN: _fovy+=incf; break;
        case GLUT_KEY_HOME:      _fovy=GS_TORAD(60.0f); _rotx=_roty=0; break;
    }
}

void AppWindow::glutMouse ( int b, int s, int x, int y )
{
    // The mouse is not used in this example.
    // Recall that a mouse click in the screen corresponds
    // to a whole line traversing the 3D scene.
}

void AppWindow::glutMotion ( int x, int y )
{
}

void AppWindow::glutMenu ( int m )
 {
   std::cout<<"Menu Event: "<<m<<std::endl;
 }

void AppWindow::glutReshape ( int w, int h )
 {
   // Define that OpenGL should use the whole window for rendering
   glViewport( 0, 0, w, h );
   _w=w; _h=h;
 }

// here we will redraw the scene according to the current state of the application.
void AppWindow::glutDisplay ()
{
    static double frameTime = 0.0;
    static high_resolution_clock::time_point previousTime = high_resolution_clock::now();
    high_resolution_clock::time_point now = high_resolution_clock::now();
    duration<double> frameTimeDelta = duration_cast<duration<double>>(now - previousTime);
    previousTime = now;
    frameTime += timeWarp * frameTimeDelta.count();
    
    
    // Clear the rendering window
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

    // Build a cross with some lines (if not built yet):
    if ( _axis.changed ) // needs update
    {
        _axis.build(1.0f); // axis has radius 1.0
    }

    // Define our scene transformation:
    GsMat rx, ry, stransf;
    rx.rotx ( _rotx );
    ry.roty ( _roty );
    stransf = ry*rx; // set the scene transformation matrix

    // Define our projection transformation:
    // (see demo program in gltutors-projection.7z, we are replicating the same behavior here)
    GsMat camview, persp, sproj;
    
    // Camera matrix
    camview.translation(GsVec(location.x, _terrain.sampleHeightAtPoint(location) + 1.8f, location.y));
    camview = camview * stransf;
    camview.invert();

    float aspect=static_cast<float>(_w)/static_cast<float>(_h), znear=0.1f, zfar=5000.0f;
    persp.perspective ( _fovy, aspect, znear, zfar ); // set our 4x4 perspective matrix

    // Our matrices are in "line-major" format, so vertices should be multiplied on the 
    // right side of a matrix multiplication, therefore in the expression below camview will
    // affect the vertex before persp, because v' = (persp*camview)*v = (persp)*(camview*v).
    sproj = persp * camview; // set final scene projection

    //  Note however that when the shader receives a matrix it will store it in column-major 
    //  format, what will cause our values to be transposed, and we will then have in our 
    //  shaders vectors on the left side of a multiplication to a matrix.

    // Draw:
    //if ( _viewaxis )
    GsMat iden;
    iden.identity();
    
    _axis.draw (iden, sproj );
    _terrain.draw ( iden, sproj );
    
    // Swap buffers and draw:
    glFlush();         // flush the pipeline (usually not necessary)
    glutSwapBuffers(); // we were drawing to the back buffer, now bring it to the front
}

void AppWindow::glutIdle()
{
  redraw();
}

