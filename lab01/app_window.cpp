#include <iostream>
#include "app_window.h"

#include <iostream>
#include <ctime>
#include <ratio>
#include <algorithm>
#include <functional>
#include <cmath>

using namespace std::chrono;

AppWindow::AppWindow ( const char* label, int x, int y, int w, int h )
    : GlutWindow ( label, x, y, w, h ), _markc(GsColor::yellow), _mark(0.0, -0.9), _w(w), _h(h), multiplier(1.0), frameTime(0.0), pointsLastUpdate(0.0)
{
    initPrograms();
    addMenuEntry( "Option 0", evOption0 );
    addMenuEntry( "Option 1", evOption1 );

    previousTime = high_resolution_clock::now();
}

void AppWindow::initPrograms()
{
    // We are following the OpenGL API version 4, 
    // see docs at: http://www.opengl.org/sdk/docs/man/

    // Load your shaders and link your programs here:
    _vertexsh.load_and_compile ( GL_VERTEX_SHADER, "vsh_mcol_flat.glsl" );
    _fragsh.load_and_compile ( GL_FRAGMENT_SHADER, "fsh_flat.glsl" );
    _prog.init_and_link ( _vertexsh, _fragsh );

    // Define buffers needed for each of your OpenGL objects:
    // (here they all use the same definitions because we are using the same shaders)
    // Program for rendering triangles:
    { 
        GlObjects& o = _tris;
        o.set_program ( _prog );
        o.gen_vertex_arrays ( 1 );
        o.gen_buffers ( 2 );
        o.uniform_locations ( 2 );
        o.uniform_location ( 0, "vTransf" );
        o.uniform_location ( 1, "vProj" );
        o.uniform_location ( 2, "fTime" );
    }

    // Define buffers needed for each of your OpenGL objects:
    {
        GlObjects& o = _pts;
        o.set_program ( _prog );
        o.gen_vertex_arrays ( 1 );
        o.gen_buffers ( 2 );
        o.uniform_locations ( 2 );
        o.uniform_location ( 0, "vTransf" );
        o.uniform_location ( 1, "vProj" );
    }

    // Define buffers needed for each of your OpenGL objects:
    { 
        GlObjects& o = _lines;
        o.set_program ( _prog );
        o.gen_vertex_arrays ( 1 );
        o.gen_buffers ( 2 );
        o.uniform_locations ( 2 );
        o.uniform_location ( 0, "vTransf" );
        o.uniform_location ( 1, "vProj" );
    }
}

// mouse events are in window coordinates, but your scene is in [0,1]x[0,1],
// so make here the conversion when needed
GsVec2 AppWindow::windowToScene ( const GsVec2& v )
{
    // GsVec2 is a lighteweight class suitable to return by value:
    return GsVec2 ( (2.0f*(v.x/float(_w))) - 1.0f, 1.0f - (2.0f*(v.y/float(_h))) );
}

// Called every time there is a window event
void AppWindow::glutKeyboard ( unsigned char key, int x, int y )
{
    struct MovingPoint p;

    switch ( key )
    {
    case ' ':
        // Add a projectile
        p.position = _mark;
        p.velocity = GsVec2(0.0, 1.25);
        _ptinstances.push_back(p);
        break;

    // Esc was pressed
    case 27: 
        exit(1);
    }
}

void AppWindow::glutSpecial ( int key, int x, int y )
{
    const float incx=0.02f;
    const float incy=0.02f;

    switch ( key )
    { 
    case GLUT_KEY_LEFT:
        _mark.x -= incx * multiplier;
        _lines.changed = 1;
        break;

    case GLUT_KEY_RIGHT:
        _mark.x+=incx * multiplier;
        _lines.changed = 1;
        break;

    case GLUT_KEY_UP:
        multiplier+=0.5;
        break;

    case GLUT_KEY_DOWN: 
        multiplier = ((multiplier - 0.5) < 0.5) ? 0.5 : multiplier - 0.5; 
        break;
    }
}

void AppWindow::glutMouse ( int b, int s, int x, int y )
{
    //_mark = windowToScene ( GsVec2(x,y) );
    //_lines.changed = 1;
}

void AppWindow::glutMotion ( int x, int y )
{
    //_mark = windowToScene ( GsVec2(x,y) );
    //_lines.changed = 1;
}

void AppWindow::glutMenu ( int m )
{
    std::cout<<"Menu Event: "<<m<<std::endl;
}

void AppWindow::glutReshape ( int w, int h )
{
    // Define that OpenGL should use the whole window for rendering
    glViewport( 0, 0, w, h );
    _w = w; 
    _h = h;
}

// here we will redraw the scene according to the current state of the application.
void AppWindow::buildObjects(double frameTime, double frameDelta)
{
    // Clear the rendering window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Define a cross with some lines:
    if (_lines.changed) // needs update
    {
        _linecoords.clear(); _linecolors.clear();

        _linecoords.push_back(GsVec(_mark.x - 0.075, _mark.y - 0.05, 0.0)); _linecolors.push_back(_markc);  
        _linecoords.push_back(GsVec(_mark.x + 0.075, _mark.y - 0.05, 0.0)); _linecolors.push_back(_markc);  

        _linecoords.push_back(GsVec(_mark.x + 0.0, _mark.y + 0.00, 0.0));   _linecolors.push_back(_markc);  
        _linecoords.push_back(GsVec(_mark.x + 0.075, _mark.y - 0.05, 0.0)); _linecolors.push_back(_markc);
        _linecoords.push_back(GsVec(_mark.x + 0.0, _mark.y + 0.00, 0.0));   _linecolors.push_back(_markc);      
        _linecoords.push_back(GsVec(_mark.x - 0.075, _mark.y - 0.05, 0.0)); _linecolors.push_back(_markc);

        // send data to OpenGL buffers:
        glBindBuffer(GL_ARRAY_BUFFER, _lines.buf[0]);
        glBufferData(GL_ARRAY_BUFFER, _linecoords.size() * 3 * sizeof(float), &_linecoords[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, _lines.buf[1]);
        glBufferData(GL_ARRAY_BUFFER, _linecolors.size() * 4 * sizeof(gsbyte), &_linecolors[0], GL_STATIC_DRAW);
        // mark that data does not need more changes:
        _lines.changed = 0;
    }

    // Define some triangles:
    if (_tris.changed) // needs update
    {
        _tricoords.clear(); _tricolors.clear();

        // "N" first stroke
        {
            _tricoords.push_back(GsVec(-0.95, 0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.90, 0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.90, -0.5, 0.0)); _tricolors.push_back(GsColor::red);

            _tricoords.push_back(GsVec(-0.90, -0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.95, -0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.95, 0.5, 0.0)); _tricolors.push_back(GsColor::red);
        }

        // "N" diagonal stroke
        {
            _tricoords.push_back(GsVec(-0.95, 0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.90, 0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.55, -0.5, 0.0)); _tricolors.push_back(GsColor::red);

            _tricoords.push_back(GsVec(-0.55, -0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.60, -0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.95, 0.5, 0.0)); _tricolors.push_back(GsColor::red);
        }

        // "N" final stroke
        {
            _tricoords.push_back(GsVec(-0.60, 0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.55, 0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.55, -0.5, 0.0)); _tricolors.push_back(GsColor::red);

            _tricoords.push_back(GsVec(-0.55, -0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.60, -0.5, 0.0)); _tricolors.push_back(GsColor::red);
            _tricoords.push_back(GsVec(-0.60, 0.5, 0.0)); _tricolors.push_back(GsColor::red);
        }

        // "A" left diagonal"
        {
            _tricoords.push_back(GsVec(-0.275, 0.5, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.225, 0.5, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.40, -0.5, 0.0)); _tricolors.push_back(GsColor::green);

            _tricoords.push_back(GsVec(-0.40, -0.5, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.45, -0.5, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.275, 0.5, 0.0)); _tricolors.push_back(GsColor::green);
        }

        // "A" right diagonal"
        {
            _tricoords.push_back(GsVec(-0.275, 0.5, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.225, 0.5, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.05, -0.5, 0.0)); _tricolors.push_back(GsColor::green);

            _tricoords.push_back(GsVec(-0.05, -0.5, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.10, -0.5, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.275, 0.5, 0.0)); _tricolors.push_back(GsColor::green);
        }

        // "A" center bar"
        {
            _tricoords.push_back(GsVec(-0.35, 0.0, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.15, 0.0, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.15, -0.05, 0.0)); _tricolors.push_back(GsColor::green);

            _tricoords.push_back(GsVec(-0.15, -0.05, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.35, -0.05, 0.0)); _tricolors.push_back(GsColor::green);
            _tricoords.push_back(GsVec(-0.35, 0.0, 0.0)); _tricolors.push_back(GsColor::green);
        }

        // "T" verticle bar"
        {
            _tricoords.push_back(GsVec(0.225, 0.5, 0.0)); _tricolors.push_back(GsColor::blue);
            _tricoords.push_back(GsVec(0.275, 0.5, 0.0)); _tricolors.push_back(GsColor::blue);
            _tricoords.push_back(GsVec(0.275, -0.5, 0.0)); _tricolors.push_back(GsColor::blue);

            _tricoords.push_back(GsVec(0.275, -0.5, 0.0)); _tricolors.push_back(GsColor::blue);
            _tricoords.push_back(GsVec(0.225, -0.5, 0.0)); _tricolors.push_back(GsColor::blue);
            _tricoords.push_back(GsVec(0.225, 0.5, 0.0)); _tricolors.push_back(GsColor::blue);
        }

        // "T" horizontal bar"
        {
            _tricoords.push_back(GsVec(0.05, 0.5, 0.0)); _tricolors.push_back(GsColor::blue);
            _tricoords.push_back(GsVec(0.45, 0.5, 0.0)); _tricolors.push_back(GsColor::blue);
            _tricoords.push_back(GsVec(0.45, 0.45, 0.0)); _tricolors.push_back(GsColor::blue);

            _tricoords.push_back(GsVec(0.45, 0.45, 0.0)); _tricolors.push_back(GsColor::blue);
            _tricoords.push_back(GsVec(0.05, 0.45, 0.0)); _tricolors.push_back(GsColor::blue);
            _tricoords.push_back(GsVec(0.05, 0.5, 0.0)); _tricolors.push_back(GsColor::blue);
        }

        // "E" verticle bar
        {
            _tricoords.push_back(GsVec(0.55, 0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.60, 0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.60, -0.5, 0.0)); _tricolors.push_back(GsColor::yellow);

            _tricoords.push_back(GsVec(0.60, -0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.55, -0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.55, 0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
        }

        // "E" horizontal top
        {
            _tricoords.push_back(GsVec(0.55, 0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.95, 0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.95, 0.45, 0.0)); _tricolors.push_back(GsColor::yellow);

            _tricoords.push_back(GsVec(0.95, 0.45, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.55, 0.45, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.55, 0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
        }

        // "E" horizontal middle
        {
            _tricoords.push_back(GsVec(0.55, 0.025, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.95, 0.025, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.95, -0.025, 0.0)); _tricolors.push_back(GsColor::yellow);

            _tricoords.push_back(GsVec(0.95, -0.025, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.55, -0.025, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.55, 0.025, 0.0)); _tricolors.push_back(GsColor::yellow);
        }

        // "E" horizontal middle
        {
            _tricoords.push_back(GsVec(0.55, -0.45, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.95, -0.45, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.95, -0.5, 0.0)); _tricolors.push_back(GsColor::yellow);

            _tricoords.push_back(GsVec(0.95, -0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.55, -0.5, 0.0)); _tricolors.push_back(GsColor::yellow);
            _tricoords.push_back(GsVec(0.55, -0.45, 0.0)); _tricolors.push_back(GsColor::yellow);
        }

        // send data to OpenGL buffers:
        glBindBuffer(GL_ARRAY_BUFFER, _tris.buf[0]);
        glBufferData(GL_ARRAY_BUFFER, _tricoords.size() * 3 * sizeof(float), &_tricoords[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, _tris.buf[1]);
        glBufferData(GL_ARRAY_BUFFER, _tricolors.size() * 4 * sizeof(gsbyte), &_tricolors[0], GL_STATIC_DRAW);

        // mark that data does not need more changes:
        _tris.changed = 0;
    }

    // Always update points (always changing)
    {
        _ptcoords.clear();
        _ptcolors.clear();

        // Create new points every 1/4 of a second of simulation time
        if ((frameTime - pointsLastUpdate) > 0.25)
        {
            // Generate random amount
            int count = 16;
            for (int i = 0; i < count; i++)
            {
                double x = (2 * (static_cast<double>(rand()) / static_cast<double>(RAND_MAX))) - 1.0;
                struct MovingPoint p;
                p.position = GsVec2(x, 1.0);
                p.velocity = GsVec2(0.0, -0.33);
                _ptinstances.push_back(p);
            }
            pointsLastUpdate = frameTime;
        }

        // Erase point which have traveled off of the screen
        const std::function<bool (MovingPoint)> predicate = [] (MovingPoint p)
        {
            return (std::abs(p.position.x) > 1.0 || std::abs(p.position.y) > 1.0);
        };

        _ptinstances.erase(std::remove_if(_ptinstances.begin(), _ptinstances.end(), predicate), _ptinstances.end());

        // Update position of points and add to GPU buffer
        for (std::vector<MovingPoint>::iterator point = _ptinstances.begin(); point != _ptinstances.end(); point++)
        {
            point->position += point->velocity * frameDelta;

            _ptcoords.push_back(point->position);
            _ptcolors.push_back(GsColor::white);
        }

        // send data to OpenGL buffers:
        glBindBuffer(GL_ARRAY_BUFFER, _pts.buf[0]);
        if (_ptcoords.size() > 0)
        {
            glBufferData(GL_ARRAY_BUFFER, _ptcoords.size() * 3 * sizeof(float), &_ptcoords[0], GL_DYNAMIC_DRAW);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
        }

        glBindBuffer(GL_ARRAY_BUFFER, _pts.buf[1]);
        if (_ptcolors.size() > 0)
        {
            glBufferData(GL_ARRAY_BUFFER, _ptcolors.size() * 4 * sizeof(gsbyte), &_ptcolors[0], GL_DYNAMIC_DRAW);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
        }
    }
}

// Use the idle function to redraw the window, note - i've enabled vsync so it doesnt kill performance 
void AppWindow::glutIdle()
{
    redraw();
}

void AppWindow::glutDisplay ()
{ 
    // Frame delta/time
    high_resolution_clock::time_point now = high_resolution_clock::now();
    duration<double> frameDeltaRaw = duration_cast<duration<double>>(now - previousTime);
    previousTime = now;
    
    double frameDelta = frameDeltaRaw.count() * multiplier;
    frameTime += frameDelta;

    // Clear the rendering window
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Update objects if needed:
    buildObjects(frameTime, frameDelta);

    // Define some identity transformations; our shaders require them but in this
    // example support code we do not need to use them, so just let them be GsMat::id:
    GsMat vtransf(GsMat::id), vproj(GsMat::id); // transformations

    // Draw Lines:
    glUseProgram ( _lines.prog );
    glBindVertexArray ( _lines.va[0] );

    glBindBuffer ( GL_ARRAY_BUFFER, _lines.buf[0] ); // positions
    glEnableVertexAttribArray ( 0 );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    glBindBuffer ( GL_ARRAY_BUFFER, _lines.buf[1] ); // colors
    glEnableVertexAttribArray ( 1 );
    glVertexAttribPointer ( 1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );

    glUniformMatrix4fv ( _lines.uniloc[0], 1, GL_FALSE, vtransf.e );
    glUniformMatrix4fv ( _lines.uniloc[1], 1, GL_FALSE, vproj.e );

    glDrawArrays ( GL_LINES, 0, _linecoords.size() );
    // (see documentation at: https://www.opengl.org/sdk/docs/man/html/glDrawArrays.xhtml)

    // Draw Points:
    glUseProgram ( _pts.prog );
    glBindVertexArray ( _pts.va[0] );

    glBindBuffer ( GL_ARRAY_BUFFER, _pts.buf[0] ); // positions
    glEnableVertexAttribArray ( 0 );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    glBindBuffer ( GL_ARRAY_BUFFER, _pts.buf[1] ); // colors
    glEnableVertexAttribArray ( 1 );
    glVertexAttribPointer ( 1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );

    glUniformMatrix4fv ( _pts.uniloc[0], 1, GL_FALSE, vtransf.e );
    glUniformMatrix4fv ( _pts.uniloc[1], 1, GL_FALSE, vproj.e );

    glDrawArrays ( GL_POINTS, 0, _ptcoords.size() );

    // Draw Triangles:
    glUseProgram ( _tris.prog );
    glBindVertexArray ( _tris.va[0] );

    glBindBuffer ( GL_ARRAY_BUFFER, _tris.buf[0] ); // positions
    glEnableVertexAttribArray ( 0 );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    glBindBuffer ( GL_ARRAY_BUFFER, _tris.buf[1] ); // colors
    glEnableVertexAttribArray ( 1 );
    glVertexAttribPointer ( 1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );

    glUniformMatrix4fv ( _tris.uniloc[0], 1, GL_FALSE, vtransf.e );
    glUniformMatrix4fv ( _tris.uniloc[1], 1, GL_FALSE, vproj.e );
    glUniform1f ( _tris.uniloc[2], frameTime * 17.5);

    glDrawArrays ( GL_TRIANGLES, 0, _tricoords.size() );

    // Swap buffers and draw
    glutSwapBuffers(); // we were drawing to the back buffer, now bring it to the front
}

