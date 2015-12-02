#include "so_line.h"

SoLine::SoLine() : _numpoints(0)
{
}

void SoLine::init(const GlProgram& prog)
{
	set_program(prog);

	gen_vertex_arrays(1);
	gen_buffers(2);

	uniform_locations(2);
	uniform_location(0, "vTransf");
	uniform_location(1, "vProj");
}

void SoLine::build(const GsArray<GsVec>& points, const GsColor color)
{
	// Generate a color buffer for the quantity of points we have
	GsArray<GsColor> C(points.size(), points.size());
	C.setall(color);

	// Upload the data to the gpu
	glBindVertexArray(va[0]);

	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GsVec) * points.size(), (const GsVec *) points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GsVec), 0);

	glBindBuffer(GL_ARRAY_BUFFER, buf[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GsColor) * C.size(), (const GsColor *) C, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GsColor), 0);

	_numpoints = points.size();
}

void SoLine::draw(GsMat& tr, GsMat& pr)
{
	if (!_numpoints)
		return;

	glUseProgram(prog);
	glBindVertexArray(va[0]);
	glUniformMatrix4fv(uniloc[0], 1, GL_FALSE, tr.e);
	glUniformMatrix4fv(uniloc[1], 1, GL_FALSE, pr.e);
	glDrawArrays(GL_LINE_STRIP, 0, _numpoints);
}
