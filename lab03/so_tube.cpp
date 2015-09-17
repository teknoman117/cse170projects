//
//  so_capsule.cpp
//  glutapp3d
//
//  Created by Nathaniel Lewis on 9/2/15.
//  Copyright (c) 2015 ucmerced. All rights reserved.
//

#include "so_tube.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159
#endif

namespace
{
    // Push a triangle
    void pushTriangle(std::vector<GsVec>& P, std::vector<GsColor>& C, std::initializer_list<GsVec> T)
    {
        P.push_back(*(T.begin() + 0)); C.push_back(GsColor::white);
        P.push_back(*(T.begin() + 1)); C.push_back(GsColor::white);
        
        P.push_back(*(T.begin() + 1)); C.push_back(GsColor::white);
        P.push_back(*(T.begin() + 2)); C.push_back(GsColor::white);
        
        P.push_back(*(T.begin() + 2)); C.push_back(GsColor::white);
        P.push_back(*(T.begin() + 0)); C.push_back(GsColor::white);
    }
}

SoTube::SoTube()
{
    _numpoints = 0;
}

void SoTube::init ( const GlProgram& prog )
{
    // Define buffers needed:
    set_program ( prog );
    gen_vertex_arrays ( 1 ); // will use 1 vertex array
    gen_buffers ( 2 );       // will use 2 buffers: one for coordinates and one for colors
    uniform_locations ( 2 ); // will send 2 variables: the 2 matrices below
    uniform_location ( 0, "vTransf" );
    uniform_location ( 1, "vProj" );
}

void SoTube::build ( float len, float r, int nfaces )
{
    P.clear(); C.clear(); // set size to zero, just in case
   
    float hlen = len/2.0f;
    float faceAngularLength = (2.0 * static_cast<float>(M_PI)) / static_cast<float>(nfaces);
    int   vertices = (nfaces * 6);

    // Reserve space for the capsule vertices
    P.reserve(vertices);
    C.reserve(vertices);

    // Generate the coordinates for the capsule body
    std::vector<GsVec> V;
    V.reserve(nfaces * 2);

    // Compute the tube section of the capsule
    for(int i = 0; i < nfaces; i++)
    {
        float p = static_cast<float>(i) * faceAngularLength;
        
        V.push_back(GsVec(r * std::cos(p), r * std::sin(p) + 0.5f,  hlen));
        V.push_back(GsVec(r * std::cos(p), r * std::sin(p) + 0.5f, -hlen));
    }
    
    // Stitch everything back together
    for(int i = 0; i < nfaces; i++)
    {
        std::vector<GsVec>::iterator v1 = V.begin() + (2 * i);
        std::vector<GsVec>::iterator v2 = V.begin() + (2 * (i + 1));
        
        if(v2 == V.end())
            v2 = V.begin();
        
        pushTriangle(P, C, { *(v1), *(v1 + 1), *(v2) });
        pushTriangle(P, C, { *(v2 + 1), *(v2), *(v1 + 1) });
    }

    // send data to OpenGL buffers:
    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glBufferData ( GL_ARRAY_BUFFER, P.size()*3*sizeof(float), &P[0], GL_STATIC_DRAW );
    glBindBuffer ( GL_ARRAY_BUFFER, buf[1] );
    glBufferData ( GL_ARRAY_BUFFER, C.size()*4*sizeof(gsbyte), &C[0], GL_STATIC_DRAW );
    
    // save size so that we can free our buffers and later just draw the OpenGL arrays:
    _numpoints = P.size();
    
    // free non-needed memory:
    P.resize(0); C.resize(0);
    changed = 0;
}

void SoTube::draw ( GsMat& tr, GsMat& pr )
{
    // Draw Lines:
    glUseProgram ( prog );
    glBindVertexArray ( va[0] );
    
    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] ); // positions
    glEnableVertexAttribArray ( 0 );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glBindBuffer ( GL_ARRAY_BUFFER, buf[1] ); // colors
    glEnableVertexAttribArray ( 1 );
    glVertexAttribPointer ( 1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );
    
    glUniformMatrix4fv ( uniloc[0], 1, GL_FALSE, tr.e );
    glUniformMatrix4fv ( uniloc[1], 1, GL_FALSE, pr.e );
    
    glDrawArrays ( GL_LINES, 0, _numpoints );
}
