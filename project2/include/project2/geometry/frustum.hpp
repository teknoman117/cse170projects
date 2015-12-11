#ifndef __FRUSTRUM_HPP__
#define __FRUSTRUM_HPP__

#include <project2/common.hpp>
#include <project2/geometry/aabox.hpp>

class Frustum
{
	typedef enum : int 
	{
		LEFT = 0,
		RIGHT,
		TOP,
		BOTTOM,
		NEAR,
		FAR
	} PlaneId;

	struct Plane
	{
		glm::vec3 point;
		glm::vec3 normal;

		float distance(glm::vec3 p) const
		{
			return dot(normal, p - point);
		}
	};

	glm::vec3 localForward;
	glm::vec3 localUp;
	glm::vec3 relativeRight;

	glm::vec3 nearHalfDimension;
	glm::vec3 farHalfDimension;

	Plane planes[6];

public:
	typedef enum : int 
	{
		OUTSIDE, 
		INTERSECT, 
		INSIDE
	} CullResult;

	Frustum();
	~Frustum();

	void SetCameraProperties(float yFov, float aspect, float nearD, float farD);
	void SetCameraOrientation(const glm::vec3& position, const glm::vec3& target, const glm::vec3& globalUp);

	CullResult Compare(const AABox &box) const;
};


#endif