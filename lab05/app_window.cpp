
# include <iostream>
# include <gsim/gs.h>
# include "app_window.h"
# include <cmath>

AppWindow::AppWindow ( const char* label, int x, int y, int w, int h )
    : GlutWindow ( label, x, y, w, h ),
    _light(GsVec(2, 2, 2), GsColor::darkgray, GsColor::white, GsColor::white),
    _material(GsColor(32, 32, 32), GsColor::white, GsColor::white, 4.0f)
{
    initPrograms ();
    
    addMenuEntry ( "Option 0", evOption0 );
    addMenuEntry ( "Option 1", evOption1 );
    
    _viewaxis = true;
    _fovy = GS_TORAD(60.0f);
    _rotx = _roty = 0;
    _w = w;
    _h = h;
    
    showNormals = false;
    showFlat = false;
    showWireframe = false;
    
    shoulderRotation = M_PI/4.0f;
    elbowRotation = -M_PI/3.0f;
    handRotation = M_PI/8.0f;
}

void AppWindow::initPrograms ()
{
    // Load your shaders and link your programs here:
#ifdef WIN32
    _vertexsh.load_and_compile ( GL_VERTEX_SHADER, "../vsh_mcol_flat.glsl" );
    _fragsh.load_and_compile ( GL_FRAGMENT_SHADER, "../fsh_flat.glsl" );
#else
    _vertexsh.load_and_compile ( GL_VERTEX_SHADER, "vsh_mcol_flat.glsl" );
    _fragsh.load_and_compile ( GL_FRAGMENT_SHADER, "fsh_flat.glsl" );
    _modelvsh.load_and_compile( GL_VERTEX_SHADER, "vsh_flat_model.glsl" );
    _modelfsh.load_and_compile( GL_FRAGMENT_SHADER, "fsh_flat_model.glsl" );
#endif

    _prog.init_and_link ( _vertexsh, _fragsh );
    _modelprog.init_and_link( _modelvsh, _modelfsh );

    // Init my scene objects:
    _axis.init ( _prog );
    _hand.init( _modelprog );
    _lowerarm.init( _modelprog );
    _upperarm.init( _modelprog );

    // Load the models
    Model rhand, rlowerarm, rupperarm;

#ifdef WIN32
#error getrekt
#else
    rhand.load("armmodel/rhand.model");
    rlowerarm.load("armmodel/rlowerarm.model");
    rupperarm.load("armmodel/rupperarm.model");
#endif
    
    // Build the model scene objects
    _hand.build( rhand );
    _lowerarm.build( rlowerarm );
    _upperarm.build( rupperarm );
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
     const float incf = (M_PI / 32.0f);
     
   switch ( key )
    { case ' ': _viewaxis = !_viewaxis; break;
      case 'z': showWireframe = !showWireframe; break;
      case 'x': showFlat = true; break;
      case 'c': showFlat = false; break;
      case 'v': showNormals = !showNormals; break;
        case 'q': shoulderRotation -= incf; break;
        case 'a': shoulderRotation += incf; break;
        case 'w': elbowRotation -= incf; break;
        case 's': elbowRotation += incf; break;
        case 'e': handRotation -= incf; break;
        case 'd': handRotation += incf; break;
	  case 27 : exit(1); // Esc was pressed
	}
     
     redraw();
 }

void AppWindow::glutSpecial ( int key, int x, int y )
 {
   bool rd=true;
   const float incr=GS_TORAD(2.5f);
   const float incf=0.05f;
   switch ( key )
    { case GLUT_KEY_LEFT:      _roty-=incr; break;
      case GLUT_KEY_RIGHT:     _roty+=incr; break;
      case GLUT_KEY_UP:        _rotx-=incr; break;
      case GLUT_KEY_DOWN:      _rotx+=incr; break;
      case GLUT_KEY_PAGE_UP:   _fovy-=incf; break;
      case GLUT_KEY_PAGE_DOWN: _fovy+=incf; break;
      case GLUT_KEY_HOME:      _fovy=GS_TORAD(60.0f); _rotx=_roty=0; break;
      default: return; // return without rendering
	}
   if (rd) redraw(); // ask the window to be rendered when possible
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
    glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);

    // Build a cross with some lines (if not built yet):
    if ( _axis.changed ) // needs update
    {
        _axis.build(1.0f); // axis has radius 1.0
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

    float aspect=1.0f, znear=0.1f, zfar=50.0f;
    persp.perspective ( _fovy, aspect, znear, zfar ); // set our 4x4 perspective matrix

    // Our matrices are in "line-major" format, so vertices should be multiplied on the 
    // right side of a matrix multiplication, therefore in the expression below camview will
    // affect the vertex before persp, because v' = (persp*camview)*v = (persp)*(camview*v).
    sproj = persp * camview; // set final scene projection

    //  Note however that when the shader receives a matrix it will store it in column-major 
    //  format, what will cause our values to be transposed, and we will then have in our 
    //  shaders vectors on the left side of a multiplication to a matrix.

    // Draw:
    if ( _viewaxis )
    {
        _axis.draw ( stransf, sproj );
    }
    _light.pos = stransf * GsVec(-2,2,2);
    
    
    GsMat scale, translation, translationB, rotation, rotationB, transform;
    scale.scale(0.0225, 0.0225, 0.0225);
    translation.translation(0, 0, -0.5f);
    rotation.roty(M_PI/2.0f);
    transform = stransf * rotation * translation * scale;
    
    
    // Draw the upper arm
    rotation.rotx(shoulderRotation);
    transform = transform * rotation;
    _upperarm.draw(transform, sproj, _light, _material);

    // Draw the lower arm
    translation.translation(0.0f, 0.0f, 26.0f);
    rotation.rotx(elbowRotation);
    transform = transform * translation * rotation;
    _lowerarm.draw(transform, sproj, _light, _material);
    
    // Draw the hand
    translation.translation(0.0, 0.0f, 24.0f);
    rotation.rotx(handRotation);
    transform = transform * translation * rotation;
    _hand.draw(transform, sproj, _light, _material);
    
    // Swap buffers and draw:
    glFlush();         // flush the pipeline (usually not necessary)
    glutSwapBuffers(); // we were drawing to the back buffer, now bring it to the front
}

