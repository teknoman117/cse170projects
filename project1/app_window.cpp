# include <destructo-base/OS.h>
# include "app_window.h"

namespace
{
    // Compute the shadow matrix (adapted from ftp://ftp.sgi.com/opengl/contrib/blythe/advanced99/notes/node192.html)
    void ComputeShadowMatrixPointLight(mat4& shadowMat, vec4 light, vec4 ground)
    {
        mat4 dotmat = mat4(1.0) * glm::dot(light, ground);
        
        shadowMat[0] = light * ground[0];
        shadowMat[1] = light * ground[1];
        shadowMat[2] = light * ground[2];
        shadowMat[3] = light * ground[3];
        
        shadowMat = dotmat - shadowMat;
    }
}

AppWindow::AppWindow ( const char* label, int x, int y, int w, int h )
    : GlutWindow ( label, x, y, w, h ), sceneRoot(new Node()), modelGroup("meshes/manifest.json", textureCache), mechwarriorRate(vec2(0.0f,0.0f)), mechwarriorRotateRate(0.0f)
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
    ground->GetNode()->LocalTransform().Translation() = vec3(0,-0.001,0);
    ground->GetNode()->Id() = "Ground";
    sceneRoot->AddChild(ground->GetNode());
    
    // Setup the cameras
    cameraSelection = false;
    mechCamera = new Node();
    mechCamera->LocalTransform().Rotation() = glm::angleAxis(-glm::pi<float>(), vec3(0,1,0));
    mechCamera->LocalTransform().Translation() = glm::vec3(0,0.35,-0.95);
    mechCameraRotator = new Node();
    mechCameraRotator->LocalTransform().Translation() = glm::vec3(0,0.5,0);
    mechCameraRotator->AddChild(mechCamera);
    
    groundCamera = new Node();
    groundCamera->LocalTransform().Translation() = vec3(1, 0.65, 1);
    groundCamera->LocalTransform().Rotation() = glm::angleAxis(glm::pi<float>() / 4.0f, vec3(0,1,0)) * glm::angleAxis(-glm::pi<float>() / 16.0f, vec3(1,0,0));
    sceneRoot->AddChild(groundCamera);
    
    // Initial state for some tracking
    keys['z'] = false;
    keys['x'] = false;
    specials[GLUT_KEY_UP] = false;
    specials[GLUT_KEY_RIGHT] = false;
    specials[GLUT_KEY_LEFT] = false;
    specials[GLUT_KEY_DOWN] = false;
    
    manualPoseAnimation = new AnimationSource;
    manualPoseAnimation->Bind(mechwarriorInstance->GetModel()->Skeleton());
    manualPoseAnimation->Update(0, 0);
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
    
    // Setup the shadow matrix for the light
    mat4 shadowMatrix;
    ComputeShadowMatrixPointLight(shadowMatrix, vec4(0,3,0,1), glm::vec4(0,1,0,0));
    modelProgram->SetShadowMatrix(shadowMatrix);
    groundProgram->SetShadowMatrix(shadowMatrix);
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
    // Check if we should trigger oneshot
    auto tracker = keys.find(key);
    if(tracker == keys.end() || !tracker->second)
    {
        keys[key] = true;
        OnKeyDown(key);
    }
}

void AppWindow::glutKeyboardUp ( unsigned char key, int x, int y )
{
    keys[key] = false;
    
    if(!keys['z'] && !keys['x'])
        mechwarriorRotateRate = 0.0f;
    else if(keys['z'])
        mechwarriorRotateRate = glm::pi<float>() / 4.0f;
    else if(keys['x'])
        mechwarriorRotateRate = -glm::pi<float>() / 4.0f;
    
    if(!keys['q'] && !keys['a'])
        mechwarriorBody.x = 0.0f;
    else if(keys['q'])
        mechwarriorBody.x = glm::pi<float>() / 3.0f;
    else if(keys['a'])
        mechwarriorBody.x = -glm::pi<float>() / 3.0f;
    
    if(!keys['w'] && !keys['s'])
        mechwarriorArm.x = 0.0f;
    else if(keys['w'])
        mechwarriorArm.x = glm::pi<float>() / 6.0f;
    else if(keys['s'])
        mechwarriorArm.x = -glm::pi<float>() / 6.0f;
    
    if(!keys['e'] && !keys['d'])
        mechwarriorLegs[0].x = 0.0f;
    else if(keys['e'])
        mechwarriorLegs[0].x = glm::pi<float>() / 10.0f;
    else if(keys['d'])
        mechwarriorLegs[0].x = -glm::pi<float>() / 10.0f;
    
    if(!keys['r'] && !keys['f'])
        mechwarriorLegs[1].x = 0.0f;
    else if(keys['r'])
        mechwarriorLegs[1].x = glm::pi<float>() / 10.0f;
    else if(keys['f'])
        mechwarriorLegs[1].x = -glm::pi<float>() / 10.0f;
    
    if(!keys['t'] && !keys['g'])
        mechwarriorLegs[2].x = 0.0f;
    else if(keys['t'])
        mechwarriorLegs[2].x = glm::pi<float>() / 10.0f;
    else if(keys['g'])
        mechwarriorLegs[2].x = -glm::pi<float>() / 10.0f;
    
}

void AppWindow::OnKeyDown(unsigned char key)
{
    switch ( key )
    {
        case 27 : exit(1); // Esc was pressed
        case ' ': cameraSelection = !cameraSelection; break;
        case 'z':
            mechwarriorRotateRate = glm::pi<float>() / 4.0f;
            break;
        case 'x':
            mechwarriorRotateRate = -glm::pi<float>() / 4.0f;
            break;
        case 'q':
            mechwarriorBody.x = glm::pi<float>() / 4.0f;
            break;
        case 'a':
            mechwarriorBody.x = -glm::pi<float>() / 4.0f;
            break;
        case 'w':
            mechwarriorArm.x = glm::pi<float>() / 6.0f;
            break;
        case 's':
            mechwarriorArm.x = -glm::pi<float>() / 6.0f;
            break;
        case 'e':
            //if(!(fabs(mechwarriorRate.x) > 0.0f || fabs(mechwarriorRate.y) > 0.0f))
                
            mechwarriorLegs[0].x = glm::pi<float>() / 10.0f;
            break;
        case 'd':
            mechwarriorLegs[0].x = -glm::pi<float>() / 10.0f;
            break;
        case 'r':
            mechwarriorLegs[1].x = glm::pi<float>() / 10.0f;
            break;
        case 'f':
            mechwarriorLegs[1].x = -glm::pi<float>() / 10.0f;
            break;
        case 't':
            mechwarriorLegs[2].x = glm::pi<float>() / 10.0f;
            break;
        case 'g':
            mechwarriorLegs[2].x = -glm::pi<float>() / 10.0f;
            break;
    }
}

void AppWindow::glutSpecial ( int key, int x, int y )
{
    auto tracker = specials.find(key);
    if(tracker == specials.end() || !tracker->second)
    {
        specials[key] = true;
        OnSpecialDown(key);
    }
}

void AppWindow::glutSpecialUp ( int key, int x, int y )
{
    specials[key] = false;
    
    switch(key)
    {
        case GLUT_KEY_DOWN:
        case GLUT_KEY_UP:
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            mechwarriorAnimationControl();
            break;
    }
}

void AppWindow::OnSpecialDown(int special)
{
    switch(special)
    {
        case GLUT_KEY_DOWN:
        case GLUT_KEY_UP:
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            mechwarriorAnimationControl();
            break;
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

void AppWindow::mechwarriorAnimationControl()
{
    static int currentState = -1;
    
    if(specials[GLUT_KEY_UP])
    {
        mechwarriorRate = vec2(0.65,0);
        if(currentState != GLUT_KEY_UP)
            mechwarriorInstance->PlayAnimation("walking");
        currentState = GLUT_KEY_UP;
    } else if(specials[GLUT_KEY_DOWN])
    {
        mechwarriorRate = vec2(-0.40,0);
        if(currentState != GLUT_KEY_DOWN)
            mechwarriorInstance->PlayAnimation("backpedal");
        currentState = GLUT_KEY_DOWN;
    } else if(specials[GLUT_KEY_LEFT])
    {
        mechwarriorRate = vec2(0,0.4);
        if(currentState != GLUT_KEY_LEFT)
            mechwarriorInstance->PlayAnimation("strafe_left");
        currentState = GLUT_KEY_LEFT;
    } else if(specials[GLUT_KEY_RIGHT])
    {
        mechwarriorRate = vec2(0,-0.4);
        if(currentState != GLUT_KEY_RIGHT)
            mechwarriorInstance->PlayAnimation("strafe_right");
        currentState = GLUT_KEY_RIGHT;
    } else
    {
        mechwarriorRate = vec2(0,0);
        if(currentState != -1)
            mechwarriorInstance->PlayNullAnimation();
        currentState = -1;
    }
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
    
    // Update mechwarrior manual animation goals
    mechwarriorBody.y += mechwarriorBody.x * (float) delta;
    mechwarriorArm.y += mechwarriorArm.x * (float) delta;
    mechwarriorLegs[0].y += mechwarriorLegs[0].x * (float) delta;
    mechwarriorLegs[1].y += mechwarriorLegs[1].x * (float) delta;
    mechwarriorLegs[2].y += mechwarriorLegs[2].x * (float) delta;
    
    // Update mechwarrior global motion
    mechwarriorInstance->Update(delta, currentTime);
    if(fabs(mechwarriorRate.x) > 0.0f || fabs(mechwarriorRate.y) > 0.0f)
    {
        mat4 currentUpdate = glm::rotate(mechwarriorRotateRate * (float) delta, vec3(0.0f,1.0f,0.0f)) * glm::translate(vec3(mechwarriorRate.y, 0, mechwarriorRate.x) * (float) delta);
        mat4 currentTransform = mechwarriorInstance->GetNode()->LocalTransform().TransformMatrix();
        mechwarriorInstance->GetNode()->LocalTransform().SetTransformMatrix(currentTransform * currentUpdate);
        
        for(int i = 0; i < 3; i++)
            mechwarriorLegs[i].y = 0.0f;
    }
    
    // Update manual animation goals for all animations
    mechwarriorInstance->Animation()->Skeleton()->FindNode("body")->LocalTransform().Rotation() *= glm::angleAxis(mechwarriorBody.y, vec3(0,1,0));
    mechwarriorInstance->Animation()->Skeleton()->FindNode("lf_elbow")->LocalTransform().Rotation() *= glm::angleAxis(mechwarriorArm.y, vec3(1,0,0));
    mechwarriorInstance->Animation()->Skeleton()->FindNode("rt_elbow")->LocalTransform().Rotation() *= glm::angleAxis(mechwarriorArm.y, vec3(1,0,0));
    mechwarriorInstance->Animation()->Skeleton()->FindNode("rt_thigh")->LocalTransform().Rotation() *= glm::angleAxis(mechwarriorLegs[0].y, vec3(1,0,0));
    mechwarriorInstance->Animation()->Skeleton()->FindNode("lf_thigh")->LocalTransform().Rotation() *= glm::angleAxis(mechwarriorLegs[0].y, vec3(1,0,0));
    mechwarriorInstance->Animation()->Skeleton()->FindNode("rt_knee2")->LocalTransform().Rotation() *= glm::angleAxis(mechwarriorLegs[1].y, vec3(0,0,1));
    mechwarriorInstance->Animation()->Skeleton()->FindNode("lf_knee2")->LocalTransform().Rotation() *= glm::angleAxis(mechwarriorLegs[1].y, vec3(0,0,1));
    mechwarriorInstance->Animation()->Skeleton()->FindNode("rt_ankle")->LocalTransform().Rotation() *= glm::angleAxis(mechwarriorLegs[2].y, vec3(1,0,0));
    mechwarriorInstance->Animation()->Skeleton()->FindNode("lf_ankle")->LocalTransform().Rotation() *= glm::angleAxis(mechwarriorLegs[2].y, vec3(1,0,0));
    mechwarriorInstance->Animation()->Skeleton()->Recalculate();
    
    // Update scene graph
    sceneRoot->Recalculate();
    
    // Compute the camera matrix
    mat4 viewMatrix = mat4();
    if(cameraSelection)
        viewMatrix = inverse(groundCamera->TransformMatrix());
    else
    {
        mechCameraRotator->Recalculate();
        viewMatrix = inverse(mechwarriorInstance->GetNode()->TransformMatrix() * mechCamera->TransformMatrix());
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
    modelProgram->SetShadowMode(false);
    mechwarriorInstance->Draw(modelProgram);
    modelProgram->SetShadowMode(true);
    mechwarriorInstance->Draw(modelProgram);
    
    previousTime = currentTime;
    glutSwapBuffers();
}

void AppWindow::glutIdle()
{
    redraw();
}

