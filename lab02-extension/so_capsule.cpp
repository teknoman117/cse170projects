//
//  so_capsule.cpp
//  glutapp3d
//
//  Created by Nathaniel Lewis on 9/2/15.
//  Copyright (c) 2015 ucmerced. All rights reserved.
//

#include "so_capsule.h"
#include <cmath>

SoCapsule::SoCapsule()
{
    _numpoints = 0;
}

void SoCapsule::init ( const GlProgram& prog )
{
    // Define buffers needed:
    set_program ( prog );
    gen_vertex_arrays ( 1 ); // will use 1 vertex array
    gen_buffers ( 4 );       // will use 3 buffers: one for coordinates, one for colors, one for indices
    uniform_locations ( 2 ); // will send 2 variables: the 2 matrices below
    uniform_location ( 0, "vTransf" );
    uniform_location ( 1, "vProj" );
}

void SoCapsule::build ( float len, float rt, float rb, int nfaces )
{
    P.clear(); C.clear(); E.clear(); N.clear(); // set size to zero, just in case
   
    float hlen = len/2.0f;
    int   levels = std::ceil(static_cast<float>(nfaces) / 2.0f);
    float faceAngularHeight = (static_cast<float>(M_PI) / 2.0) / static_cast<float>(levels);
    float faceAngularLength = (2.0 * static_cast<float>(M_PI)) / static_cast<float>(nfaces);
    int   vertices = 2 + (2*levels + 1) * nfaces;
    int   indicies = (2*levels + 2) * (nfaces + 1);
    
    // Reserve space for the capsule vertices
    P.reserve(vertices);
    C.reserve(vertices);
    N.reserve(vertices);
    E.reserve(indicies);
    
    // Push the bottom vertex
    P.push_back(GsVec(0.0f, -hlen - rb, 0.0f)); C.push_back(GsColor::white); N.push_back(GsVec(0.0f, -1.0f, 0.0f));
    
    // Generate the bottom vertices
    for(int j = levels-1; j >= 0; j--)
    {
        // theta coordinate
        float t = static_cast<float>(j) * faceAngularHeight;
        for(int i = 0; i < nfaces; i++)
        {
            // phi coordinate
            float p = static_cast<float>(i) * faceAngularLength;
            GsVec vertex ((rb * std::cos(p)) * std::cos(t), -hlen - (rb * std::sin(t)), (rb * std::sin(p)) * std::cos(t));
            GsVec normal = vertex - GsVec(0.0f, -hlen, 0.0f);
            normal.normalize();
            
            P.push_back(vertex);
            N.push_back(normal);
            C.push_back(GsColor::white);
        }
    }
    
    // Generate the top vertices
    for(int j = 0; j < levels; j++)
    {
        // theta coordinate
        float t = static_cast<float>(j) * faceAngularHeight;
        for(int i = 0; i < nfaces; i++)
        {
            // phi coordinate
            float p = static_cast<float>(i) * faceAngularLength;
            GsVec vertex ((rt * std::cos(p)) * std::cos(t), hlen + (rt * std::sin(t)), (rt * std::sin(p)) * std::cos(t));
            GsVec normal = vertex - GsVec(0.0f, hlen, 0.0f);
            normal.normalize();
            
            P.push_back(vertex);
            N.push_back(normal);
            C.push_back(GsColor::white);
        }
    }

    P.push_back(GsVec(0.0f, hlen + rt, 0.0f)); C.push_back(GsColor::white); N.push_back(GsVec(0.0f, 1.0f, 0.0f));
    
    // Compute indices of bottom cap
    for(int j = 1; j <= nfaces; j++)
    {
        E.push_back(0); E.push_back(j);
    }
    E.push_back(0); E.push_back(1);
    
    // Compute indices of tube
    for(int i = 0; i < (levels * 2) - 1; i++)
    {
        int lIdxBase = 1 + (nfaces * i);
        int uIdxBase = 1 + (nfaces * (i + 1));
        
        for(int j = 0; j < nfaces; j++)
        {
            E.push_back(lIdxBase+j);
            E.push_back(uIdxBase+j);
        }
        
        E.push_back(lIdxBase);
        E.push_back(uIdxBase);
    }
    
    // Compute indices of top cap
    int lastIndex = P.size() - 1;
    for(int j = 0; j < nfaces; j++)
    {
        E.push_back((lastIndex - nfaces) + j); E.push_back(lastIndex);
    }
    E.push_back(lastIndex - nfaces); E.push_back(lastIndex);

    // send data to OpenGL buffers:
    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glBufferData ( GL_ARRAY_BUFFER, P.size()*3*sizeof(float), &P[0], GL_STATIC_DRAW );
    glBindBuffer ( GL_ARRAY_BUFFER, buf[1] );
    glBufferData ( GL_ARRAY_BUFFER, C.size()*4*sizeof(gsbyte), &C[0], GL_STATIC_DRAW );
    glBindBuffer ( GL_ARRAY_BUFFER, buf[2] );
    glBufferData ( GL_ARRAY_BUFFER, N.size()*3*sizeof(float), &N[0], GL_STATIC_DRAW );
    glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, buf[3] );
    glBufferData ( GL_ELEMENT_ARRAY_BUFFER, E.size()*sizeof(GLshort), &E[0], GL_STATIC_DRAW );
    
    // save size so that we can free our buffers and later just draw the OpenGL arrays:
    _numpoints = P.size();
    
    // free non-needed memory:
    P.resize(0); C.resize(0);
    changed = 0;
}

void SoCapsule::draw ( GsMat& tr, GsMat& pr )
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
    
    glBindBuffer ( GL_ARRAY_BUFFER, buf[2] ); // normals
    glEnableVertexAttribArray ( 2 );
    glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, buf[3] ); // indices
    
    glUniformMatrix4fv ( uniloc[0], 1, GL_FALSE, tr.e );
    glUniformMatrix4fv ( uniloc[1], 1, GL_FALSE, pr.e );
    
    //glDrawArrays ( GL_POINTS, 0, _numpoints );
    glDrawElements(GL_TRIANGLE_STRIP, E.size(), GL_UNSIGNED_SHORT, 0);
}
