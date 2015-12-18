#include <project2/common.hpp>
#include <vector>

size_t DecodePolyline(std::vector<glm::dvec2>& coordinates, const std::string& polyline);
size_t DecodePolylineFromFile(std::vector<glm::dvec2>& coordinates, const std::string& path);
