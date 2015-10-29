
# include <iostream>
# include <gsim/gs.h>
# include "app_window.h"
#include "curve_eval.h"

AppWindow::AppWindow ( const char* label, int x, int y, int w, int h )
          :GlutWindow ( label, x, y, w, h )
 {
   initPrograms ();
   addMenuEntry ( "Option 0", evOption0 );
   addMenuEntry ( "Option 1", evOption1 );
   _viewaxis = true;
   _fovy = GS_TORAD(60.0f);
   _rotx = _roty = 0;
   _w = w;
   _h = h;
   _pickprec = 0.035f;
   _poly.select ( -1, _pickprec );
   _oktodrag = false;

   _eye.set(0,0,2);
   _center.set(0,0,0);
   _up.set(0,1,0);
   _aspect=1.0f; _znear=0.1f; _zfar=50.0f;
   _updated = true;
   _curveType = None;
   _autoUpdate3D = true;
   _forcedUpdate = false;
   _segments = 10;
 }

void AppWindow::initPrograms ()
 {
   // Load your shaders and link your programs here:
   _flatvsh.load_and_compile ( GL_VERTEX_SHADER, "../vsh_mcol_flat.glsl" );
   _flatfsh.load_and_compile ( GL_FRAGMENT_SHADER, "../fsh_flat.glsl" );
   _prog.init_and_link ( _flatvsh, _flatfsh );

   _linevsh.load_and_compile(GL_VERTEX_SHADER, "../vsh_lineshape.glsl");
   _linefsh.load_and_compile(GL_FRAGMENT_SHADER, "../fsh_lineshape.glsl");
   _lineprog.init_and_link(_linevsh, _linefsh);

   // Init my scene objects:
   _axis.init ( _prog );
   _line.init ( _prog );
   _lineShape.init(_lineprog);
   _poly.init ( _prog, GsColor::darkblue, GsColor::darkred );

   // Build some of my scene objects:
   _axis.build(1.0f); // axis has radius 1.0
 }

// mouse events are in window coordinates, but your 2D scene is in [-1,1]x[-1,1],
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
    { case ' ': _viewaxis = !_viewaxis; redraw(); break;

      case 127: // Del pressed
        if ( _poly.selection()>=0 ) // there is a selection
         { _poly.del ( _poly.selection() );
           _updated = true;
           redraw ();
         } break;

	  case 27: // Esc was pressed
        if ( _poly.selection()>=0 ) // there is a selection
         { _poly.select ( -1, _pickprec );
           redraw ();
         } 
        else exit(1);
        break;

	  case '0':
		  _curveType = None;
		  _updated = true;
		  redraw();
		  break;

	  case '1':
		  _curveType = Lagrange;
		  _updated = true;
		  redraw();
		  break;

	  case '2':
		  _curveType = Bezier;
		  _updated = true;
		  redraw();
		  break;

	  case 'z':
		  _updated = true;
		  _forcedUpdate = true;
		  redraw();
		  break;

	  case 'x':
		  _autoUpdate3D = !_autoUpdate3D;
		  break;

	  case 'q':
		  _segments++;
		  _updated = true;
		  redraw();
		  break;
	  case 'a':
		  _segments = _segments > 3 ? _segments - 1 : _segments;
		  _updated = true;
		  redraw();
		  break;
    }
 }

void AppWindow::glutSpecial ( int key, int x, int y )
 {
   bool rd=true;
   const float incr=GS_TORAD(2.5f);
   const float inct=0.05f;
   const float incf=0.05f;
   bool ctrl = glutGetModifiers()&GLUT_ACTIVE_CTRL? true:false;
   bool alt = glutGetModifiers()&GLUT_ACTIVE_ALT? true:false;
   //std::cout<<ctrl<<gsnl;

   switch ( key )
    { case GLUT_KEY_LEFT:      if(ctrl)_roty-=incr; else if(alt)_trans.x-=inct; break;
      case GLUT_KEY_RIGHT:     if(ctrl)_roty+=incr; else if(alt)_trans.x+=inct; break;
      case GLUT_KEY_UP:        if(ctrl)_rotx+=incr; else if(alt)_trans.y+=inct; break;
      case GLUT_KEY_DOWN:      if(ctrl)_rotx-=incr; else if(alt)_trans.y-=inct; break;
      case GLUT_KEY_PAGE_UP:   if(ctrl)_fovy-=incf; else if(alt)_trans.z+=inct; break;
      case GLUT_KEY_PAGE_DOWN: if(ctrl)_fovy+=incf; else if(alt)_trans.z-=inct; break;
      default: return; // return without rendering
	}
   if (rd) redraw(); // ask the window to be rendered when possible
 }

static GsVec plane_intersect ( const GsVec& c, const GsVec& n, const GsVec& p1, const GsVec& p2 )
 {
   GsVec coords = n;
   float coordsw = -dot(n,c);
   float nl = n.norm();
   if (nl==0.0) return GsVec::null;
   coords/=nl; coordsw/=nl;
   float fact = dot ( coords, p1-p2 );
   if ( fact==0.0 ) return GsVec::null;
   float k = (coordsw+dot(coords,p1)) / fact;
   return mix ( p1, p2, k );
 }

GsVec AppWindow::rayXYintercept ( const GsVec2& v )
 {
   // Recall that a mouse click in the screen corresponds to a ray traversing the 3D scene
   // Here we intersect this ray with the XY plane:
   GsVec m ( windowToScene ( v ) );
   GsVec p1(m.x,m.y,_znear); // mouse click in the near plane
   GsMat M(_spr); M.invert();
   p1 = M * p1;          // mouse click in camera coordinates
   GsVec p2 = p1-_eye; 
   p2.normalize();
   p2 *= (_zfar+_znear); // zfar and znear are >0
   p2 += p1;             // p1,p2 ray in camera coordinates
   M=_str; M.invert();
   p1 = M * p1;
   p2 = M * p2;          // p1,p2 ray in scene coordinates
   return plane_intersect ( GsVec::null, GsVec::k, p1, p2 ); // intersect p1,p2 ray with XY plane
 }

void AppWindow::glutMouse ( int button, int state, int x, int y )
 {
   GsVec m = rayXYintercept ( GsVec2(x,y) );
   //std::cout<<m<<gsnl;

   // Check if a vertex is being selected, ie, if m is very close to a vertex:
   int i;
   const GsArray<GsVec>& V = _poly.vertexarray();
   for ( i=0; i<V.size(); i++ )
    if ( dist(V[i],m)<=_pickprec ) break;

   if ( i<V.size() ) // vertex selected
    { _poly.select ( i, _pickprec );
      _oktodrag = true;
      redraw();
    }
   else if ( state==GLUT_UP )
    { // At this point no selection was detected and we will add a new vertex:
      _poly.add ( m );
	  _updated = true;
      _oktodrag = false;
      redraw ();
    }
 }

void AppWindow::glutMotion ( int x, int y )
 {
   //std::cout<<"Motion"<<std::endl;
   if ( !_oktodrag ) { return; }
   GsVec m = rayXYintercept ( GsVec2(x,y) );
   if (_poly.selection() >= 0) { _poly.move(_poly.selection(), m); _updated = true; redraw(); }
 }

void AppWindow::glutMenu ( int m )
 {
   std::cout<<"Menu Event: "<<m<<std::endl;
 }

void AppWindow::glutReshape ( int w, int h )
 {
   // Define that OpenGL should use the whole window for rendering
   glViewport ( 0, 0, w, h );
   _w=w; _h=h;
   _aspect = float(_w) / float(_h);
 }

// here we will redraw the scene according to the current state of the application.
void AppWindow::glutDisplay ()
 {
   // Clear the rendering window
   glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   // Define our scene transformation:
   GsMat rx, ry;
   rx.rotx ( _rotx );
   ry.roty ( _roty );
   _str = rx*ry;
   _str.setrans ( _trans );

   // Define our projection transformation:
   // (see gltutors example projection.exe, we are replicating the same behavior here)
   _cam.lookat ( _eye, _center, _up );
   _proj.perspective ( _fovy, _aspect, _znear, _zfar );
   _spr = _proj * _cam; // final scene projection

   // Is the polygon up to date
   if (_updated && _poly.vertexarray().size() > 1 && _curveType != None)
   {
	   GsArray<GsVec> curve;
	   if (_curveType == Lagrange)
		   evaluate_lagrange(_segments, curve, _poly.vertexarray());
	   else
           evaluate_bezier(_segments, curve, _poly.vertexarray());

	   _line.build(curve, GsColor::orange);

	   // Build the 3d figure
	   if (_autoUpdate3D || _forcedUpdate)
	   {
		   _lineShape.build(curve, GsColor::cyan);
	   }
   }
   _updated = false;
   _forcedUpdate = false;

   // Draw:
   if ( _viewaxis ) _axis.draw ( _str, _spr );
  
   if (_curveType != None)
   {
	   _line.draw(_str, _spr);
	   _lineShape.draw(_str, _spr);
   }

	_poly.draw ( _str, _spr );

   // Swap buffers and draw:
   glFlush();         // flush the pipeline (usually not necessary)
   glutSwapBuffers(); // we were drawing to the back buffer, now bring it to the front
}

