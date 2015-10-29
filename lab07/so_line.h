
// Ensure the header file is included only once in multi-file projects
#ifndef SO_LINE_H
#define SO_LINE_H

// Include needed header files
# include <gsim/gs_color.h>
# include <gsim/gs_array.h>
# include <gsim/gs_vec.h>
# include "ogl_tools.h"

// Scene objects should be implemented in their own classes; and
// here is an example of how to organize a scene object in a class.
// Scene object axis:
class SoLine : public GlObjects
{
	int _numpoints;     // just saves the number of points

public:
	SoLine();
	void init(const GlProgram& prog);
	void build(const GsArray<GsVec>& points, const GsColor color);
	void draw(GsMat& tr, GsMat& pr);
};

#endif // SO_AXIS_H
