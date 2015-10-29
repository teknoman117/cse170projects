
# include <iostream>
# include "ogl_tools.h"
# include "app_window.h"

//==========================================================================
// Main routine
//==========================================================================
int main ( int argc, char** argv )
 {
   // Init freeglut library:
   glutInit ( &argc, argv );
#ifndef __APPLE__
   glutInitDisplayMode ( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
   glutInitContextProfile ( GLUT_CORE_PROFILE );
#else
   glutInitDisplayMode ( GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
#endif
     
   // Now create the window of your application:
   AppWindow* w = new AppWindow ( "GlutApp3D CSE 170 Support Code", 300, 300, 640, 480 );

   // Use a white canvas etc:
   glClearColor ( 1, 1, 1, 1 );
   glEnable ( GL_BLEND ); // for transparency and antialiasing smoothing
   glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glEnable ( GL_LINE_SMOOTH );
   glHint ( GL_LINE_SMOOTH_HINT, GL_NICEST );
   glEnable ( GL_POINT_SMOOTH );
   glPointSize ( 5.0 );

   // Give some instructions:
   std::cout << "\n";
   std::cout << "Mouse clicks to edit a control polygon\n";
   std::cout << "Use Ctrl+Arrows and Ctrl+PageUp/Dn to control camera\n";
   std::cout << "\n";

   // Finally start the main loop:
   w->run ();
}
