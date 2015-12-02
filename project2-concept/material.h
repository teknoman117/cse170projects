#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <gsim/gs_vec.h>
#include <gsim/gs_color.h>

class Material
{
public :
    GsColor amb, dif, spe; // ambient, diffuse, specular
    float sh; // shininess
    
    Material ( const GsColor a, const GsColor d, const GsColor s, float sn )
        : amb(a), dif(d), spe(s), sh(sn)
    {
    }
};

#endif
