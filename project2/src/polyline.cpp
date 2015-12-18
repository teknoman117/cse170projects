#include <project2/polyline.hpp>
#include <fstream>

using namespace glm;

size_t DecodePolyline(std::vector<dvec2>& coordinates, const std::string& polyline)
{
    const char *bytes = polyline.c_str();
    size_t length = polyline.size();
    size_t idx = 0;
    coordinates.resize(0);

    dvec2 position = dvec2(0,0);
    while (idx < length) 
    {
        char byte = 0;
        int res = 0;
        char shift = 0;

        do 
        {
            byte = bytes[idx++] - 63;
            res |= (byte & 0x1F) << shift;
            shift += 5;
        } while (byte >= 0x20);

        float deltaLat = ((res & 1) ? ~(res >> 1) : (res >> 1));
        position.y += deltaLat;

        shift = 0;
        res = 0;

        do 
        {
            byte = bytes[idx++] - 0x3F;
            res |= (byte & 0x1F) << shift;
            shift += 5;
        } while (byte >= 0x20);

        float deltaLon = ((res & 1) ? ~(res >> 1) : (res >> 1));
        position.x += deltaLon;

        coordinates.push_back(position * 1E-5);
    }

    return coordinates.size();
}

size_t DecodePolylineFromFile(std::vector<glm::dvec2>& coordinates, const std::string& path)
{
    // Read the polyline string
    std::ifstream polylineFile(path);
    if(!polylineFile.is_open())
    {
        std::cerr << "[FATAL] [POLYLINE \"" << path << "\"] Unable to open specified path" << std::endl;
        throw std::runtime_error("Unable to open specified path");
    }
    
    polylineFile.seekg ( 0, std::ios::end );
    size_t polylineSize = polylineFile.tellg();
    polylineFile.seekg ( 0, std::ios::beg );
    
    std::string polyline;
    polyline.resize(polylineSize);
    polylineFile.read ( &polyline[0], polylineSize );
    polylineFile.close();

    return DecodePolyline(coordinates, polyline);
}

