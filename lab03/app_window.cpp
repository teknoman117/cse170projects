
#include <iostream>
#include <gsim/gs.h>
#include "app_window.h"

#include <cmath>

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
    
    previousTime = high_resolution_clock::now();
    frameTime = 0.0;
    paused = false;
}

void AppWindow::initPrograms ()
{
    // Load your shaders and link your programs here:
    _vertexsh.load_and_compile ( GL_VERTEX_SHADER, "vsh_mcol_flat.glsl" );
    _fragsh.load_and_compile ( GL_FRAGMENT_SHADER, "fsh_flat.glsl" );
    _prog.init_and_link ( _vertexsh, _fragsh );

    // Init my scene objects:
    _axis.init ( _prog );
    _border.init ( _prog );
    _subseconds.init ( _prog );
    _seconds.init ( _prog );
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
    case ' ': paused = !paused; break;
    case '\r': frameTime = 0.0; break;
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
    // Frame time calculations
    high_resolution_clock::time_point currentTime = high_resolution_clock::now();
    duration<double> frameDelta = duration_cast<duration<double>>(currentTime - previousTime);
    previousTime = currentTime;
    
    if(!paused)
        frameTime += frameDelta.count();
    
    double subsecondsRotation = frameTime * (2.0 * M_PI);
    double secondsRotation = (frameTime / 60.0) * (2.0 * M_PI);
    subsecondsRotation = -subsecondsRotation;
    secondsRotation = -secondsRotation;
    
    // Clear the rendering window
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Build a cross with some lines (if not built yet):
    if ( _axis.changed ) // needs update
    {
        _axis.build(1.0f); // axis has radius 1.0
    }
    if ( _border.changed )
    {
        _border.build(0.1f, 0.5f, 60);
    }
    if ( _seconds.changed )
    {
        _seconds.build(0.85, 0.01666, 0.01666, 5);
    }
    if ( _subseconds.changed )
    {
        _subseconds.build(0.60, 0.01666, 0.01666, 5);
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

    float aspect=static_cast<float>(_w) / static_cast<float>(_h), znear=0.1f, zfar=50.0f;
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
        _axis.draw ( stransf, sproj );
    
    // Draw the border
    /*GsMat borderTranslation, borderTransform;
    borderTranslation.translation(0.0, 0.5, 0.0);
    borderTransform = stransf;
    
    _border.draw(borderTransform, sproj);*/
    
    // Compute the sub sections transformations
    /*GsMat subsecondsRotationFinal, subsecondsRot, subsecondsTranslation;
    subsecondsRot.rotz(subsecondsRotation);
    subsecondsTranslation.translation(0.0f, 0.30f, 0.0f);
    subsecondsRotationFinal = stransf * subsecondsRot * subsecondsTranslation;
    _subseconds.draw(subsecondsRotationFinal, sproj);
    
    // Compute the sub sections transformations
    GsMat secondsRotationFinal, secondsRot, secondsTranslation;
    secondsRot.rotz(secondsRotation);
    secondsTranslation.translation(0.0f, 0.425f, 0.0f);
    secondsRotationFinal = stransf * secondsRot * secondsTranslation;
    _seconds.draw(secondsRotationFinal, sproj);*/
    
    // Compute the transformation matrices for the projection!!
    GsMat shadowMatrix;
    GLfloat groundPlane[4] = {0, 1, 0, 0};
    
    GLfloat light[4] = {2, 2, 2, 1};
    
    //GsVec lightPosition = stransf * GsVec(0, 2, 2);
    //GLfloat light[4] = {lightPosition.x, lightPosition.y, lightPosition.z, 0};
    
    GLfloat dotProduct = (groundPlane[0] * light[0]) +
                         (groundPlane[1] * light[1]) +
                         (groundPlane[2] * light[2]) +
                         (groundPlane[3] * light[3]);
    
    shadowMatrix.e[0] = dotProduct - (light[0] * groundPlane[0]);
    shadowMatrix.e[4] =            - (light[0] * groundPlane[1]);
    shadowMatrix.e[8] =            - (light[0] * groundPlane[2]);
    shadowMatrix.e[12] =           - (light[0] * groundPlane[3]);
    
    shadowMatrix.e[1] =            - (light[1] * groundPlane[0]);
    shadowMatrix.e[5] = dotProduct - (light[1] * groundPlane[1]);
    shadowMatrix.e[9] =            - (light[1] * groundPlane[2]);
    shadowMatrix.e[13] =           - (light[1] * groundPlane[3]);
    
    shadowMatrix.e[2] =             - (light[2] * groundPlane[0]);
    shadowMatrix.e[6] =             - (light[2] * groundPlane[1]);
    shadowMatrix.e[10] = dotProduct - (light[2] * groundPlane[2]);
    shadowMatrix.e[14] =            - (light[2] * groundPlane[3]);
    
    shadowMatrix.e[3] =             - (light[3] * groundPlane[0]);
    shadowMatrix.e[7] =             - (light[3] * groundPlane[1]);
    shadowMatrix.e[11] =            - (light[3] * groundPlane[2]);
    shadowMatrix.e[15] = dotProduct - (light[3] * groundPlane[3]);
    
    //GsMat shadowTransform = stransf * shadowMatrix;
    //_border.draw(shadowTransform, sproj);
    
    
    // Swap buffers and draw:
    glFlush();         // flush the pipeline (usually not necessary)
    glutSwapBuffers(); // we were drawing to the back buffer, now bring it to the front
}

void AppWindow::glutIdle()
{
    redraw();
}

