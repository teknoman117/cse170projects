//
//  so_capsule.cpp
//  glutapp3d
//
//  Created by Nathaniel Lewis on 9/2/15.
//  Copyright (c) 2015 ucmerced. All rights reserved.
//

#include "so_capsule.h"
#include <cmath>

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

    void pushLevel(std::vector<GsVec>& P, std::vector<GsColor>& C, std::vector<GsVec>& topPoints, std::vector<GsVec>& bottomPoints, int nfaces)
    {
        // Build the triangles
        std::vector<GsVec>::iterator btIt = topPoints.begin();
        std::vector<GsVec>::iterator bbIt = bottomPoints.begin();
        for(int i = 0; i < nfaces; i++, btIt++, bbIt++)
        {
            GsVec a = *btIt;
            GsVec b = (btIt + 1 != topPoints.end()) ? *(btIt + 1) : *(topPoints.begin());
            GsVec c = *bbIt;
            GsVec d = (bbIt + 1 != bottomPoints.end()) ? *(bbIt + 1) : *(bottomPoints.begin());
            
            pushTriangle(P, C, {a, b, c});
            pushTriangle(P, C, {c, d, b});
        }
    }

    void pushCap(std::vector<GsVec>& P, std::vector<GsColor> C, std::vector<GsVec>& points, GsVec v, int nfaces)
    {
        for(int i = 0; i < nfaces; i++)
        {
            GsVec a = points[i];
            GsVec b = (i+1 == nfaces) ? points[0] : points[i+1];

            pushTriangle(P, C, {a, b, v});
        }
    }
}

SoCapsule::SoCapsule()
{
    _numpoints = 0;
}

void SoCapsule::init ( const GlProgram& prog )
{
    // Define buffers needed:
    set_program ( prog );
    gen_vertex_arrays ( 1 ); // will use 1 vertex array
    gen_buffers ( 2 );       // will use 2 buffers: one for coordinates and one for colors
    uniform_locations ( 2 ); // will send 2 variables: the 2 matrices below
    uniform_location ( 0, "vTransf" );
    uniform_location ( 1, "vProj" );
}

void SoCapsule::build ( float len, float rt, float rb, int nfaces )
{
    P.clear(); C.clear(); // set size to zero, just in case
   
    float hlen = len/2.0f;
    int   levels = std::floor(static_cast<float>(nfaces) / 2.0f);
    float faceAngularHeight = (static_cast<float>(M_PI) / 2.0) / static_cast<float>(levels);
    float faceAngularLength = (2.0 * static_cast<float>(M_PI)) / static_cast<float>(nfaces);
    int   vertices = 2 + (2*levels + 1)*(nfaces * 6);

    // Reserve space for the capsule vertices
    P.reserve(vertices);
    C.reserve(vertices);

    // Generate the coordinates for the capsule body
    std::vector<GsVec> bodyTopCoordinates;
    std::vector<GsVec> bodyBottomCoordinates;
    bodyTopCoordinates.reserve(nfaces);
    bodyBottomCoordinates.reserve(nfaces);

    // Compute the tube section of the capsule
    for(int i = 0; i < nfaces; i++)
    {
        float p = static_cast<float>(i) * faceAngularLength;
        
        bodyTopCoordinates.push_back(GsVec(rt * std::cos(p), hlen, rt * std::sin(p)));
        bodyBottomCoordinates.push_back(GsVec(rb * std::cos(p), -hlen, rb * std::sin(p)));
    }
    pushLevel(P, C, bodyTopCoordinates, bodyBottomCoordinates, nfaces);

    // Compute the top of the capsule
    std::vector<GsVec>& previousTopVector = bodyTopCoordinates;
    std::vector<GsVec>  currentTopVector;
    currentTopVector.reserve(nfaces);

    for(int j = 1; j < levels; j++)
    {
        // theta coordinate
        float t = static_cast<float>(j) * faceAngularHeight;
        for(int i = 0; i < nfaces; i++)
        {
            // phi coordinate
            float p = static_cast<float>(i) * faceAngularLength;
            currentTopVector.push_back(GsVec((rt * std::cos(p)) * std::cos(t), 
                                             hlen + (rt * std::sin(t)), 
                                             (rt * std::sin(p)) * std::cos(t)));

        }
        pushLevel(P, C, currentTopVector, previousTopVector, nfaces);

        std::swap(currentTopVector, previousTopVector);
        currentTopVector.clear();
    }

    // Compute the cap of the capsule
    GsVec tv(0.0f, hlen + rt, 0.0f);
    for(int i = 0; i < nfaces; i++)
    {
        GsVec a = previousTopVector[i];
        GsVec b = (i+1 == nfaces) ? previousTopVector[0] : previousTopVector[i+1];

        pushTriangle(P, C, {a, b, tv});
    }

    // Compute the bottom of the capsule
    std::vector<GsVec>& previousBottomVector = bodyBottomCoordinates;
    std::vector<GsVec>  currentBottomVector;
    currentBottomVector.reserve(nfaces);
    
    for(int j = 1; j < levels; j++)
    {
        // theta coordinate
        float t = static_cast<float>(j) * faceAngularHeight;
        for(int i = 0; i < nfaces; i++)
        {
            // phi coordinate
            float p = static_cast<float>(i) * faceAngularLength;
            currentBottomVector.push_back(GsVec((rb * std::cos(p)) * std::cos(t), 
                                                -hlen - (rb * std::sin(t)), 
                                                (rb * std::sin(p)) * std::cos(t)));

        }
        pushLevel(P, C, currentBottomVector, previousBottomVector, nfaces);

        std::swap(currentBottomVector, previousBottomVector);
        currentBottomVector.clear();
    }

    // Compute the cap of the capsule
    GsVec bv(0.0f, -hlen - rb, 0.0f);
    for(int i = 0; i < nfaces; i++)
    {
        GsVec a = previousBottomVector[i];
        GsVec b = (i+1 == nfaces) ? previousBottomVector[0] : previousBottomVector[i+1];

        pushTriangle(P, C, {a, bv, b});
    }
    //pushCap(P, C, previousBottomVector, bv, nfaces);

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
    
    glUniformMatrix4fv ( uniloc[0], 1, GL_FALSE, tr.e );
    glUniformMatrix4fv ( uniloc[1], 1, GL_FALSE, pr.e );
    
    glDrawArrays ( GL_LINES, 0, _numpoints );
}
