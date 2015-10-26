
// Ensure the header file is included only once in multi-file projects
#ifndef APP_WINDOW_H
#define APP_WINDOW_H

# include <destructo-base/stdafx.h>
# include <destructo-base/TextureCache.h>
# include <destructo-base/ModelGroup.h>
# include <destructo-base/ModelInstance.h>

# include <destructo-base/GL3DProgram.h>
# include <destructo-base/MaterialProgram.h>

# include "GLTexturedQuad.hpp"
# include "glut_window.h"

// The functionality of your application should be implemented inside AppWindow
class AppWindow : public GlutWindow
{
    // OpenGL Resources
    TextureCache      textureCache;
    ModelGroup        modelGroup;
    MaterialProgram  *modelProgram;
    GL3DProgram      *groundProgram;
    
    // My scene objects
    Node             *sceneRoot;
    ModelInstance    *mechwarriorInstance;
    GLTexturedQuad   *ground;
    
    Node             *camera;
    Transform         cameraTransform;

    // App data:
    float             _rotx, _roty;
    glm::vec3         rotation;
    glm::vec2         viewport;

public :
    AppWindow ( const char* label, int x, int y, int w, int h );
    void initPrograms ();
    vec2 windowToScene ( const vec2& v );

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
