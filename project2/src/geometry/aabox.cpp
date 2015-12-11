/* ------------------------------------------------------

 Axis Aligned Boxes - Lighthouse3D
 Modified by Nathaniel R. Lewis 2015

  -----------------------------------------------------*/

#include <project2/geometry/aabox.hpp>

using namespace glm;

AABox::AABox(const glm::vec3& a, const glm::vec3& b) 
	: a(min(a,b)), b(max(a,b))
{
}

AABox::AABox(void)
	: a(vec3(-0.5,-0.5,-0.5)), b(vec3(0.5,0.5,0.5))
{	
}

AABox::~AABox() 
{
}

void AABox::setBox(const glm::vec3& a, const glm::vec3& b) 
{
	this->a = min(a, b);
	this->b = max(a, b);
}

vec3 AABox::getVertexP(const vec3 &normal) const
{
	vec3 P = a;

	if (normal.x >= 0)
		P.x = b.x;

	if (normal.y >= 0)
		P.y = b.y;

	if (normal.z >= 0)
		P.z = b.z;

	return P;
}

vec3 AABox::getVertexN(const vec3 &normal) const 
{
	vec3 N = b;

	if (normal.x >= 0)
		N.x = a.x;

	if (normal.y >= 0)
		N.y = a.y;

	if (normal.z >= 0)
		N.z = a.z;

	return N;
}

	

