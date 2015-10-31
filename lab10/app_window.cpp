
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
    : GlutWindow ( label, x, y, w, h ),
    _sun(GsVec(2, 2, 2), GsColor::white, GsColor::white, GsColor::white),
    _material(GsColor(16,16,16), GsColor::blue, GsColor::cyan, 32.f),
    _texturedmaterial(GsColor::darkgray, GsColor::gray, GsColor::white, 16.0f),
    _texturedmaterialflat(GsColor::white, GsColor::white, GsColor::white, 16.0f)
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
    
    resolution = 0;
    wireframe = false;
    textured = false;
    flat = false;
    adaptive = true;
    timeWarp = 1.0;
    sizeOnScreen = 15;
}

void AppWindow::initPrograms ()
{
    // Load your shaders and link your programs here:
#ifdef WIN32
    _vertexsh.load_and_compile ( GL_VERTEX_SHADER, "../vsh_mcol_flat.glsl" );
    _fragsh.load_and_compile ( GL_FRAGMENT_SHADER, "../fsh_flat.glsl" );
    _flatvsh.load_and_compile(GL_VERTEX_SHADER, "../vsh_flat_textured.glsl");
    _flatfsh.load_and_compile(GL_FRAGMENT_SHADER, "../fsh_flat_textured.glsl");
    _lightvsh.load_and_compile(GL_VERTEX_SHADER, "../vsh_lighting.glsl");
    _lightfsh.load_and_compile(GL_FRAGMENT_SHADER, "../fsh_lighting.glsl");
    _texture.Load("../earth.bmp");
#else
	_vertexsh.load_and_compile(GL_VERTEX_SHADER, "vsh_mcol_flat.glsl");
	_fragsh.load_and_compile(GL_FRAGMENT_SHADER, "fsh_flat.glsl");
	_flatvsh.load_and_compile(GL_VERTEX_SHADER, "vsh_flat_textured.glsl");
    _flatfsh.load_and_compile(GL_FRAGMENT_SHADER, "fsh_flat_textured.glsl");
	_lightvsh.load_and_compile(GL_VERTEX_SHADER, "vsh_lighting.glsl");
    _lightfsh.load_and_compile(GL_FRAGMENT_SHADER, "fsh_lighting.glsl");
    _texture.Load("earth.bmp");
#endif

    _prog.init_and_link ( _vertexsh, _fragsh );
    _flatprog.init_and_link (  _flatvsh, _flatfsh );
    _lightprog.init_and_link ( _lightvsh, _lightfsh );
    
    // Init my scene objects:
    _axis.init ( _prog );
    _sphereRenderer.init( _lightprog );
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
      case ' ': adaptive = !adaptive; break;
      case 'q': resolution++; break;
      case 'a': resolution = (resolution > 0) ? resolution - 1 : 0; break;
      case 'z': wireframe = !wireframe; break;
      case 'x': _sphereRenderer.init(_flatprog); flat = true; break;
      case 'c': textured = !textured; break;
      case 'v': _sphereRenderer.init(_lightprog); flat = false; break;
      case 'w': _lightpos += (M_PI / 36.0f); break;
      case 's': _lightpos -= (M_PI / 36.0f); break;
      case 'e': timeWarp += 0.2; break;
      case 'd': timeWarp = (timeWarp - 0.2 > 0.2) ? timeWarp - 0.2 : 0.2; break;
      case 'r': sizeOnScreen++; break;
      case 'f': sizeOnScreen = (sizeOnScreen > 3) ? sizeOnScreen - 1 : 3; break;
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
    static double frameTime = 0.0;
    static high_resolution_clock::time_point previousTime = high_resolution_clock::now();
    high_resolution_clock::time_point now = high_resolution_clock::now();
    duration<double> frameTimeDelta = duration_cast<duration<double>>(now - previousTime);
    previousTime = now;
    frameTime += timeWarp * frameTimeDelta.count();
    
    
    // Clear the rendering window
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
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
    stransf = rx*ry; // set the scene transformation matrix
    _sun.pos = stransf * GsVec(5.0f*sinf(_lightpos), 5.0f, 5.0f*cosf(_lightpos));

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
    //if ( _viewaxis )
    //    _axis.draw ( stransf, sproj );
    
    // Select the material
    Material *material = &_material;
    if(textured && flat)
        material = &_texturedmaterialflat;
    else if(textured)
        material = &_texturedmaterial;
    
    (textured ? _texture : _white).Bind(GL_TEXTURE0);
    
    // Generate 3 rows x 5 columns of spheres
    for(int j = -1; j <= 1; j++)
    {
        for(int i = -2; i <= 2; i++)
        {
            // Compute the sphere position
            GsMat translate;
            translate.translation(0.60f * float(i), 0.60f * float(j), (3.0f*(sinf(frameTime + float(j) * 0.50f + float(i) * 0.33f))) - 2.5f);
            translate = stransf * translate;
            
            // Select an LOD
            const SoSphere *sphere = NULL;
            if(adaptive)
            {
                // Get the distance from the camera
                GsVec position = translate * GsVec(0,0,0);
                float distance = dist(position, eye);
                
                // Select the proper LOD
                int lod = 0;
                sphere = GetSphereLOD(0);
                while (GetPixelsOnScreen(sphere->GetCrossDistance(), distance, persp) > sizeOnScreen)
                {
                    sphere = GetSphereLOD(++lod);
                }
            }
            else
            {
                sphere = GetSphereLOD(resolution);
            }
            
            // Renderer the spheres
            _sphereRenderer.SetSphere(sphere);
            _sphereRenderer.draw(translate, sproj, _sun, *material);
        }
    }
    
    // Swap buffers and draw:
    glFlush();         // flush the pipeline (usually not necessary)
    glutSwapBuffers(); // we were drawing to the back buffer, now bring it to the front
}

const SoSphere* AppWindow::GetSphereLOD(int resolution)
{
    // Get the LOD for the sphere
    auto lod = _sphereLODs.find(resolution);
    if(lod == _sphereLODs.end())
    {
        std::cout << "Constructing Sphere LOD level " << resolution << std::endl;
        SoSphere *sphere = new SoSphere;
        sphere->init();
        sphere->build(0.25f, resolution);
        lod = _sphereLODs.insert(std::make_pair(resolution, (const SoSphere *) sphere)).first;
    }
    
    return lod->second;
}

int AppWindow::GetPixelsOnScreen(float length, float distance, GsMat &persp)
{
    GsVec a(-0.5f * length, 0.0f, -distance);
    GsVec b( 0.5f * length, 0.0f, -distance);
    GsVec ta = persp * a;
    GsVec tb = persp * b;
    
    GsVec2 sa, sb;
    sa.x = ((ta.x + 1.0f) / 2.0f) * float(_w);
    sa.y = ((ta.y + 1.0f) / 2.0f) * float(_h);
    sb.x = ((tb.x + 1.0f) / 2.0f) * float(_w);
    sb.y = ((tb.y + 1.0f) / 2.0f) * float(_h);
    
    return dist(sa,sb);
}

void AppWindow::glutIdle()
{
  redraw();
}

