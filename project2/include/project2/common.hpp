#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <iostream>
#include <memory>
#include <map>
#include <cinttypes>
#include <chrono>
#include <string>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#ifndef ssize_t
#define ssize_t int64_t
#endif

#endif
