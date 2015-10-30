#include "so_sphere.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159f
#endif

namespace
{
    GsVec midpoint(GsVec& a, GsVec& b)
    {
        return (a+b)/2.0f;
    }
    
    // A class to manage the subdivision of the sphere
    struct Face
    {
        GsVec a, b, c;
        GsVec na, nb, nc;
        unsigned short subdivison;
        
        // method to generate a surface from this
        void build(std::vector<GsVec>& V, std::vector<GsVec>& N, float radius, unsigned short lod)
        {
            // If this level is the desired, write it out
            if(lod == subdivison)
            {
                V.push_back(a); N.push_back(na);
                V.push_back(b); N.push_back(nb);
                V.push_back(c); N.push_back(nc);
                return;
            }
            
            // Compute the edges
            GsVec nap = midpoint(a, b); nap.normalize();
            GsVec nbp = midpoint(b, c); nbp.normalize();
            GsVec ncp = midpoint(c, a); ncp.normalize();
            GsVec ap = nap * radius;
            GsVec bp = nbp * radius;
            GsVec cp = ncp * radius;
            
            // Create the 4 subdivided faces
            Face f;
            f.subdivison = subdivison+1;
            
            // face 1 (center)
            f.a = ap; f.na = nap;
            f.b = bp; f.nb = nbp;
            f.c = cp; f.nc = ncp;
            f.build(V, N, radius, lod);
            
            // face 2 (north)
            f.a = a; f.na = na;
            f.b = ap; f.nb = nap;
            f.c = cp; f.nc = ncp;
            f.build(V, N, radius, lod);
            
            // face 3 (south east)
            f.a = ap; f.na = nap;
            f.b = b; f.nb = nb;
            f.c = bp; f.nc = nbp;
            f.build(V, N, radius, lod);
            
            // face 4 (south west)
            f.a = cp; f.na = ncp;
            f.b = bp; f.nb = nbp;
            f.c = c; f.nc = nc;
            f.build(V, N, radius, lod);
        }
    };
}

void SoSphere::init ()
{
    glGenBuffers(2, &_buffers[0]);
}

void SoSphere::build ( float radius, unsigned short resolution )
{
    std::vector<GsVec> V, N;
    V.reserve(8 * pow( 4, resolution ) * 3);
    N.reserve(8 * pow( 4, resolution ) * 3);
    
    // Generate the 8 faces of the sphere
    Face f;
    f.subdivison = 0;
    
    // Face 1
    f.na = GsVec(0,radius,0); f.na.normalize(); f.a = f.na * radius;
    f.nb = GsVec(0,0,-radius); f.nb.normalize(); f.b = f.nb * radius;
    f.nc = GsVec(radius,0,0); f.nc.normalize(); f.c = f.nc * radius;
    f.build(V, N, radius, resolution);
    
    // Face 2
    f.na = GsVec(0,radius,0); f.na.normalize(); f.a = f.na * radius;
    f.nb = GsVec(-radius,0,0); f.nb.normalize(); f.b = f.nb * radius;
    f.nc = GsVec(0,0,-radius); f.nc.normalize(); f.c = f.nc * radius;
    f.build(V, N, radius, resolution);
    
    // Face 3
    f.na = GsVec(0,radius,0); f.na.normalize(); f.a = f.na * radius;
    f.nb = GsVec(0,0,radius); f.nb.normalize(); f.b = f.nb * radius;
    f.nc = GsVec(-radius,0,0); f.nc.normalize(); f.c = f.nc * radius;
    f.build(V, N, radius, resolution);
    
    // Face 4
    f.na = GsVec(0,radius,0); f.na.normalize(); f.a = f.na * radius;
    f.nb = GsVec(radius,0,0); f.nb.normalize(); f.b = f.nb * radius;
    f.nc = GsVec(0,0,radius); f.nc.normalize(); f.c = f.nc * radius;
    f.build(V, N, radius, resolution);
    
    // Face 5
    f.na = GsVec(0,0,-radius); f.na.normalize(); f.a = f.na * radius;
    f.nb = GsVec(0,-radius,0); f.nb.normalize(); f.b = f.nb * radius;
    f.nc = GsVec(radius,0,0); f.nc.normalize(); f.c = f.nc * radius;
    f.build(V, N, radius, resolution);
    
    // Face 6
    f.na = GsVec(-radius,0,0); f.na.normalize(); f.a = f.na * radius;
    f.nb = GsVec(0,-radius,0); f.nb.normalize(); f.b = f.nb * radius;
    f.nc = GsVec(0,0,-radius); f.nc.normalize(); f.c = f.nc * radius;
    f.build(V, N, radius, resolution);
    
    // Face 7
    f.na = GsVec(0,0,radius); f.na.normalize(); f.a = f.na * radius;
    f.nb = GsVec(0,-radius,0); f.nb.normalize(); f.b = f.nb * radius;
    f.nc = GsVec(-radius,0,0); f.nc.normalize(); f.c = f.nc * radius;
    f.build(V, N, radius, resolution);
    
    // Face 8
    f.na = GsVec(radius,0,0); f.na.normalize(); f.a = f.na * radius;
    f.nb = GsVec(0,-radius,0); f.nb.normalize(); f.b = f.nb * radius;
    f.nc = GsVec(0,0,radius); f.nc.normalize(); f.c = f.nc * radius;
    f.build(V, N, radius, resolution);
    
    
    // send data to OpenGL buffers:
    glBindBuffer ( GL_ARRAY_BUFFER, _buffers[0] );
    glBufferData ( GL_ARRAY_BUFFER, V.size()*3*sizeof(float), &V[0], GL_STATIC_DRAW );
    glBindBuffer ( GL_ARRAY_BUFFER, _buffers[1] );
    glBufferData ( GL_ARRAY_BUFFER, N.size()*3*sizeof(float), &N[0], GL_STATIC_DRAW );
    
    // Final element count
    _numpoints = V.size();
    
    // Deallocate
    V.resize(0);
    N.resize(0);
}

void SoSphere::Renderer::init ( const GlProgram& prog )
{
    set_program ( prog );
    gen_vertex_arrays ( 1 ); // will use 1 vertex array
    
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
    
    // Update vertex array
    const SoSphere *oldsphere = sphere;
    sphere = NULL;
    SetSphere(oldsphere);
}

void SoSphere::Renderer::draw ( GsMat& tr, GsMat& pr, const Light& l, const Material& m )
{
    if(!sphere) return;
    
    // Draw Lines:
    glUseProgram ( prog );
    glBindVertexArray ( va[0] );
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
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) sphere->_numpoints);
}

void SoSphere::Renderer::SetSphere(const SoSphere *sphere)
{
    if(sphere == NULL || sphere == this->sphere)
        return;
    
    // construct vertex array
    glBindVertexArray ( va[0] );
    
    glBindBuffer ( GL_ARRAY_BUFFER, sphere->_buffers[0] );
    glEnableVertexAttribArray ( 0 );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glBindBuffer ( GL_ARRAY_BUFFER, sphere->_buffers[1] );
    glEnableVertexAttribArray ( 1 );
    glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    this->sphere = sphere;
}

const SoSphere* SoSphere::Renderer::GetSphere() const
{
    return sphere;
}

