
# include <iostream>
//# include <gsim/gs.h>
# include "ogl_tools.h"
# include "app_window.h"

//==========================================================================
// Main routine
//==========================================================================
int main ( int argc, char** argv )
 {
   std::cout<<"CSE-170 Superquadric Demo/Support Code\n\n";
   std::cout<<"Three different objects can be displayed:\n";
   std::cout<<"Normals (w/SoLines), superquadric (gouraud or phong programs), axis.\n";
   std::cout<<"Enjoy!  (and send any improvements to me)\n\n";

   // Init freeglut library:
   glutInit ( &argc, argv );

   // OS specific GLUT initialization
#if defined __APPLE__
     glutInitDisplayMode ( GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
#else
     glutInitDisplayMode ( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
     glutInitContextProfile ( GLUT_CORE_PROFILE );
#endif

   // Now create the window of your application:
   AppWindow* w = new AppWindow ( "Superquadric - UCM CSE-170", 300, 300, 640, 480 );

   // Display some info:
   w->printInfo ();

   // Finally start the main loop:
   w->run ();
}



