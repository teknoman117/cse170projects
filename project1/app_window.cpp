# include <destructo-base/OS.h>

# include <iostream>
# include <cmath>

# include "app_window.h"

AppWindow::AppWindow ( const char* label, int x, int y, int w, int h )
    : GlutWindow ( label, x, y, w, h ), modelGroup("meshes/manifest.json", textureCache)
{
    initPrograms ();
    
    // Create an instance of the mechwarrior model
    mechwarriorInstance = modelGroup.NewInstance("robot02");
    
    // Add the animations as menu options
    int animationId = 0;
    addMenuEntry( "Stop Animations", animationId++);
    for( auto animation : mechwarriorInstance->GetModel()->Animations() )
        addMenuEntry(animation.second->Id().c_str(), animationId++);
}

void AppWindow::initPrograms ()
{
    // Load your shaders and link your programs here:
#ifdef WIN32

#else

#endif
    
    // Load the program for the model shader
    modelProgram = new MaterialProgram("shaders/vsh_model.glsl", "shaders/fsh_model.glsl");
}

// mouse events are in window coordinates, but your 2D scene is in [0,1]x[0,1],
// so make here the conversion when needed
vec2 AppWindow::windowToScene ( const vec2& v )
{
    // GsVec2 is a lighteweight class suitable to return by value:
    return vec2 ( (2.0f*(v.x/(viewport.x))) - 1.0f,
                   1.0f - (2.0f*(v.y/(viewport.y))) );
}

// Called every time there is a window event
void AppWindow::glutKeyboard ( unsigned char key, int x, int y )
{
    switch ( key )
    {
        case 27 : exit(1); // Esc was pressed
    }
}

void AppWindow::glutSpecial ( int key, int x, int y )
{
    const float incr=2.5f * (M_PI / 180.0f);
    
    switch ( key )
    {
        case GLUT_KEY_LEFT:      rotation.y-=incr; break;
        case GLUT_KEY_RIGHT:     rotation.y+=incr; break;
        case GLUT_KEY_UP:        rotation.x-=incr; break;
        case GLUT_KEY_DOWN:      rotation.x+=incr; break;
        
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
    // Update the projection matrix with the new viewport information
    viewport = vec2(w,h);
    modelProgram->Camera.SetFrustrum(glm::pi<float>() / 3.0f, viewport.x / viewport.y, 0.1f, 50.0f);
    
    // Define that OpenGL should use the whole window for rendering
    glViewport( 0, 0, w, h );
}

// here we will redraw the scene according to the current state of the application.
void AppWindow::glutDisplay ()
{
    // Refesh the texture cache
    textureCache.Refresh();
    
    // Compute the frame delta
    static double previousTime = OS::Now();
    double        currentTime  = OS::Now();
    double        delta        = currentTime - previousTime;

    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    modelProgram->UseProgram();
    
    modelProgram->Camera.SetCameraPosition(vec3(0,0,2), vec3(0,0,0), vec3(0,1,0));
    modelProgram->Camera.Apply();
    
    mechwarriorInstance->Update(delta, currentTime);
    mechwarriorInstance->GetTransform().Rotation() = glm::angleAxis(rotation.x, vec3(1,0,0)) *
                                                     glm::angleAxis(rotation.y, vec3(0,1,0));
    mechwarriorInstance->Draw(modelProgram);
    
    previousTime = currentTime;
    glutSwapBuffers();
}

void AppWindow::glutIdle()
{
    redraw();
}

