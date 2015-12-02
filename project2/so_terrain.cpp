#include "so_terrain.h"
#include <cmath>
#include <fstream>
#include <cinttypes>

#ifndef M_PI
#define M_PI 3.14159f
#endif

#ifdef WIN32
#define ssize_t int32_t
#endif

void SoTerrain::build ( std::string filename, size_t width, size_t height, GsVec convertToMeters)
{
    this->width = width;
    this->height = height;
    this->convertToMeters = convertToMeters;
    
    std::vector<GsVec> N;
    std::vector<GLuint> E;
    
    // Read the data file
    std::vector<float> elevation;
    {
        std::ifstream inputFile(filename, std::ifstream::binary | std::ifstream::in);
        size_t size;
        
        if(!inputFile.is_open()) return;
        
        /*get filesize:*/
        inputFile.seekg(0, inputFile.end);
        size = inputFile.tellg();
        inputFile.seekg(0, inputFile.beg);
        
        /* read contents of the file into the vector */
        elevation.resize(size/4);
        inputFile.read((char *) elevation.data(), size);
        inputFile.close();
    }
    
    // Generate the vertex buffer
    V.resize(width*height);
    for(size_t j = 0; j < height; j++)
    {
        for(size_t i = 0; i < width; i++)
        {
            size_t idx = j*width + i;
            V[idx] = GsVec( float(i) * convertToMeters.x, elevation[idx] * convertToMeters.y, float(j) * convertToMeters.z);
        }
    }
    
    // Generate the normal buffer
    N.resize(width*height);
    for(size_t j = 1; j < height-1; j++)
    {
        for(size_t i = 1; i < width-1; i++)
        {
            
            size_t idx01 = (j-1)*width+i;
            size_t idx21 = (j+1)*width+i;
            size_t idx10 = j*width+(i-1);
            size_t idx12 = j*width+(i+1);
            
            GsVec nx = V[idx21] - V[idx01];
            nx.normalize();
            
            GsVec nz = V[idx12] - V[idx10];
            nz.normalize();
            
            N[j*width+i] = cross(nx, nz);
        }
    }
    
    // Generate the index buffer
    for(size_t j = 0; j < height - 1; j++)
    {
        if(j%2 == 0)
        {
            for (ssize_t i = 0; i < width; i++)
            {
                size_t idxA = (j+0)*width+i;
                size_t idxB = (j+1)*width+i;
                
                E.push_back((GLuint) idxA);
                E.push_back((GLuint) idxB);
            }
        }
        
        else
        {
            for (ssize_t i = width-1; i >= 0; i--)
            {
                size_t idxA = (j+0)*width+i;
                size_t idxB = (j+1)*width+i;
                
                E.push_back((GLuint) idxA);
                E.push_back((GLuint) idxB);
            }
        }
        
        // insert degenerate triangle if this isn't the last row
        if(j != height - 2)
        {
            E.push_back(*(E.end() - 1));
        }
    }
    
    
    // send data to OpenGL buffers:
    glBindVertexArray(va[0]);
    
    glEnableVertexAttribArray(0);
    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glBufferData ( GL_ARRAY_BUFFER, V.size()*3*sizeof(float), &V[0], GL_STATIC_DRAW );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glEnableVertexAttribArray(1);
    glBindBuffer ( GL_ARRAY_BUFFER, buf[1] );
    glBufferData ( GL_ARRAY_BUFFER, N.size()*3*sizeof(float), &N[0], GL_STATIC_DRAW );
    glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, buf[2] );
    glBufferData ( GL_ELEMENT_ARRAY_BUFFER, E.size()*sizeof(GLuint), &E[0], GL_STATIC_DRAW );
    
    // Final element count
    _numpoints = E.size();
    
    // Deallocate
    N.resize(0);
	E.resize(0);
}

void SoTerrain::init ( const GlProgram& prog )
{
    set_program ( prog );
    gen_vertex_arrays ( 1 ); // will use 1 vertex array
    gen_buffers(3);
    
    uniform_locations ( 10 ); // will send 2 variables: the 2 matrices below
    uniform_location ( 0, "vTransf" );
    uniform_location ( 1, "vProj" );
    

    /*uniform_location(2, "lPos");
    uniform_location(3, "la");
    uniform_location(4, "ld");
    uniform_location(5, "ls");
    uniform_location(6, "ka");
    uniform_location(7, "kd");
    uniform_location(8, "ks");
    uniform_location(9, "sh");*/
}

void SoTerrain::draw ( const GsMat& tr, const GsMat& pr )
{
    // Draw Lines:
    glUseProgram ( prog );
    glBindVertexArray ( va[0] );
    glUniformMatrix4fv ( uniloc[0], 1, GL_FALSE, tr.e );
    glUniformMatrix4fv ( uniloc[1], 1, GL_FALSE, pr.e );
    
    /*float f[4]; // we convert below our color values to be in [0,1]
    glUniform3fv ( uniloc[2], 1, l.pos.e );
    glUniform4fv ( uniloc[3], 1, l.amb.get(f) );
    glUniform4fv ( uniloc[4], 1, l.dif.get(f) );
    glUniform4fv ( uniloc[5], 1, l.spe.get(f) );
    glUniform4fv ( uniloc[6], 1, m.amb.get(f) );
    glUniform4fv ( uniloc[7], 1, m.dif.get(f) );
    glUniform4fv ( uniloc[8], 1, m.spe.get(f) );
    glUniform1fv ( uniloc[9], 1, &m.sh );*/
    
    //glDrawArrays(GL_TRIANGLES, 0, (GLsizei) sphere->_numpoints);
    glDrawElements(GL_TRIANGLE_STRIP, (GLsizei) _numpoints, GL_UNSIGNED_INT, 0);
}

// Sample the heigth at a point
float SoTerrain::sampleHeightAtPoint(GsVec2 p)
{
	if (V.size() == 0)
		return 0;

    GsVec2 point = GsVec2(p.x / convertToMeters.x, p.y / convertToMeters.z);
    
    size_t x1 = floor(point.x);
    size_t x2 = ceil(point.x);
    size_t y1 = floor(point.y);
    size_t y2 = ceil(point.y);
    
    GsVec v00 = V[y1*width + x1];
    GsVec v01 = V[y2*width + x1];
    GsVec v10 = V[y1*width + x2];
    GsVec v11 = V[y2*width + x2];
    
    GsVec a = mix(v00, v10, point.x - floor(point.x));
    GsVec b = mix(v01, v11, point.x - floor(point.x));
    GsVec c = mix(a, b, point.y - floor(point.y));
    
    return c.y;
}

