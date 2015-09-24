#include "so_model_normals.h"

namespace
{
    struct Vertex
    {
        GsVec vertex;
        GsVec normal;
    };
}

SoModelNormals::SoModelNormals()
{
    
}

void SoModelNormals::init ( const GlProgram& prog )
{
    set_program ( prog );
    gen_vertex_arrays ( 1 );
    gen_buffers ( 2 );
    
    uniform_locations ( 2 );
    uniform_location ( 0, "vTransf" );
    uniform_location ( 1, "vProj" );
}

void SoModelNormals::build ( const Model& model )
{
    std::vector<GsVec> P;
    std::vector<GsColor> C;
    
    P.reserve(model.fsize * 4);
    C.reserve(model.fsize * 4);
    
    // Smooth normals
    _smoothStart = P.size();
    for(size_t i = 0; i < model.fsize; i++)
    {
        struct Vertex a, b, c;
        
        Model::Fac face = model.F[i];
        
        a.vertex = model.V[face.va];
        a.normal = model.N[face.na];
        
        b.vertex = model.V[face.vb];
        b.normal = model.N[face.nb];
        
        c.vertex = model.V[face.vc];
        c.normal = model.N[face.nc];
        
        P.push_back(a.vertex); C.push_back(GsColor::red);
        P.push_back(a.vertex + (a.normal * 2.5)); C.push_back(GsColor::red);
        
        P.push_back(b.vertex); C.push_back(GsColor::red);
        P.push_back(b.vertex + (b.normal * 2.5)); C.push_back(GsColor::red);
        
        P.push_back(c.vertex); C.push_back(GsColor::red);
        P.push_back(c.vertex + (c.normal * 2.5)); C.push_back(GsColor::red);
    }
    _smoothCount = P.size();
    
    // Flat normals
    _flatStart = P.size();
    for(size_t i = 0; i < model.fsize; i++)
    {
        struct Vertex a, b, c;
        
        Model::Fac face = model.F[i];
        
        a.vertex = model.V[face.va];
        a.normal = model.N[face.na];
        
        b.vertex = model.V[face.vb];
        b.normal = model.N[face.nb];
        
        c.vertex = model.V[face.vc];
        c.normal = model.N[face.nc];
        
        GsVec v = (a.vertex + b.vertex + c.vertex) / 3;
        GsVec n = (a.normal + b.normal + c.normal) / 3;
        n.normalize();
        
        P.push_back(v); C.push_back(GsColor::red);
        P.push_back(v + (n * 2.5)); C.push_back(GsColor::red);
    }
    _flatCount = P.size() - _smoothCount;
    
    // send data to OpenGL buffers:
    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glBufferData ( GL_ARRAY_BUFFER, P.size()*3*sizeof(float), &P[0], GL_STATIC_DRAW );
    glBindBuffer ( GL_ARRAY_BUFFER, buf[1] );
    glBufferData ( GL_ARRAY_BUFFER, C.size()*4*sizeof(gsbyte), &C[0], GL_STATIC_DRAW );
}

void SoModelNormals::draw ( GsMat& tr, GsMat& pr, bool showFlatNormals )
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
    
    if(showFlatNormals)
        glDrawArrays(GL_LINES, (GLint) _flatStart, (GLsizei) _flatCount);
    else
        glDrawArrays(GL_LINES, (GLint) _smoothStart, (GLsizei) _smoothCount);
}

