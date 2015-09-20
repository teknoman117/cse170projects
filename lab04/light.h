#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <gsim/gs_vec.h>
#include <gsim/gs_color.h>

class Light
{
public:
    GsVec pos;
    GsColor amb, dif, spe; // ambient, diffuse, specular
    
    Light ( const GsVec p, const GsColor a, const GsColor d, const GsColor s )
        : pos(p), amb(a), dif(d), spe(s)
    {
    }
};

#endif
