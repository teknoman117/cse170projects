#include <project2/terrain.hpp>
#include <cmath>
#include <fstream>
#include <cinttypes>

#ifndef M_PI
#define M_PI 3.14159f
#endif

#ifdef WIN32
#define ssize_t int32_t
#endif

using namespace glm;

GLTerrain::GLTerrain ( std::string filename, size_t width, size_t height, vec3 convertToMeters)
    : GLObject(3), width(width), height(height), convertToMeters(convertToMeters)
{
    std::vector<vec3> N;
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
            V[idx] = vec3( float(i) * convertToMeters.x, elevation[idx] * convertToMeters.y, float(j) * convertToMeters.z);
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
            
            vec3 nx = normalize(V[idx21] - V[idx01]);
            vec3 nz = normalize(V[idx12] - V[idx10]);
            
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
    glBindVertexArray(vertexArrayObject);
    
    glEnableVertexAttribArray(0);
    glBindBuffer ( GL_ARRAY_BUFFER, buffers[0] );
    glBufferData ( GL_ARRAY_BUFFER, V.size()*3*sizeof(float), &V[0], GL_STATIC_DRAW );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glEnableVertexAttribArray(1);
    glBindBuffer ( GL_ARRAY_BUFFER, buffers[1] );
    glBufferData ( GL_ARRAY_BUFFER, N.size()*3*sizeof(float), &N[0], GL_STATIC_DRAW );
    glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, buffers[2] );
    glBufferData ( GL_ELEMENT_ARRAY_BUFFER, E.size()*sizeof(GLuint), &E[0], GL_STATIC_DRAW );
    
    // Final element count
    numPoints = E.size();
    
    // Deallocate
    N.resize(0);
	E.resize(0);

    std::cout << "[INFO] [TERRAIN \"" << filename << "\"] Constructed terrain with " << numPoints << " indices" << std::endl;
}

void GLTerrain::Draw()
{
    glBindVertexArray(vertexArrayObject);
    glDrawElements(GL_TRIANGLE_STRIP, (GLsizei) numPoints, GL_UNSIGNED_INT, 0);
}

// Sample the heigth at a point
float GLTerrain::sampleHeightAtPoint(vec2 p)
{
	if (V.size() == 0)
		return 0;

    vec2 point = vec2(p.x / convertToMeters.x, p.y / convertToMeters.z);
    
    size_t x1 = floor(point.x);
    size_t x2 = ceil(point.x);
    size_t y1 = floor(point.y);
    size_t y2 = ceil(point.y);
    
    vec3 v00 = V[y1*width + x1];
    vec3 v01 = V[y2*width + x1];
    vec3 v10 = V[y1*width + x2];
    vec3 v11 = V[y2*width + x2];
    
    vec3 a = mix(v00, v10, point.x - floor(point.x));
    vec3 b = mix(v01, v11, point.x - floor(point.x));
    vec3 c = mix(a, b, point.y - floor(point.y));
    
    return c.y;
}

