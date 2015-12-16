#include <project2/chunkedterrain.hpp>

#include <cmath>
#include <fstream>
#include <cinttypes>
#include <iomanip>
#include <limits>
#include <chrono>
#include <omp.h>

using namespace glm;
using namespace std::chrono;

namespace
{
    const double earthRadius = 6371000.0;

    dquat rotationFromTo(dvec3 u, dvec3 v)
    {
        dvec3 w = cross(u, v);
        dquat q = quat(1.f + dot(u, v), w.x, w.y, w.z);
        return normalize(q);
    }
}

/**
 * Generate terrain from raw dump
 * @param path The path to the raw file dump
 * @param width the width of the raster to be read
 * @param height the height of the raster to be read
 * @param resolution the per pixel resolution in degrees
 * @param nwCoordinate the northwest coordinate of the raster
 */
ChunkedTerrain::ChunkedTerrain(const std::string& path, ivec2 rasterSize, dvec2 resolution, dvec2 corner)
    : GLObject(2)
{
    // Compute the chunk grid properties
    shaderParameters.rasterSize = rasterSize;
    shaderParameters.chunkSize  = ivec2(8,8);
    shaderParameters.gridSize   = (rasterSize-1) / shaderParameters.chunkSize;
    shaderParameters.dataSize   = shaderParameters.gridSize*shaderParameters.chunkSize + 1;
    shaderParameters.triSize    = 5.f;

    std::cout << "raster: " << shaderParameters.rasterSize.x << " " << shaderParameters.rasterSize.y << std::endl;
    std::cout << "chunk: " << shaderParameters.chunkSize.x << " " << shaderParameters.chunkSize.y << std::endl;
    std::cout << "grid: " << shaderParameters.gridSize.x << " " << shaderParameters.gridSize.y << std::endl;
    std::cout << "data: " << shaderParameters.dataSize.x << " " << shaderParameters.dataSize.y << std::endl;

    // Compute the projection parameters
    ComputeBounds(shaderParameters.dataSize, resolution, corner);

    // Upload terrain data to GPU
    glBindBuffer(GL_UNIFORM_BUFFER, buffers[1]);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(struct TerrainShaderParameters), (GLvoid *) &shaderParameters, GL_STATIC_DRAW);

    // Read the elevation data from the raster
    raster.reserve(rasterSize.x * rasterSize.y);
    {
        std::ifstream inputFile(path, std::ifstream::binary | std::ifstream::in);
        if(!inputFile.is_open()) 
        {
            std::cerr << "[FATAL] [TERRAIN \"" << path << "\"] Unable to open resource" << std::endl;
            throw std::runtime_error("Unable to open resource");
        }
        
        inputFile.seekg(0, inputFile.end);
        size_t actualSize = inputFile.tellg();
        inputFile.seekg(0, inputFile.beg);

        size_t expectedSize = sizeof(float) * rasterSize.x * rasterSize.y;
        if(expectedSize != actualSize)
        {
            std::cerr << "[FATAL] [TERRAIN \"" << path << "\"] Loaded resource is not of dimension: ";
            std::cerr << shaderParameters.rasterSize.x << " " << shaderParameters.rasterSize.y << std::endl;
            throw std::runtime_error("Resource does not conform to expectations");
        }

        // Read all the data from the terrain heightmap
        raster.resize(actualSize/4);
        inputFile.read((char *) raster.data(), actualSize);
        heightmap = std::make_shared<Texture>(rasterSize.x, rasterSize.y, GL_R32F, GL_RED, GL_FLOAT, raster.data());
    }

    // Compute minimum and maximum heights of chunks
    {
        std::vector<vec2> chunkHeights;
        chunkHeights.resize(shaderParameters.gridSize.x * shaderParameters.gridSize.y);
        for(size_t i = 0; i < chunkHeights.size(); i++)
            chunkHeights[i] = vec2(std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());

        #pragma omp parallel for collapse(2)
        for(int j = 0; j < shaderParameters.gridSize.y; j++)
        {
            for(int i = 0; i < shaderParameters.gridSize.x; i++)
            {
                vec2&  chunkHeight = *(chunkHeights.data() + j*shaderParameters.gridSize.x + i);
                float* chunkRasterStart = raster.data() + (j*shaderParameters.chunkSize.y*shaderParameters.rasterSize.x) + (i*shaderParameters.chunkSize.x);

                // Find the minimum and maximum height of the chunk
                for(int jChunk = 0; jChunk < shaderParameters.chunkSize.y; jChunk++)
                {
                    float *rasterRow = chunkRasterStart + (jChunk*shaderParameters.rasterSize.x);

                    for(int iChunk = 0; iChunk < shaderParameters.chunkSize.x; iChunk++)
                    {
                        chunkHeight.x = min<float>(rasterRow[iChunk], chunkHeight.x);
                        chunkHeight.y = max<float>(rasterRow[iChunk], chunkHeight.y);
                    }
                }
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*chunkHeights.size(), chunkHeights.data(), GL_STATIC_DRAW);
    }

    // Pass tile maximum height
    glBindVertexArray(vertexArrayObject);
    {
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);
        glVertexAttribDivisor(0, 1);
    }
    glBindVertexArray(0);

    std::cout << "[INFO] [TERRAIN \"" << path << "\"] Dimensions:" << shaderParameters.gridSize.x << "," << shaderParameters.gridSize.y << " chunks"<< std::endl;
}

ChunkedTerrain::~ChunkedTerrain()
{
}

// Compute the projection of the chunk into the xz plane
void ChunkedTerrain::ComputeBounds(ivec2 size, glm::dvec2 resolution, glm::dvec2 corner)
{
    // Compute the corners of the area this represents on Earth (remap onto left handed coordinates)
    double latitude  = (glm::pi<double>()/2.0) - corner.y;
    double longitude = corner.x;

    nw = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    nw = dvec3(-nw.y, nw.z, -nw.x);


    latitude =  (glm::pi<double>()/2.0) - corner.y;
    longitude = corner.x + double(size.x)*resolution.x;

    ne = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    ne = dvec3(-ne.y, ne.z, -ne.x);


    latitude  = ((glm::pi<double>()/2.0) - corner.y) + (double(size.y)*resolution.y);
    longitude = corner.x;

    sw = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    sw = dvec3(-sw.y, sw.z, -sw.x);


    latitude  = ((glm::pi<double>()/2.0) - corner.y) + (double(size.y)*resolution.y);
    longitude = corner.x + double(size.x)*resolution.x;

    se = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    se = dvec3(-se.y, se.z, -se.x);
    
    // Compute the midpoint and cardinal points
    n = (nw+ne)/2.0;
    s = (sw+se)/2.0;
    w = (nw+sw)/2.0;
    e = (ne+se)/2.0;
    midpoint = (n+s)/2.0;

    nw = nw - midpoint;
    ne = ne - midpoint;
    se = se - midpoint;
    sw = sw - midpoint;

    // Compute the surface normal of the patch
    dvec3 a = cross(normalize(ne), normalize(nw));
    dvec3 b = cross(normalize(sw), normalize(se));
    dvec3 N = normalize((a+b)/2.0);

    // Project from sphere component onto a flat surface
    projector = rotationFromTo(N, dvec3(0,1,0));

    ne = projector*ne;
    nw = projector*nw;
    se = projector*se;
    sw = projector*sw;

    n = projector*n;
    s = projector*s;
    w = projector*w;
    e = projector*e;

    // Compute normal vectors for the lines that define the boundaries
    dvec3 t = ne - nw;
    nn = normalize(dvec3(-t.z,0,t.x));
    if(dot(nn,n)<0)
        nn = normalize(dvec3(t.z,0,-t.x));

    t = nw - sw;
    wn = normalize(dvec3(-t.z,0,t.x));
    if(dot(wn,w)<0)
        wn = normalize(dvec3(t.z,0,-t.x));

    t = sw - se;
    sn = normalize(dvec3(-t.z,0,t.x));
    if(dot(sn,s)<0)
        sn = normalize(dvec3(t.z,0,-t.x));

    t = se - ne;
    en = normalize(dvec3(-t.z,0,t.x));
    if(dot(en,e)<0)
        en = normalize(dvec3(t.z,0,-t.x));

    std::cout << "ne " << ne.x << " " << ne.y << " " << ne.z << std::endl;
    std::cout << "nw " << nw.x << " " << nw.y << " " << nw.z << std::endl;
    std::cout << "se " << se.x << " " << se.y << " " << se.z << std::endl;
    std::cout << "sw " << sw.x << " " << sw.y << " " << sw.z << std::endl;

    shaderParameters.ne = ne;
    shaderParameters.nw = nw;
    shaderParameters.se = se;
    shaderParameters.sw = sw;
}

vec3 ChunkedTerrain::ComputeVertexPosition(size_t i, size_t j)
{
    // Compute the position of each of the elements of the grid
    float  *rasterRow = raster.data() + j*shaderParameters.rasterSize.x;
    dvec2 blend = dvec2(i,j) / dvec2(shaderParameters.dataSize-1);
    dvec3 location = mix( mix(nw,ne,blend.x), mix(sw,se,blend.x), blend.y);

    return vec3(location.x, rasterRow[i], location.z);    
}

float ChunkedTerrain::GetElevationAt(glm::vec3 location)
{
    glm::dvec3 position(location.x, 0, location.z);

    // Distance to each side of the figure
    double nDist = dot(nw - position, nn);
    double wDist = dot(sw - position, wn);
    double sDist = dot(se - position, sn);
    double eDist = dot(ne - position, en);

    // If we are outside the area, bail
    if(nDist < 0 || wDist < 0 || sDist < 0 || eDist < 0)
        return std::numeric_limits<float>::infinity();

    // Get the coordinates
    dvec2 coordinate = dvec2(wDist / (wDist + eDist), nDist / (nDist + sDist)) * 
                       dvec2(shaderParameters.dataSize-1);

    size_t x1 = floor(coordinate.x);
    size_t x2 = ceil(coordinate.x);
    size_t y1 = floor(coordinate.y);
    size_t y2 = ceil(coordinate.y);

    vec3 v00 = ComputeVertexPosition(x1, y1);
    vec3 v01 = ComputeVertexPosition(x1, y2);
    vec3 v10 = ComputeVertexPosition(x2, y1);
    vec3 v11 = ComputeVertexPosition(x2, y2);
    
    vec3 a = mix(v00, v10, coordinate.x - floor(coordinate.x));
    vec3 b = mix(v01, v11, coordinate.x - floor(coordinate.x));
    vec3 c = mix(a, b, coordinate.y - floor(coordinate.y));

    return c.y;
}

glm::vec3 ChunkedTerrain::GetLocationOfWGS84Coordinate(glm::dvec2 coordinate)
{
    double latitude  = (glm::pi<double>()/2.0) - coordinate.y;
    double longitude = coordinate.x;

    dvec3 p = dvec3(earthRadius * cos(longitude) * sin(latitude),
                    earthRadius * sin(longitude)  * sin(latitude),
                    earthRadius * cos(latitude));
    p = dvec3(-p.y, p.z, -p.x);

    return projector*(p-midpoint);
}

void ChunkedTerrain::Draw(const std::shared_ptr<Program>& program)
{
    glBindVertexArray(vertexArrayObject);

    heightmap->Bind(GL_TEXTURE0);
    glUniform1i(program->GetUniform("heightmap"), 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, buffers[1]);

    glPatchParameteri(GL_PATCH_VERTICES, 1);
    glDrawArraysInstanced(GL_PATCHES, 0, 1, shaderParameters.gridSize.x * shaderParameters.gridSize.y);
}
