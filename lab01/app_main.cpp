
# include <iostream>
# include "ogl_tools.h"
# include "app_window.h"

//==========================================================================
// Main routine
//==========================================================================
int main ( int argc, char** argv )
 {
   // Init freeglut library
   glutInit ( &argc, argv );

#ifdef __APPLE__
   glutInitDisplayMode ( GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
#else
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitContextVersion (3, 2);
   glutInitContextProfile ( GLUT_CORE_PROFILE );
#endif

   // Now create the window of your application:
   AppWindow* w = new AppWindow ( "CSE-170 Support Code", 300, 300, 600, 600 );

   // Finally start the main loop:
   w->run ();
}
