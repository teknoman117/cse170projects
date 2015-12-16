#ifndef __CHUNKED_TERRAIN_H__
#define __CHUNKED_TERRAIN_H__

#include <project2/objects/globject.hpp>
#include <project2/program.hpp>
#include <project2/texture.hpp>

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
    // Parameters we can pass to the shader
    struct TerrainShaderParameters
    {
        glm::ivec2 rasterSize;
        glm::ivec2 dataSize;
        glm::ivec2 gridSize;
        glm::ivec2 chunkSize;

        glm::vec3 ne;
        glm::vec3 nw;
        glm::vec3 se;
        glm::vec3 sw;

        float triSize;
    } shaderParameters;
    
    std::shared_ptr<Texture> heightmap;
    std::vector<float>       raster;

    // Corners, cardinal directions, boundary normals, midpoint
    glm::dvec3 ne, nw, se, sw;
    glm::dvec3 n,  w,  s,  e;
    glm::dvec3 nn, wn, sn, en;
    glm::dvec3 midpoint;
    glm::dquat projector;

    // Compute all the projection data from defined input
    void      ComputeBounds(glm::ivec2 size, glm::dvec2 resolution, glm::dvec2 corner);
    glm::vec3 ComputeVertexPosition(size_t x, size_t y);

public:
    /**
     * Generate terrain from raw dump
     * @param path The path to the raw file dump
     * @param width the width of the raster to be read
     * @param height the height of the raster to be read
     * @param resolution the per pixel resolution in degrees
     * @param corner the northwest coordinate of the raster
     */
    ChunkedTerrain(const std::string& path, glm::ivec2 rasterSize, glm::dvec2 resolution, glm::dvec2 corner);
    virtual ~ChunkedTerrain();

    void      Draw(const std::shared_ptr<Program>& program);

    glm::vec3 GetLocationOfWGS84Coordinate(glm::dvec2 coordinate);
    float     GetElevationAt(glm::vec3 location);
};

#endif
