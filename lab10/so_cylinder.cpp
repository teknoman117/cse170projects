#include "so_cylinder.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159f
#endif

void SoCylinder::init ( const GlProgram& prog, const GlProgram& nprog, bool smooth )
{
    // Define buffers needed:
    set_program ( prog );
    gen_vertex_arrays ( 1 ); // will use 1 vertex array
    gen_buffers ( 3 );       // will use 2 buffers: one for coordinates, one for normals, one for indices
    
    if(smooth)
    {
        uniform_locations ( 10 ); // will send 2 variables: the 2 matrices below
        uniform_location ( 0, "vTransf" );
        uniform_location ( 1, "vProj" );
        
        uniform_location(2, "lPos");
        uniform_location(3, "la");
        uniform_location(4, "ld");
        uniform_location(5, "ls");
        uniform_location(6, "ka");
        uniform_location(7, "kd");
        uniform_location(8, "ks");
        uniform_location(9, "sh");
    }
    
    else
    {
        uniform_locations ( 2 ); // will send 2 variables: the 2 matrices below
        uniform_location ( 0, "vTransf" );
        uniform_location ( 1, "vProj" );
    }
    
    normals.init(nprog);
    this->smooth = smooth;
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
    
    // derp
    normals.P.clear();
    normals.C.clear();
    
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
    for(unsigned short i = 0; i < nfaces; i++)
    {
        E.push_back(i + (0*nfaces));
        E.push_back(i + (1*nfaces));
    }
    E.push_back(0*nfaces);
    E.push_back(1*nfaces);

    // Compute the indices for the top
    for(unsigned short i = 0; i < nfaces; i++)
    {
        E.push_back(i + (2*nfaces));
        E.push_back(0 + (4*nfaces));
    }
    E.push_back(2*nfaces);
    E.push_back(4*nfaces);
    
    // Compute the indices for the bottom
    for(unsigned short i = 0; i < nfaces; i++)
    {
        E.push_back(1 + (4*nfaces));
        E.push_back(i + (3*nfaces));
    }
    E.push_back((4*nfaces)+1);
    E.push_back(3*nfaces);
    
    // Build the normals for smooth surface
    if(smooth)
    {
        for(std::vector<GsVec>::iterator v = V.begin(), n = N.begin(); v != V.end(); v++, n++)
        {
            normals.P.push_back(*v);
            normals.C.push_back(GsColor::green);
            
            normals.P.push_back((*v) + (*n * 0.075f));
            normals.C.push_back(GsColor::green);
        }
    }
    
    // build the normals for the hard surface
    else
    {
        for(size_t i = 2; i < E.size(); i += 2)
        {
            // -2, -1, 0
            // 1, 0, -1
            
            unsigned short ai = E[i-2];
            unsigned short bi = E[i-1];
            unsigned short ci = E[i];
            
            GsVec a = V[ai];
            GsVec b = V[bi];
            GsVec c = V[ci];
            
            GsVec origin = (a + b + c) / 3;
            
            GsVec an = N[ai];
            GsVec bn = N[bi];
            GsVec cn = N[ci];
            
            GsVec normal = (an + bn + cn) / 3;
            normal.normalize();
            
            if(ai != bi && ai != ci && bi != ci  && !( !(i % (nfaces+1)) && i != 0  ))
            {
                normals.P.push_back(origin); normals.C.push_back(GsColor::yellow);
                normals.P.push_back(origin + (normal * 0.075f)); normals.C.push_back(GsColor::yellow);
            }
            
            ai = E[i+1];
            bi = E[i];
            ci = E[i-1];
            
            a = V[ai];
            b = V[bi];
            c = V[ci];
            
            origin = (a + b + c) / 3;
            
            an = N[ai];
            bn = N[bi];
            cn = N[ci];
            
            normal = (an + bn + cn) / 3;
            normal.normalize();
            
            if(ai != bi && ai != ci && bi != ci && !( !(i % (nfaces+1)) && i != 0  ))
            {
                normals.P.push_back(origin); normals.C.push_back(GsColor::yellow);
                normals.P.push_back(origin + (normal * 0.075)); normals.C.push_back(GsColor::yellow);
            }
        }

    }
    
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
    
    // Build the normals
    normals.build();
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

void SoCylinder::draw ( GsMat& tr, GsMat& pr, const Light& l, const Material& m )
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
    
    float f[4]; // we convert below our color values to be in [0,1]
    glUniform3fv ( uniloc[2], 1, l.pos.e );
    glUniform4fv ( uniloc[3], 1, l.amb.get(f) );
    glUniform4fv ( uniloc[4], 1, l.dif.get(f) );
    glUniform4fv ( uniloc[5], 1, l.spe.get(f) );
    glUniform4fv ( uniloc[6], 1, m.amb.get(f) );
    glUniform4fv ( uniloc[7], 1, m.dif.get(f) );
    glUniform4fv ( uniloc[8], 1, m.spe.get(f) );
    glUniform1fv ( uniloc[9], 1, &m.sh );
    
    glDrawElements( GL_TRIANGLE_STRIP, (GLsizei) elementCount, GL_UNSIGNED_SHORT, 0);
}

void SoCylinder::drawNormals(GsMat& tr, GsMat& pr)
{
    normals.draw(tr, pr);
}

