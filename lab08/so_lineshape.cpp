#include "so_lineshape.h"

SoLineShape::SoLineShape() : _numpoints(0)
{
}

void SoLineShape::init(const GlProgram& prog)
{
	set_program(prog);

	gen_vertex_arrays(1);
	gen_buffers(3);

	uniform_locations(2);
	uniform_location(0, "vTransf");
	uniform_location(1, "vProj");
}

void SoLineShape::build(const GsArray<GsVec>& points, const GsColor color)
{
	int cnt = points.size() * 2;

	// Generate a color buffer for the quantity of points we have
	GsArray<GsColor> C(cnt, cnt);
	GsArray<GsVec> P(cnt, cnt), N(cnt, cnt);
	C.setall(color);

	// Populate the vertices
	for (int i = 0; i < points.size(); i++)
	{
		// Lookup point
		GsVec p = points[i];

		// Compute the normal of this point (compute the tangent)
		float slope = 0.0;
		int num = 0;

		if (i > 0)
		{
			slope += (p.y - points[i - 1].y) / (p.x - points[i - 1].x);
			num++;
		}
		if (i < points.size()-1)
		{
			slope += (p.y - points[i + 1].y) / (p.x - points[i + 1].x);
			num++;
		}

		slope = slope / float(num);

		GsVec tangent(1.f, slope, 0.f), bitangent(0.f, 0.f, -1.f);
		tangent.normalize();
		GsVec normal = cross(tangent, bitangent);

		P[(i * 2) + 0] = p;
		P[(i * 2) + 1] = p + GsVec(0, 0, -0.2f);
		N[(i * 2) + 0] = normal;
		N[(i * 2) + 1] = normal;
	}

	// Upload the data to the gpu
	glBindVertexArray(va[0]);

	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GsVec) * P.size(), (const GsVec *) P, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GsVec), 0);

	glBindBuffer(GL_ARRAY_BUFFER, buf[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GsColor) * C.size(), (const GsColor *) C, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GsColor), 0);

	glBindBuffer(GL_ARRAY_BUFFER, buf[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GsVec) * N.size(), (const GsVec *)N, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GsVec), 0);

	_numpoints = P.size();
}

void SoLineShape::draw(GsMat& tr, GsMat& pr)
{
	if (!_numpoints)
		return;

	glUseProgram(prog);
	glBindVertexArray(va[0]);
	glUniformMatrix4fv(uniloc[0], 1, GL_FALSE, tr.e);
	glUniformMatrix4fv(uniloc[1], 1, GL_FALSE, pr.e);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, _numpoints);
}
