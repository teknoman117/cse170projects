#ifndef _CURVE_EVAL_
#define _CURVE_EVAL_

#include "gsim/gs.h"
#include "gsim/gs_array.h"
#include "gsim/gs_color.h"
#include "gsim/gs_vec.h"

void evaluate_lagrange(int resolution, GsArray<GsVec>& curve, const GsArray<GsVec>& points);
void evaluate_bezier(int resolution, GsArray<GsVec>& curve, const GsArray<GsVec>& points);

#endif
