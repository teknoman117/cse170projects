#include "curve_eval.h"

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

void evaluate_lagrange(int resolution, GsArray<GsVec>& curve, const GsArray<GsVec>& points)
{
	int size = points.size();
	curve.size(resolution);

	// Find the minimum and maximum x values
	float min = 10000.0;
	float max = -10000.0;
	for (int i = 0; i < size; i++)
	{
		GsVec v = points[i];
		if (v.x < min) min = v.x;
		if (v.x > max) max = v.x;
	}

	// Parameterize the curve
	float incr = (max - min) / float(resolution-1);
	for (int t = 0; t < resolution; t++)
	{
		float x = min + (float(t) * incr);
		float y = 0.0f;

		for (int i = 0; i < size; i++)
		{
			GsVec pi = points[i];
			float b = 1.0f;
			for (int j = 0; j < size; j++)
			{
				if (j == i)
					continue;

				GsVec pj = points[j];

				b *= (x - pj.x) / (pi.x - pj.x);
			}
			y += pi.y * b;
		}
		curve[t] = GsVec(x, y, 0.f);
	}
}

void evaluate_bezier(int resolution, GsArray<GsVec>& curve, const GsArray<GsVec>& points)
{
	int size = points.size();
	curve.size(resolution);

	// Do the thing
	for (int ti = 0; ti < resolution; ti++)
	{
		float t = float(ti) / float(resolution-1);
		GsVec p(0, 0, 0);

		for (int i = 0; i < size; i++)
		{
			float B = float(chooseFunctionf(i, size - 1)) * powf(t, i) * powf(1 - t, size - i - 1);
			p += points[i] * B;
		}

		curve[ti] = p;
	}
}
