/* ------------------------------------------------------

 Axis Aligned Boxes - Lighthouse3D
 Modified by Nathaniel R. Lewis 2015

  -----------------------------------------------------*/


#ifndef _AABOX_
#define _AABOX_

#include <project2/common.hpp>

class AABox 
{
	glm::vec3 a;
	glm::vec3 b;

public:
	AABox(const glm::vec3& a, const glm::vec3& b);
	AABox();
	~AABox();

	void setBox(const glm::vec3& a, const glm::vec3& b);

	// for use in frustum computations
	glm::vec3 getVertexP(const glm::vec3 &normal) const;
	glm::vec3 getVertexN(const glm::vec3 &normal) const;
};


#endif