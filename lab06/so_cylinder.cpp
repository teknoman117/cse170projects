
# include "so_cylinder.h"
# include <cmath>

SoTexturedCylinder::SoTexturedCylinder()
 {
   _numpoints = 0;
   _phong = false;
   _flatn = true;
 }

void SoTexturedCylinder::init ()
 {
   // Load programs:
     
     
#if (defined WIN32)
     _vshgou.load_and_compile ( GL_VERTEX_SHADER, "../vsh_mcol_gouraud.glsl" );
     _fshgou.load_and_compile ( GL_FRAGMENT_SHADER, "../fsh_gouraud.glsl" );
#else
     _vshgou.load_and_compile ( GL_VERTEX_SHADER, "vsh_mcol_gouraud.glsl" );
     _fshgou.load_and_compile ( GL_FRAGMENT_SHADER, "fsh_gouraud.glsl" );
#endif
     
     _proggouraud.init_and_link ( _vshgou, _fshgou );
     
#if (defined WIN32)
     _vshphong.load_and_compile ( GL_VERTEX_SHADER, "../vsh_mcol_phong.glsl" );
     _fshphong.load_and_compile ( GL_FRAGMENT_SHADER, "../fsh_mcol_phong.glsl" );
#else
     _vshphong.load_and_compile ( GL_VERTEX_SHADER, "vsh_mcol_phong.glsl" );
     _fshphong.load_and_compile ( GL_FRAGMENT_SHADER, "fsh_mcol_phong.glsl" );
#endif

   _progphong.init_and_link ( _vshphong, _fshphong );

   // Define buffers needed:
   gen_vertex_arrays ( 1 ); // will use 1 vertex array
   gen_buffers ( 3 );       // will use 3 buffers

   _proggouraud.uniform_locations ( 11 ); // will send 9 variables
   _proggouraud.uniform_location ( 0, "vTransf" );
   _proggouraud.uniform_location ( 1, "vProj" );
   _proggouraud.uniform_location ( 2, "lPos" );
   _proggouraud.uniform_location ( 3, "la" );
   _proggouraud.uniform_location ( 4, "ld" );
   _proggouraud.uniform_location ( 5, "ls" );
   _proggouraud.uniform_location ( 6, "ka" );
   _proggouraud.uniform_location ( 7, "kd" );
   _proggouraud.uniform_location ( 8, "ks" );
   _proggouraud.uniform_location ( 9, "sh" );
   _proggouraud.uniform_location(10, "Texture");
     _proggouraud.uniform_location(11, "BlendFactor");
     
   _progphong.uniform_locations ( 11 ); // will send 9 variables
   _progphong.uniform_location ( 0, "vTransf" );
   _progphong.uniform_location ( 1, "vProj" );
   _progphong.uniform_location ( 2, "lPos" );
   _progphong.uniform_location ( 3, "la" );
   _progphong.uniform_location ( 4, "ld" );
   _progphong.uniform_location ( 5, "ls" );
   _progphong.uniform_location ( 6, "ka" );
   _progphong.uniform_location ( 7, "kd" );
   _progphong.uniform_location ( 8, "ks" );
   _progphong.uniform_location ( 9, "sh" );
   _progphong.uniform_location(10, "Texture");
     _progphong.uniform_location(11, "BlendFactor");
 }

void SoTexturedCylinder::build ( float rt, float rb, float len, int numfaces )
{
    // Build the cylinder
    GsArray<GsVec>  P;
    GsArray<GsVec>  N;
    GsArray<GsVec2> T;
    
    P.size(0); N.size(0); T.size(0); NL.size(0);
    
    // Generate the walls of the cyliner
    float angularLength = (2.0f * M_PI) / static_cast<float>(numfaces);
    float hlen = len/2.0f;
    
    for(int f = 0; f <= numfaces; f++)
    {
        float theta = angularLength * static_cast<float>(f);
        
        // lower vertex
        P.push() = GsVec(rb * std::sin(theta), -hlen, rb * std::cos(theta));
        N.push() = GsVec(std::sin(theta), 0.0f, std::cos(theta));
        T.push() = GsVec2(static_cast<float>(f) / static_cast<float>(numfaces), 1.0f);
        
        NL.push() = P.top();
        NL.push() = P.top() + (0.05f * N.top());
        
        // upper vertex
        P.push() = GsVec(rt * std::sin(theta), hlen, rt * std::cos(theta));
        N.push() = GsVec(std::sin(theta), 0.0f, std::cos(theta));
        T.push() = GsVec2(static_cast<float>(f) / static_cast<float>(numfaces), 0.0f);
        
        NL.push() = P.top();
        NL.push() = P.top() + (0.05f * N.top());
    }
    
    
    _mtl.specular.set ( 255,255,255 );
    _mtl.shininess = 8; // increase specular effect

    // send data to OpenGL buffers:
    glBindVertexArray ( va[0] );
    glEnableVertexAttribArray ( 0 );
    glEnableVertexAttribArray ( 1 );
    glEnableVertexAttribArray ( 2 );

    glBindBuffer ( GL_ARRAY_BUFFER, buf[0] );
    glBufferData ( GL_ARRAY_BUFFER, 3*sizeof(float)*P.size(), P.pt(), GL_STATIC_DRAW );
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    glBindBuffer ( GL_ARRAY_BUFFER, buf[1] );
    glBufferData ( GL_ARRAY_BUFFER, 3*sizeof(float)*N.size(), N.pt(), GL_STATIC_DRAW );
    glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    glBindBuffer ( GL_ARRAY_BUFFER, buf[2] );
    glBufferData ( GL_ARRAY_BUFFER, 2*sizeof(float)*T.size(), T.pt(), GL_STATIC_DRAW );
    glVertexAttribPointer ( 2, 2, GL_FLOAT, GL_FALSE, 0, 0 );

    glBindVertexArray(0); // break the existing vertex array object binding.

    // save size so that we can free our buffers and later draw the OpenGL arrays:
    _numpoints = P.size();
}

void SoTexturedCylinder::draw ( const GsMat& tr, const GsMat& pr, const GsLight& l, int textureId, float blendFactor )
{
    float f[4];
    float sh = (float) _mtl.shininess;
    if ( sh<0.001f ) sh=64;

    if ( _phong )
    {
        glUseProgram ( _progphong.id );
        glUniformMatrix4fv ( _progphong.uniloc[0], 1, GL_FALSE, tr.e );
        glUniformMatrix4fv ( _progphong.uniloc[1], 1, GL_FALSE, pr.e );
        glUniform3fv ( _progphong.uniloc[2], 1, l.pos.e );
        glUniform4fv ( _progphong.uniloc[3], 1, l.amb.get(f) );
        glUniform4fv ( _progphong.uniloc[4], 1, l.dif.get(f) );
        glUniform4fv ( _progphong.uniloc[5], 1, l.spe.get(f) );
        glUniform4fv ( _progphong.uniloc[6], 1, _mtl.ambient.get(f) );
        glUniform4fv ( _progphong.uniloc[7], 1, _mtl.diffuse.get(f) );
        glUniform4fv ( _progphong.uniloc[8], 1, _mtl.specular.get(f) );
        glUniform1fv ( _progphong.uniloc[9], 1, &sh );
        glUniform1i ( _progphong.uniloc[10], textureId );
        glUniform1f ( _progphong.uniloc[11], blendFactor );
        
    }
    
    else
    {
        glUseProgram ( _proggouraud.id );
        glUniformMatrix4fv ( _proggouraud.uniloc[0], 1, GL_FALSE, tr.e );
        glUniformMatrix4fv ( _proggouraud.uniloc[1], 1, GL_FALSE, pr.e );
        glUniform3fv ( _proggouraud.uniloc[2], 1, l.pos.e );
        glUniform4fv ( _proggouraud.uniloc[3], 1, l.amb.get(f) );
        glUniform4fv ( _proggouraud.uniloc[4], 1, l.dif.get(f) );
        glUniform4fv ( _proggouraud.uniloc[5], 1, l.spe.get(f) );
        glUniform4fv ( _proggouraud.uniloc[6], 1, _mtl.ambient.get(f) );
        glUniform4fv ( _proggouraud.uniloc[7], 1, _mtl.diffuse.get(f) );
        glUniform4fv ( _proggouraud.uniloc[8], 1, _mtl.specular.get(f) );
        glUniform1fv ( _proggouraud.uniloc[9], 1, &sh );
        glUniform1i ( _proggouraud.uniloc[10], textureId );
        glUniform1f (_proggouraud.uniloc[11], blendFactor );
    }

    glBindVertexArray ( va[0] );
    glDrawArrays ( GL_TRIANGLE_STRIP, 0, _numpoints );
    glBindVertexArray(0); // break the existing vertex array object binding.
}

