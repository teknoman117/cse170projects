// Ensure the header file is included only once in multi-file projects
#ifndef SO_TERRAIN_H
#define SO_TERRAIN_H

// Include needed header files
#include <project2/common.hpp>
#include <project2/objects/globject.hpp> 

#include <vector>

class GLTerrain : public GLObject
{
    std::vector<glm::vec3> V;
    
    size_t width;
    size_t height;
    size_t numPoints;
    
    glm::vec3 convertToMeters;
    
public:
    GLTerrain ( std::string filename, size_t width, size_t height, glm::vec3 convertToMeters = glm::vec3(1,1,1) );
    void Draw();
    
    float sampleHeightAtPoint ( glm::vec2 point );
};

#endif 