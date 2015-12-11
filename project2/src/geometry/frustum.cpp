#include <project2/geometry/frustum.hpp>
#include <cmath>

using namespace glm;

Frustum::Frustum() 
{
}

Frustum::~Frustum()
{
}

// Compute the dimensions of the near and far planes (distane isn't half, width/height are)
void Frustum::SetCameraProperties(float yFov, float aspect, float nearD, float farD)
{
	float h = tan(yFov * 0.5);

	nearHalfDimension.z = nearD;

	farHalfDimension.x = farD * h * aspect;
	farHalfDimension.y = farD * h;
	farHalfDimension.z = farD;
}

void Frustum::SetCameraOrientation(const glm::vec3& position, const glm::vec3& target, const glm::vec3& globalUp)
{
	// Compute important vectors
	localForward = normalize(target - position);
	relativeRight = cross(localForward, globalUp);
	localUp = cross(relativeRight, localForward);

	vec3 nearCenter = position + nearHalfDimension.z*localForward;
	vec3 farCenter = position + farHalfDimension.z*localForward;

	vec3 farLeftTop     = farCenter - (relativeRight * farHalfDimension.x) + (localUp * farHalfDimension.y);
	vec3 farLeftBottom  = farCenter - (relativeRight * farHalfDimension.x) - (localUp * farHalfDimension.y);
	vec3 farRightTop    = farCenter + (relativeRight * farHalfDimension.x) + (localUp * farHalfDimension.y);
	vec3 farRightBottom = farCenter + (relativeRight * farHalfDimension.x) - (localUp * farHalfDimension.y);

	planes[NEAR] = {
		.point = nearCenter,
		.normal = localForward,
	};

	planes[FAR] = {
		.point = farCenter,
		.normal = -localForward,
	};

	// Near plane's dimensions are so negligable that we assume all points are at the center of it
	planes[TOP] = {
		.point = nearCenter,
		.normal = normalize(cross(farLeftTop - nearCenter, farRightTop - nearCenter)),
	};

	planes[BOTTOM] = {
		.point = nearCenter,
		.normal = normalize(cross(farRightBottom - nearCenter, farLeftBottom - nearCenter)),
	};

	planes[LEFT] = {
		.point = nearCenter,
		.normal = normalize(cross(farLeftBottom - nearCenter, farLeftTop - nearCenter)),
	};

	planes[RIGHT] = {
		.point = nearCenter,
		.normal = normalize(cross(farRightTop - nearCenter, farRightBottom - nearCenter)),
	};
}

Frustum::CullResult Frustum::Compare(const AABox &box) const
{
	Frustum::CullResult result = INSIDE;

	for(int i = 0; i < 6; i++)
	{
		if(planes[i].distance(box.getVertexP(planes[i].normal)) < 0)
		{
			return OUTSIDE;
		}
		if(planes[i].distance(box.getVertexN(planes[i].normal)) < 0)
		{
			result = INTERSECT;
		}
	}

	return result;
}