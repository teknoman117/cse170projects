cmake_minimum_required (VERSION 2.8)
project (Project2)

file(GLOB Sources
    "src/*.cpp"
    "src/objects/*.cpp"
    
    "include/project2/*.hpp"
    "include/project2/objects/*.hpp"
)

# Add universal includes
include_directories("include")

# Are we compiling on Window?
IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Project2 Executable
    add_executable(Project2 ${Sources})

    # Find system OpenGL
    find_package(OpenGL)
    target_link_libraries(Project2 ${OPENGL_LIBRARIES})
    include_directories(${OPENGL_INCLUDE_DIRS})

    get_filename_component(DEPS "dependencies/windows/libs/" ABSOLUTE)

    # Pull in the bundled libraries
    include_directories(${PROJECT_SOURCE_DIR}/dependencies/common)
    include_directories(${PROJECT_SOURCE_DIR}/dependencies/common/glew)
    target_link_libraries(Project2 ${DEPS}/glew32.lib)
    target_link_libraries(Project2 ${DEPS}/glew32mx.lib)
    target_link_libraries(Project2 ${DEPS}/SDL2.lib)
    target_link_libraries(Project2 ${DEPS}/SDL2main.lib)
    target_link_libraries(Project2 ${DEPS}/SDL2test.lib)
    
    #enable Multiprocessor build
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" )
    
    #copy game and dlls
    file(COPY ${PROJECT_SOURCE_DIR}/content DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
    file(COPY ${PROJECT_SOURCE_DIR}/dependencies/windows/dlls/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
    file(COPY ${PROJECT_SOURCE_DIR}/dependencies/windows/dlls/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)    
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# Are we compiling on Mac OS X?
IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Make sure Xcode uses Clang set for libc++ and c++ version 2011
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "Deployment target for OSX" FORCE)

    # Needed for performing Makefile build on OS X, as above parameters are Xcode specific
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -stdlib=libc++ -g -Wall")

    # Setup the names for things
    set( EXE_NAME "Project2")
    set( MACOSX_BUNDLE_BUNDLE_VERSION 1 )
    set( MACOSX_BUNDLE_GUI_IDENTIFIER "net.teknoman117.cse170.project2" )

    # Find libraries
    find_library(OPENGL_FRAMEWORK     NAMES OpenGL     PATHS ${PROJECT_SOURCE_DIR}/dependencies/osx/frameworks)
    find_library(SDL2_FRAMEWORK       NAMES SDL2       PATHS ${PROJECT_SOURCE_DIR}/dependencies/osx/frameworks)

    # Copy the resources into the app bundle
    file(GLOB MACOSX_RESOURCES ${PROJECT_SOURCE_DIR}/content )
    set_source_files_properties(${MACOSX_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    # Copy the frameworks into the app bundle
    file(GLOB MACOSX_FRAMEWORKS ${PROJECT_SOURCE_DIR}/dependencies/osx/frameworks/* )
    set(MACOSX_FRAMEWORKS ${SDL2_FRAMEWORK})
    set_source_files_properties(${MACOSX_FRAMEWORKS} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)

    add_executable(Project2 MACOSX_BUNDLE ${Sources} src/apple-compat.mm ${MACOSX_RESOURCES} ${MACOSX_FRAMEWORKS})

    set_target_properties(Project2 PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path @loader_path/../Frameworks")
    set_target_properties(Project2 PROPERTIES XCODE_ATTRIBUTE_COPY_PHASE_STRIP "No")

    # Link the frameworks to destructo
    target_link_libraries(Project2 ${SDL2_FRAMEWORK})
    target_link_libraries(Project2 ${OPENGL_FRAMEWORK})

    # Pull in the bundled GLEW library
    include_directories("dependencies/common/glew")
    get_filename_component(CGLEW "dependencies/osx/libs/" ABSOLUTE)
    target_link_libraries(Project2 ${CGLEW}/libGLEW.a)
    target_link_libraries(Project2 ${CGLEW}/libGLEWmx.a)

    # Pull in the OpenGL Mathematics Library
    include_directories("dependencies/common/glm")

ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Are we compiling on Linux?
IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    # Linux is very good with package management and users should understand about the
    # highly organized system they run on.  Either way, depend on the libraries
    # being installed on the system.  Arch Linux has everything in the repos or AUR

    # Required Libraries
    #
    # OpenGL Runtime             (Mesa-libGL1) - http://www.mesa3d.org/
    # OpenGL Extension Wrangler  (GLEW)        - http://glew.sourceforge.net/
    # OpenGL Mathematics Library (GLM)         - http://glm.g-truc.net/
    #
    # Simple Direct Media Layer (requires 2.0.0 or greater) - http://libsdl.org/download-2.0.php
    #
    # A graphics card and driver supporting at least OpenGL 2.0, recommended 3.3+
    #
    # Graphics cards known to work
    #     Nvidia GeForce GTX 980 (4.5.0 NVIDIA 352.63)
    #

    # Project2 Executable
    add_executable(Project2 ${Sources})

    # Discover include path and library for the SDL2 library
    find_path(SDL_INCLUDE_DIR NAMES SDL2/SDL.h)
    include_directories(${SDL_INCLUDE_DIR})
    find_library(SDL_LIBRARY NAMES SDL2)
    target_link_libraries(Project2 ${SDL_LIBRARY})

    # Discover the include path and library for the system OpenGL Library
    find_path(OPENGL_INCLUDE_DIR NAMES GL/gl.h)
    include_directories(${OPENGL_INCLUDE_DIR})
    find_library(OPENGL_LIBRARY NAMES GL)
    target_link_libraries(Project2 ${OPENGL_LIBRARY})

    # Discover the include path and library for the GLU Library
    find_path(GLU_INCLUDE_DIR NAMES GL/glu.h)
    include_directories(${GLU_INCLUDE_DIR})
    find_library(GLU_LIBRARY NAMES GLU)
    target_link_libraries(Project2 ${GLU_LIBRARY})

    # Discover the include path and library for the OpenGL Extension Wrangler
    find_path(GLEW_INCLUDE_DIR NAMES GL/glew.h)
    include_directories(${GLEW_INCLUDE_DIR})
    find_library(GLEW_LIBRARY NAMES GLEW)
    target_link_libraries(Project2 ${GLEW_LIBRARY})

    # Discover the include path for the OpenGL Mathematics Library
    #find_path(GLM_INCLUDE_DIR NAMES glm/glm.hpp)
    #include_directories(${GLM_INCLUDE_DIR})
    # Pull in the OpenGL Mathematics Library
    include_directories("dependencies/common")

    # Make a define, only used with Linux.  Also, make sure we use c++11
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__LINUX__ -fopenmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -Wall -fopenmp")

ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
