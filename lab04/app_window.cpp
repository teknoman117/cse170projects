
# include <iostream>
# include <gsim/gs.h>
# include "app_window.h"
# include <cmath>

#ifndef M_PI
#define M_PI 3.14159
#endif

AppWindow::AppWindow ( const char* label, int x, int y, int w, int h )
    : GlutWindow ( label, x, y, w, h ), _sun(GsVec(2, 2, 2), GsColor(128, 128, 0), GsColor::white, GsColor::white), _material(GsColor(128, 128, 0), GsColor(255, 255, 0), GsColor::white, 3.0f)
{
    initPrograms ();
    addMenuEntry ( "Option 0", evOption0 );
    addMenuEntry ( "Option 1", evOption1 );
    
    _viewaxis = true;
    _fovy = GS_TORAD(60.0f);
    _rotx = _roty = 0;
    _w = w;
    _h = h;
    
    nfaces = 16;
    flat = false;
    normals = false;
    lightpos = 0.0;
}

void AppWindow::initPrograms ()
{
    // Load your shaders and link your programs here:
#ifdef WIN32
    _vertexsh.load_and_compile ( GL_VERTEX_SHADER, "../vsh_mcol_flat.glsl" );
    _fragsh.load_and_compile ( GL_FRAGMENT_SHADER, "../fsh_flat.glsl" );
    _flatvsh.load_and_compile(GL_VERTEX_SHADER, "../vsh_flat.glsl");
    _lightvsh.load_and_compile(GL_VERTEX_SHADER, "../vsh_lighting.glsl");
    _lightfsh.load_and_compile(GL_FRAGMENT_SHADER, "../fsh_lighting.glsl");
#else
	_vertexsh.load_and_compile(GL_VERTEX_SHADER, "vsh_mcol_flat.glsl");
	_fragsh.load_and_compile(GL_FRAGMENT_SHADER, "fsh_flat.glsl");
	_flatvsh.load_and_compile(GL_VERTEX_SHADER, "vsh_flat.glsl");
	_lightvsh.load_and_compile(GL_VERTEX_SHADER, "vsh_lighting.glsl");
	_lightfsh.load_and_compile(GL_FRAGMENT_SHADER, "fsh_lighting.glsl");
#endif

    _prog.init_and_link ( _vertexsh, _fragsh );
    _flatprog.init_and_link (  _flatvsh, _fragsh );
    _lightprog.init_and_link ( _lightvsh, _lightfsh );
    
    // Init my scene objects:
    _axis.init ( _prog );
    _cylinder.init ( _lightprog, _prog, true );
    _flatcylinder.init( _flatprog, _prog, false );
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
    switch ( key )
    {
      case ' ': _viewaxis = !_viewaxis; break;
      case 'q': nfaces++; _cylinder.changed = 1; break;
      case 'a': nfaces = (nfaces > 3) ? nfaces - 1 : 3; _cylinder.changed = 1; break;
      case 'z': flat = true; break;
      case 'x': flat = false; break;
      case 'c': normals = true; break;
      case 'v': normals = false; break;
      case 'w': lightpos += (M_PI / 36.0); break;
      case 's': lightpos -= (M_PI / 36.0); break;
      case 27 : exit(1); // Esc was pressed
    }
}

void AppWindow::glutSpecial ( int key, int x, int y )
{
    const float incr=GS_TORAD(2.5f);
    const float incf=0.05f;
    switch ( key )
    { 
        case GLUT_KEY_LEFT:      _roty-=incr; break;
        case GLUT_KEY_RIGHT:     _roty+=incr; break;
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
    // Clear the rendering window
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Build a cross with some lines (if not built yet):
    if ( _axis.changed ) // needs update
    {
        _axis.build(1.0f); // axis has radius 1.0
    }
    if ( _cylinder.changed )
    {
        _cylinder.build(1.0f, 0.25f, nfaces);
        _flatcylinder.build(1.0f, 0.25f, nfaces);
    }

    // Define our scene transformation:
    GsMat rx, ry, stransf;
    rx.rotx ( _rotx );
    ry.roty ( _roty );
    stransf = rx*ry; // set the scene transformation matrix

    // Define our projection transformation:
    // (see demo program in gltutors-projection.7z, we are replicating the same behavior here)
    GsMat camview, persp, sproj;

    GsVec eye(0,0,2), center(0,0,0), up(0,1,0);
    camview.lookat ( eye, center, up ); // set our 4x4 "camera" matrix

    float aspect=static_cast<float>(_w)/static_cast<float>(_h), znear=0.1f, zfar=50.0f;
    persp.perspective ( _fovy, aspect, znear, zfar ); // set our 4x4 perspective matrix

    // Our matrices are in "line-major" format, so vertices should be multiplied on the 
    // right side of a matrix multiplication, therefore in the expression below camview will
    // affect the vertex before persp, because v' = (persp*camview)*v = (persp)*(camview*v).
    sproj = persp * camview; // set final scene projection

    //  Note however that when the shader receives a matrix it will store it in column-major 
    //  format, what will cause our values to be transposed, and we will then have in our 
    //  shaders vectors on the left side of a multiplication to a matrix.

    // Draw:
    if ( _viewaxis ) _axis.draw ( stransf, sproj );
    
    _sun.pos = stransf * GsVec(2.0 * std::cos(lightpos), 2.0, 2.0 * std::sin(lightpos));
    
    if(flat)
    {
        _flatcylinder.draw(stransf, sproj);
        if(normals)
        {
            _flatcylinder.drawNormals(stransf, sproj);
        }
    }
    else
    {
        _cylinder.draw(stransf, sproj, _sun, _material);
        if(normals)
        {
            _cylinder.drawNormals(stransf, sproj);
        }
    }
    
    // Swap buffers and draw:
    glFlush();         // flush the pipeline (usually not necessary)
    glutSwapBuffers(); // we were drawing to the back buffer, now bring it to the front
}

void AppWindow::glutIdle()
{
  redraw();
}

