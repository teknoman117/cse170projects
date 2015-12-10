#ifndef __CHUNKED_TERRAIN_H__
#define __CHUNKED_TERRAIN_H__

#include <project2/objects/globject.hpp>
#include <project2/program.hpp>

#include <vector>

/**
 * An implementation of a terrain built from chunks.  Performs all functions
 * such as loading terrain, generating chunks, etc.  The chunked terrain
 * parent class is an OpenGL object representing the data from the whole 
 * chunk.  The chunks themselves only represent the AABB of the chunk itself
 * and index data required to render it.
 */
class ChunkedTerrain : public GLObject
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
    };

    struct Chunk
    {
        std::vector<GLuint> indices;
        
        // axis aligned bounding box
        glm::vec3 a;
        glm::vec3 b;
    };

    size_t width;
    size_t height;
    size_t elementsPerRow;

    size_t chunkGridWidth;
    size_t chunkGridHeight;

    std::vector<Chunk> chunks;

    // Corners, boundary normals, midpoint
    glm::dvec3 ne, nw, se, sw;
    glm::dvec3 n,  w,  s,  e;
    glm::dvec3 nn, wn, sn, en;
    glm::dvec3 midpoint;

    // rotate points into projected region
    glm::dquat projector;

public:
    /**
     * Generate terrain from raw dump
     * @param path The path to the raw file dump
     * @param width the width of the raster to be read
     * @param height the height of the raster to be read
     * @param resolution the per pixel resolution in degrees
     * @param corner the northwest coordinate of the raster
     */
    ChunkedTerrain(const std::string& path, size_t width, size_t height, glm::dvec2 resolution, glm::dvec2 corner);
    virtual ~ChunkedTerrain();

    void      Draw(const std::shared_ptr<Program>& program);

    glm::vec3 GetLocationOfCoordinate(glm::dvec2 coordinate);
    float     GetElevationAt(glm::vec3 location);
};

#endif
