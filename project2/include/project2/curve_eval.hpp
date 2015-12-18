#ifndef _CURVE_EVAL_
#define _CURVE_EVAL_

#include <project2/common.hpp>
#include <vector>

void evaluate_bezier(size_t resolution, std::vector<glm::vec3>& curve, const std::vector<glm::vec3>& points);
void evaluate_hermite_splines(size_t resolution, std::vector<glm::vec3>& curve, const std::vector<glm::vec3>& points);

#endif
