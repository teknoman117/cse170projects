#include <project2/chunkedterrain.hpp>

#include <cmath>
#include <fstream>
#include <cinttypes>
#include <iomanip>
#include <limits>

using namespace glm;

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
ChunkedTerrain::ChunkedTerrain(const std::string& path, size_t width, size_t height, dvec2 resolution, dvec2 corner)
    : GLObject(1)
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

    size_t expectedSize = sizeof(float) * width * height;
    if(expectedSize != actualSize)
    {
        std::cerr << "[FATAL] [TERRAIN \"" << path << "\"] Loaded resource is not of dimension: " << width << " " << height << std::endl;
        throw std::runtime_error("Resource does not conform to expectations");
    }

    // Read all the data from the terrain heightmap
    std::vector<float> elevation;
    elevation.resize(actualSize/4);
    inputFile.read((char *) elevation.data(), actualSize);
    inputFile.close();

    // Compute the chunk grid properties
    chunkGridWidth  = (width-1) / 16;
    chunkGridHeight = (height-1) / 16;
    elementsPerRow  = width*sizeof(float);
    chunks.resize(chunkGridWidth*chunkGridHeight);

    // Compute the corners of the area this represents on Earth (remap onto left handed coordinates)
    double latitude  = (glm::pi<double>()/2.0) - corner.y;
    double longitude = corner.x;

    nw = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    nw = dvec3(-nw.y, nw.z, -nw.x);


    latitude =  (glm::pi<double>()/2.0) - corner.y;
    longitude = corner.x + double(16*chunkGridWidth + 1)*resolution.x;

    ne = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    ne = dvec3(-ne.y, ne.z, -ne.x);


    latitude  = ((glm::pi<double>()/2.0) - corner.y) + (double(16*chunkGridHeight + 1)*resolution.y);
    longitude = corner.x;

    sw = dvec3(earthRadius * cos(longitude) * sin(latitude),
               earthRadius * sin(longitude)  * sin(latitude),
               earthRadius * cos(latitude));
    sw = dvec3(-sw.y, sw.z, -sw.x);


    latitude  = ((glm::pi<double>()/2.0) - corner.y) + (double(16*chunkGridHeight + 1)*resolution.y);
    longitude = corner.x + double(16*chunkGridWidth + 1)*resolution.x;

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

    // Allocate storage for the opengl resources
    for(size_t i = 0; i < chunkGridWidth; i++)
    {

    }
}

ChunkedTerrain::~ChunkedTerrain()
{

}

void ChunkedTerrain::Draw(const std::shared_ptr<Program>& program)
{

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
    double x = (wDist / (wDist + eDist)) * (16*chunkGridWidth + 1);
    double y = (nDist / (nDist + sDist)) * (16*chunkGridHeight + 1);

    size_t x1 = floor(x);
    size_t x2 = ceil(x);
    size_t y1 = floor(y);
    size_t y2 = ceil(y);

    return 0.0f;
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
