# include <destructo-base/OS.h>
# include "app_window.h"

AppWindow::AppWindow ( const char* label, int x, int y, int w, int h )
    : GlutWindow ( label, x, y, w, h ), sceneRoot(new Node()), modelGroup("meshes/manifest.json", textureCache)
{
    initPrograms ();
    
    sceneRoot->Id() = "Scene Root";
    
    // Create an instance of the mechwarrior model
    mechwarriorInstance = modelGroup.NewInstance("robot02");
    mechwarriorInstance->GetNode()->Id() = "Mechwarrior";
    sceneRoot->AddChild(mechwarriorInstance->GetNode());
    
    // Create a textured quad for the ground (tile the texture 50 times across its surface
    ground = new GLTexturedQuad(textureCache, "textures/terrain.png");
    ground->Build(groundProgram, 50.0);
    ground->GetNode()->LocalTransform().Scale() = vec3(100,1,100);
    ground->GetNode()->Id() = "Ground";
    sceneRoot->AddChild(ground->GetNode());
    
    // Setup the cameras
    cameraSelection = false;
    
    mechCamera = new Node();
    mechCamera->LocalTransform().Rotation() = glm::angleAxis(-glm::pi<float>(), vec3(0,1,0));
    mechCamera->LocalTransform().Translation() = glm::vec3(0,0.8,-2);
    mechCameraRotator = new Node();
    mechCameraRotator->AddChild(mechCamera);
    //mechwarriorInstance->GetNode()->FindNode("body")->AddChild(mechCameraRotator);
    
    groundCamera = new Node();
    groundCamera->LocalTransform().Translation() = vec3(1, 0.65, 1);
    groundCamera->LocalTransform().Rotation() = glm::angleAxis(glm::pi<float>() / 4.0f, vec3(0,1,0)) * glm::angleAxis(-glm::pi<float>() / 16.0f, vec3(1,0,0));
    sceneRoot->AddChild(groundCamera);
    
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
    groundProgram = new GL3DProgram("shaders/vsh_quad.glsl", "shaders/fsh_quad.glsl");
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
        case ' ': cameraSelection = !cameraSelection;
    }
}

void AppWindow::glutKeyboardUp ( unsigned char key, int x, int y )
{
    switch ( key )
    {
        
    }
}

void AppWindow::glutSpecial ( int key, int x, int y )
{
    const float incr=2.5f * (M_PI / 180.0f);
    
    switch ( key )
    {
        /*case GLUT_KEY_LEFT:      rotation.y-=incr; break;
        case GLUT_KEY_RIGHT:     rotation.y+=incr; break;
        case GLUT_KEY_UP:        rotation.x-=incr; break;
        case GLUT_KEY_DOWN:      rotation.x+=incr; break;*/
        
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
    groundProgram->Camera.SetFrustrum(glm::pi<float>() / 3.0f, viewport.x / viewport.y, 0.1f, 50.0f);
    
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
    
    // Perform all computations
    mechwarriorInstance->Update(delta, currentTime);
    
    sceneRoot->Recalculate();
    
    // Compute the camera matrix
    mat4 viewMatrix = mat4();
    if(cameraSelection)
        viewMatrix = inverse(groundCamera->TransformMatrix());
    else
    {
        mechCameraRotator->Recalculate();
        viewMatrix = inverse(mechwarriorInstance->GetTransformOfSkeletalNode("body") * mechCamera->TransformMatrix());
    }
    groundProgram->Camera.SetViewMatrix(viewMatrix);
    modelProgram->Camera.SetViewMatrix(viewMatrix);
    
    // Perform all renderering
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Render the ground
    groundProgram->UseProgram();
    ground->Draw(groundProgram);
    
    // Render the mechwarrior
    modelProgram->UseProgram();
    mechwarriorInstance->Draw(modelProgram);
    
    previousTime = currentTime;
    glutSwapBuffers();
}

void AppWindow::glutIdle()
{
    redraw();
}

