#include "so_model.h"


SoModel::SoModel()
{
    _numpoints = 0;
}

void SoModel::init ( const GlProgram& prog )
{
    set_program ( prog );
    gen_vertex_arrays ( 1 );
    gen_buffers ( 1 );
    
    uniform_locations ( 10 );
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

void SoModel::build ( const Model& model )
{
    // Allocate a buffer for the model face data
    std::vector<struct Vertex> V;
    V.reserve(model.fsize * 3);
    
    // Because this format is fail and doesn't have a unified vertex index (vertex, normal, color, etc), we combine manually
    for(size_t i = 0; i < model.fsize; i++)
    {
        struct Vertex a, b, c;
        
        Model::Fac face = model.F[i];
        GsColor    facecolor(face.r, face.g, face.b, 255);
        
        a.vertex = model.V[face.va];
        a.normal = model.N[face.na];
        a.color = facecolor;
        
        b.vertex = model.V[face.vb];
        b.normal = model.N[face.nb];
        b.color = facecolor;
        
        c.vertex = model.V[face.vc];
        c.normal = model.N[face.nc];
        c.color = facecolor;
        
        V.push_back(a); V.push_back(b); V.push_back(c);
    }

    // send data to OpenGL buffers:
    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glBufferData ( GL_ARRAY_BUFFER, V.size()*sizeof(struct Vertex), &V[0], GL_STATIC_DRAW );
    
    // Store the number of primitives
    _numpoints = V.size();
}

void SoModel::draw ( GsMat& tr, GsMat& pr, const Light& l, const Material& m  )
{
    // Draw Lines:
    glUseProgram ( prog );
    glBindVertexArray ( va[0] );

    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glEnableVertexAttribArray ( 0 );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (GLvoid *) offsetof(struct Vertex, vertex) );

    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glEnableVertexAttribArray ( 1 );
    glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (GLvoid *) offsetof(struct Vertex, normal) );

    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glEnableVertexAttribArray ( 2 );
    glVertexAttribPointer ( 2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct Vertex), (GLvoid *) offsetof(struct Vertex, color) );

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
    
    glDrawArrays ( GL_TRIANGLES, 0, (GLsizei) _numpoints );
}

