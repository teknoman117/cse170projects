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

//#define __PARALLELCULL;

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
ChunkedTerrain::ChunkedTerrain(const std::string& path, size_t rasterWidth, size_t rasterHeight, dvec2 resolution, dvec2 corner)
    : GLObject(2)
{
    // Read the elevation data from the raster
    std::vector<float> raster;
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

        size_t expectedSize = sizeof(float) * rasterWidth * rasterHeight;
        if(expectedSize != actualSize)
        {
            std::cerr << "[FATAL] [TERRAIN \"" << path << "\"] Loaded resource is not of dimension: " << rasterWidth << " " << rasterHeight << std::endl;
            throw std::runtime_error("Resource does not conform to expectations");
        }

        // Read all the data from the terrain heightmap
        raster.resize(actualSize/4);
        inputFile.read((char *) raster.data(), actualSize);
        heightmap = std::make_shared<Texture>(rasterWidth, rasterHeight, GL_R32F, GL_RED, GL_FLOAT, raster.data());
    }

    // Compute the chunk grid properties
    chunkGridWidth  = (rasterWidth-1) / 16;
    chunkGridHeight = (rasterHeight-1) / 16;
    dataWidth       = 16*chunkGridWidth + 1;
    dataHeight      = 16*chunkGridHeight + 1;

    ComputeBounds(dataWidth, dataHeight, resolution, corner);

    // Compute the position of each of the elements of the grid
    vertices.resize(dataWidth * dataHeight);
    for(size_t j = 0; j < dataHeight; j++)
    {
        Vertex *verticesRow = vertices.data() + j*dataWidth;
        float  *rasterRow   = raster.data() + j*rasterWidth;
        double  y           = double(j) / double(dataHeight-1);

        for(size_t i = 0; i < dataWidth; i++)
        {
            double x = double(i) / double(dataWidth-1);
            dvec3 location = mix( mix(nw,ne,x), mix(sw,se,x), y);

            verticesRow[i].position = vec3(location.x, rasterRow[i], location.z);
        }
    }

    // Allocate storage for the opengl resources
    chunks.resize(chunkGridWidth*chunkGridHeight);

    #pragma omp parallel for
    for(size_t j = 0; j < chunkGridHeight; j++)
    {
        Chunk *chunkRow = chunks.data() + j*chunkGridWidth;

        for(size_t i = 0; i < chunkGridWidth; i++)
        {
            chunkRow[i].Build(vertices, dataWidth, dataHeight, i, j);
        }
    }

    // Upload dummy data to GPU and setup vertex array object
    glBindVertexArray(vertexArrayObject);
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);

        std::vector<GLushort> dummy;
        dummy.resize(dataWidth*dataHeight);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLushort)*dummy.size(), dummy.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 1, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(GLshort), 0);
    }
    glBindVertexArray(0);

    size_t threadCount = 0;
    #pragma omp parallel
    {
        #pragma omp master
        threadCount = omp_get_num_threads();
    }

    // Command buffer for the terrain index data
    commandBuffers.resize(threadCount);
    for(size_t i = 0; i < threadCount; i++)
    {
        commandBuffers[i].CreateBuffer((chunkGridWidth * chunkGridHeight * chunks[0].indices.size()) / max<size_t>(1, threadCount/2));
    }

    std::cout << "[INFO] [TERRAIN \"" << path << "\"] Constructed with " << chunkGridWidth << "," << chunkGridHeight << " chunks, ";
    std::cout << "Culling using " << threadCount << " threads" << std::endl;
}

ChunkedTerrain::~ChunkedTerrain()
{
}

void ChunkedTerrain::Chunk::Build(const std::vector<Vertex>& data, size_t dataWidth, size_t dataHeight, size_t x, size_t y)
{
    size_t baseIndex   = (16*y*dataHeight) + 16*x;

    // rows*(columns*2 verts + 2 end verts) + (rows-1)*1 degenerate vert + primitiverestart
    indices.reserve(16*(16*2 + 2) + (16-1)*1 + 1);
    vec3 a = vec3(std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity());
    vec3 b = vec3(-std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity());

    // Generate the index buffer
    for(size_t j = 0; j < 16; j++)
    {
        if(j%2)
        {
            for(size_t i = 0; i < 17; i++)
            {
                size_t idxA = baseIndex + (j+0)*dataWidth+i;
                size_t idxB = baseIndex + (j+1)*dataWidth+i;

                indices.push_back((GLuint) idxA);
                indices.push_back((GLuint) idxB);

                a = min(a, min(data[idxA].position, data[idxB].position));
                b = max(b, max(data[idxA].position, data[idxB].position));
            }
        }

        else 
        {
            for(size_t ip = 17; ip > 0; ip--)
            {
                size_t i = ip - 1;

                size_t idxA = baseIndex + (j+0)*dataWidth+i;
                size_t idxB = baseIndex + (j+1)*dataWidth+i;

                indices.push_back((GLuint) idxA);
                indices.push_back((GLuint) idxB);

                a = min(a, min(data[idxA].position, data[idxB].position));
                b = max(b, max(data[idxA].position, data[idxB].position));
            }
        }

        if(j != 15)
        {
            indices.push_back(*(indices.end()-1));
        }
    }

    indices.push_back((GLuint) -1);
    bounds.setBox(a,b);
}

// Compute the projection of the chunk into the xz plane
void ChunkedTerrain::ComputeBounds(size_t width, size_t height, glm::dvec2 resolution, glm::dvec2 corner)
{
    // Compute the corners of the area this represents on Earth (remap onto left handed coordinates)
    double latitude  = (glm::pi<double>()/2.0) - corner.y;
    double longitude = corner.x;

    nw = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    nw = dvec3(-nw.y, nw.z, -nw.x);


    latitude =  (glm::pi<double>()/2.0) - corner.y;
    longitude = corner.x + double(width)*resolution.x;

    ne = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    ne = dvec3(-ne.y, ne.z, -ne.x);


    latitude  = ((glm::pi<double>()/2.0) - corner.y) + (double(height)*resolution.y);
    longitude = corner.x;

    sw = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    sw = dvec3(-sw.y, sw.z, -sw.x);


    latitude  = ((glm::pi<double>()/2.0) - corner.y) + (double(height)*resolution.y);
    longitude = corner.x + double(width)*resolution.x;

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
    double x = (wDist / (wDist + eDist)) * (dataWidth-1);
    double y = (nDist / (nDist + sDist)) * (dataHeight-1);

    size_t x1 = floor(x);
    size_t x2 = ceil(x);
    size_t y1 = floor(y);
    size_t y2 = ceil(y);

    vec3 v00 = vertices[y1*dataWidth + x1].position;
    vec3 v01 = vertices[y2*dataWidth + x1].position;
    vec3 v10 = vertices[y1*dataWidth + x2].position;
    vec3 v11 = vertices[y2*dataWidth + x2].position;
    
    vec3 a = mix(v00, v10, x - floor(x));
    vec3 b = mix(v01, v11, x - floor(x));
    vec3 c = mix(a, b, y - floor(y));

    return c.y;
}

glm::vec3 ChunkedTerrain::GetLocationOfCoordinate(glm::dvec2 coordinate)
{
    double latitude  = (glm::pi<double>()/2.0) - coordinate.y;
    double longitude = coordinate.x;

    dvec3 p = dvec3(earthRadius * cos(longitude) * sin(latitude),
                    earthRadius * sin(longitude)  * sin(latitude),
                    earthRadius * cos(latitude));
    p = dvec3(-p.y, p.z, -p.x);

    return projector*(p-midpoint);
}

void ChunkedTerrain::Draw(const Frustum& cameraFrustum, const std::shared_ptr<Program>& program)
{
    glBindVertexArray(vertexArrayObject);

    size_t maxindex = (chunkGridWidth * chunkGridHeight * chunks[0].indices.size()) / max<size_t>(1, commandBuffers.size()/2);

    heightmap->Bind(GL_TEXTURE0);
    glUniform1i(program->GetUniform("heightmap"), 0);
    glUniform3f(program->GetUniform("nw"), nw.x, nw.y, nw.z);
    glUniform3f(program->GetUniform("ne"), ne.x, ne.y, ne.z);
    glUniform3f(program->GetUniform("sw"), sw.x, sw.y, sw.z);
    glUniform3f(program->GetUniform("se"), se.x, se.y, se.z);
    glUniform2i(program->GetUniform("dataSize"), dataWidth, dataHeight);

    std::vector<GLuint *> indexData;
    std::vector<GLvoid *> indexOffset; 
    std::vector<GLuint *> indexDataPtr;

    for(size_t i = 0; i < commandBuffers.size(); i++)
    {
        indexData.push_back(commandBuffers[i].GetBuffer());
        indexOffset.push_back(commandBuffers[i].GetBufferOffset());
        indexDataPtr.push_back(indexData[i]);
    }

    // Load chunks which are visible
    #pragma omp parallel for
    for(size_t i = 0; i < chunks.size(); i++)
    {
        Frustum::CullResult result = cameraFrustum.Compare(chunks[i].bounds);
        if(result == Frustum::INSIDE || result == Frustum::INTERSECT)
        {
            size_t t = omp_get_thread_num();
            memcpy((void *) indexDataPtr[t], (void *)chunks[i].indices.data(), chunks[i].indices.size()*sizeof(GLuint));
            indexDataPtr[t] += chunks[i].indices.size();
        }
    }

    // Draw results
    for(size_t i = 0; i < commandBuffers.size(); i++)
    {
        commandBuffers[i].FlushBuffer();
        commandBuffers[i].BindBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glDrawElements(GL_TRIANGLE_STRIP, (indexDataPtr[i] - indexData[i]), GL_UNSIGNED_INT, indexOffset[i]);
        std::cout << "thread " << i << " wrote " << (indexDataPtr[i] - indexData[i]) << " indices (max " << maxindex << std::endl;
    }
    std::cout << std::endl;
}
