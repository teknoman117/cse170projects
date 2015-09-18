#include "so_cylinder.h"
#include <cmath>

void SoCylinder::init ( const GlProgram& prog )
{
    // Define buffers needed:
    set_program ( prog );
    gen_vertex_arrays ( 1 ); // will use 1 vertex array
    gen_buffers ( 3 );       // will use 2 buffers: one for coordinates, one for normals, one for indices
    
    uniform_locations ( 2 ); // will send 2 variables: the 2 matrices below
    uniform_location ( 0, "vTransf" );
    uniform_location ( 1, "vProj" );
}

void SoCylinder::build ( float length, float radius, unsigned short nfaces )
{
    // Allocate local storage for the cylinder data
    std::vector<GsVec> V;
    std::vector<GsVec> N;
    std::vector<unsigned short> E;
    
    // Compute constants for cylinder
    const float angularLength = (2.0f * M_PI) / static_cast<float>(nfaces);
    const float hlen = length / 2.0f;
    
    // Compute the total number of
    size_t vertices = (4 * nfaces) + 2;
    V.resize(vertices);
    N.resize(vertices);
    
    // Compute the vertices
    for(int i = 0; i < nfaces; i++)
    {
        // Compute the vertex
        float theta = static_cast<float>(i) * angularLength;
        
        // Compute the verices
        V[i + (0*nfaces)] = GsVec(std::cos(theta) * radius, hlen, std::sin(theta) * radius);
        V[i + (1*nfaces)] = GsVec(std::cos(theta) * radius, -hlen, std::sin(theta) * radius);
        V[i + (2*nfaces)] = V[i + (0*nfaces)];
        V[i + (3*nfaces)] = V[i + (1*nfaces)];
        
        // Compute the normals
        N[i + (0*nfaces)] = V[i + (0*nfaces)];
        N[i + (0*nfaces)].y = 0;
        N[i + (0*nfaces)].normalize();
        
        N[i + (1*nfaces)] = V[i + (1*nfaces)];
        N[i + (1*nfaces)].y = 0;
        N[i + (1*nfaces)].normalize();
        
        N[i + (2*nfaces)] = GsVec(0,1,0);
        N[i + (3*nfaces)] = GsVec(0,-1,0);
    }
    
    // Vertices and normals of the caps
    V[(4*nfaces) + 0] = GsVec(0, hlen, 0);
    V[(4*nfaces) + 1] = GsVec(0, -hlen, 0);
    N[(4*nfaces) + 0] = GsVec(0,1,0);
    N[(4*nfaces) + 1] = GsVec(0,-1,0);
    
    // Compute the indices into the vertices for the tube
    for(size_t i = 0; i < nfaces; i++)
    {
        E.push_back(i + (0*nfaces));
        E.push_back(i + (1*nfaces));
    }
    E.push_back(0*nfaces);
    E.push_back(1*nfaces);

    // Compute the indices for the top
    for(size_t i = 0; i < nfaces; i++)
    {
        E.push_back(i + (2*nfaces));
        E.push_back(0 + (4*nfaces));
    }
    E.push_back(2*nfaces);
    E.push_back(4*nfaces);
    
    // Compute the indices for the bottom
    for(size_t i = 0; i < nfaces; i++)
    {
        E.push_back(1 + (4*nfaces));
        E.push_back(i + (3*nfaces));
    }
    E.push_back((4*nfaces)+1);
    E.push_back(3*nfaces);
    
    // send data to OpenGL buffers:
    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glBufferData ( GL_ARRAY_BUFFER, V.size()*3*sizeof(float), &V[0], GL_STATIC_DRAW );
    glBindBuffer ( GL_ARRAY_BUFFER, buf[1] );
    glBufferData ( GL_ARRAY_BUFFER, N.size()*3*sizeof(float), &N[0], GL_STATIC_DRAW );
    glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, buf[2] );
    glBufferData ( GL_ELEMENT_ARRAY_BUFFER, E.size()*sizeof(short), &E[0], GL_STATIC_DRAW );
    
    // Final element count
    elementCount = E.size();
    changed = 0;
}

void SoCylinder::draw ( GsMat& tr, GsMat& pr )
{
    // Draw Lines:
    glUseProgram ( prog );
    glBindVertexArray ( va[0] );
    
    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] ); // positions
    glEnableVertexAttribArray ( 0 );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glBindBuffer ( GL_ARRAY_BUFFER, buf[1] ); // colors
    glEnableVertexAttribArray ( 1 );
    glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, buf[2] );
    
    glUniformMatrix4fv ( uniloc[0], 1, GL_FALSE, tr.e );
    glUniformMatrix4fv ( uniloc[1], 1, GL_FALSE, pr.e );
    
    glDrawElements( GL_TRIANGLE_STRIP, (GLsizei) elementCount, GL_UNSIGNED_SHORT, 0);
}

