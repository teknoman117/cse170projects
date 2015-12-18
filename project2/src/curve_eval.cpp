#include <project2/curve_eval.hpp>
#include <cmath>

using namespace glm;

namespace
{
	float factorialf(float n)
	{
		if (n < 2.0f)
			return 1.0f;

		return n * factorialf(n - 1.0f);
	}

	float chooseFunctionf(float i, float n)
	{
		return factorialf(n) / (factorialf(i) * factorialf(n - i));
	}
}

void evaluate_bezier(size_t resolution, std::vector<glm::vec3>& curve, const std::vector<glm::vec3>& points)
{
	size_t size = points.size();
	curve.resize(resolution);

	// Do the thing
	for (size_t ti = 0; ti < resolution; ti++)
	{
		float t = float(ti) / float(resolution-1);
		vec3 p(0, 0, 0);

		for (size_t i = 0; i < size; i++)
		{
			float B = float(chooseFunctionf(i, size - 1)) * powf(t, i) * powf(1 - t, size - i - 1);
			p += points[i] * B;
		}

		curve[ti] = p;
		//std::cout << ti << std::endl;
	}
}
