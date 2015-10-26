# include <destructo-base/OS.h>

# include <iostream>
# include <cmath>

# include "app_window.h"

AppWindow::AppWindow ( const char* label, int x, int y, int w, int h )
    : GlutWindow ( label, x, y, w, h ),
      textureCache(),
      modelGroup("meshes/manifest.json", textureCache),
      materialProgram("shaders/vsh_model.glsl", "shaders/fsh_model.glsl")
{
    initPrograms ();
    
    // Create an instance of the mechwarrior model
    mechwarriorInstance = modelGroup.NewInstance("robot02");
    
    // Add the animations as menu options
    int animationId = 0;
    addMenuEntry( "Stop Animations", animationId++);
    for( auto animation : mechwarriorInstance->GetModel()->Animations() )
        addMenuEntry(animation.second->Id().c_str(), animationId++);
    
    // Initialize some shit
    _viewaxis = true;
    _fovy = M_PI / 3.0f;
    _rotx = _roty = _rotz = 0;
    _w = w;
    _h = h;
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
#endif
     
   _prog.init_and_link ( _vertexsh, _fragsh );

   // Init my scene objects:
   _axis.init ( _prog );
 }

// mouse events are in window coordinates, but your 2D scene is in [0,1]x[0,1],
// so make here the conversion when needed
vec2 AppWindow::windowToScene ( const vec2& v )
 {
   // GsVec2 is a lighteweight class suitable to return by value:
   return vec2 ( (2.0f*(v.x/(_w))) - 1.0f,
                  1.0f - (2.0f*(v.y/(_h))) );
 }

// Called every time there is a window event
void AppWindow::glutKeyboard ( unsigned char key, int x, int y )
{
    switch ( key )
    {
        case ' ': _viewaxis = !_viewaxis; break;
        case 27 : exit(1); // Esc was pressed
    }
}

void AppWindow::glutSpecial ( int key, int x, int y )
{
    const float incr=2.5f * (M_PI / 180.0f);
    const float incf=0.05f;
    
    switch ( key )
    {
        case GLUT_KEY_LEFT:      _roty-=incr; break;
        case GLUT_KEY_RIGHT:     _roty+=incr; break;
        case GLUT_KEY_UP:        _rotx-=incr; break;
        case GLUT_KEY_DOWN:      _rotx+=incr; break;
        case GLUT_KEY_PAGE_UP:   _fovy-=incf; break;
        case GLUT_KEY_PAGE_DOWN: _fovy+=incf; break;
        case GLUT_KEY_HOME:      _fovy=(M_PI / 3.0f); _rotx=_roty=0; break;
        
        default: return; // return without rendering
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
    if(!m)
        mechwarriorInstance->Animation().Stop();
    else
    {
        auto animationId = std::next(mechwarriorInstance->GetModel()->Animations().begin(), m - 1);
        mechwarriorInstance->PlayAnimation(animationId->second->Id());
    }
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
    static double previousTime = OS::Now();
    double        currentTime  = OS::Now();
    double        delta        = currentTime - previousTime;
    
    // Any processing for the frame
    textureCache.Refresh();
    mechwarriorInstance->Update(delta, currentTime);
    
    // Define our camera matrix
    vec3 eye(0,0,2), center(0,0,0), up(0,1,0);
    mat4 camview = glm::lookAt(eye, center, up);
    
    // Define our projection transformation:
    float znear=0.1f, zfar=50.0f;
    mat4 persp = glm::perspectiveFov(_fovy, _w, _h, znear, zfar);
    mat4 sproj = persp * camview; // set final scene projection
    
    // Define our scene transformation:
    mat4 stransf = glm::eulerAngleXY(_rotx, _roty);
    
    // Build a cross with some lines (if not built yet):
    if ( _axis.changed ) // needs update
    {
        _axis.build(1.0f); // axis has radius 1.0
    }
    
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    if ( _viewaxis )
        _axis.draw ( stransf, sproj );
    
    materialProgram.UseProgram();
    
    materialProgram.Camera.SetProjectionMatrix(persp);
    materialProgram.Camera.SetViewMatrix(camview);
    materialProgram.Camera.Apply();
    
    mechwarriorInstance->GetTransform().Scale() = vec3(0.33, 0.33, 0.33);
    mechwarriorInstance->GetTransform().Rotation() = glm::angleAxis(_rotx, vec3(1,0,0)) * glm::angleAxis(_roty, vec3(0,1,0));
    mechwarriorInstance->Draw(&materialProgram);
    
    previousTime = currentTime;
    glutSwapBuffers();
}

void AppWindow::glutIdle()
{
    redraw();
}

