#ifndef __CHUNKED_TERRAIN_H__
#define __CHUNKED_TERRAIN_H__

#include <project2/objects/globject.hpp>
#include <project2/program.hpp>
#include <project2/persistentbuffer.hpp>

#include <project2/geometry/aabox.hpp>
#include <project2/geometry/frustum.hpp>

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
        AABox               bounds;

        // Build the chunk
        void Build(const std::vector<Vertex>& data, size_t dataWidth, size_t dataHeight, size_t x, size_t y);
    };

    size_t dataWidth;
    size_t dataHeight;

    size_t chunkGridWidth;
    size_t chunkGridHeight;

    PersistentBuffer<GLuint> commandBuffer;

    std::vector<Vertex> vertices;
    std::vector<Chunk>  chunks;
    std::vector<GLuint> indices;

    // Corners, cardinal directions, boundary normals, midpoint
    glm::dvec3 ne, nw, se, sw;
    glm::dvec3 n,  w,  s,  e;
    glm::dvec3 nn, wn, sn, en;
    glm::dvec3 midpoint;

    // Quaternion describing rotation into flattened plane
    glm::dquat projector;

    // Compute all the projection data from defined input
    void ComputeBounds(size_t width, size_t height, glm::dvec2 resolution, glm::dvec2 corner);

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

    void      Draw(const Frustum& cameraFrustum);

    glm::vec3 GetLocationOfCoordinate(glm::dvec2 coordinate);
    float     GetElevationAt(glm::vec3 location);
};

#endif
