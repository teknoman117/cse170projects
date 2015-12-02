cmake_minimum_required (VERSION 2.8)
project (Project2)

file(GLOB Sources
	"src/*.cpp"
    "include/project2/*.hpp"
)

# Add universal includes
include_directories("include")

# Are we compiling on Window?
#IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	# Project2 Executable
#	add_executable(Project2 ${Sources} "icon/Project2Icon.rc" )

    # Windows should consume headers bundles with source
#    include_directories("includes/SDL")
#    include_directories("includes/glew")
#    include_directories("includes/glm-0.9.4.4")

    # Windows Libraries
#    target_link_libraries(Project2 "../libs/windows/glew/glew32")
#    target_link_libraries(Project2 "../libs/windows/SDL/SDL2")
#    target_link_libraries(Project2 "opengl32.lib")

	#enable Multiprocessor build
#	SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" )
	
	#copy game and dlls
#	file(COPY ${PROJECT_SOURCE_DIR}/game/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
#	file(COPY ${PROJECT_SOURCE_DIR}/libs/windows/_alldlls/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
#	file(COPY ${PROJECT_SOURCE_DIR}/libs/windows/_alldlls/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)

	
#ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# Are we compiling on Mac OS X?
IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Make sure Xcode uses Clang set for libc++ and c++ version 2011
	#set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

	# Needed for performing Makefile build on OS X, as above parameters are Xcode specific
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -stdlib=libc++ -g -Wall")

	# Setup the names for things
    set( EXE_NAME "Project2")
    set( MACOSX_BUNDLE_BUNDLE_VERSION 1 )
    #set( MACOSX_BUNDLE_ICON_FILE "destructo.icns" )
    set( MACOSX_BUNDLE_GUI_IDENTIFIER "net.teknoman117.cse170.project2" )

    # Find libraries
	find_library(OPENGL_FRAMEWORK     NAMES OpenGL     PATHS ${PROJECT_SOURCE_DIR}/dependencies/osx/frameworks)
	find_library(SDL2_FRAMEWORK       NAMES SDL2       PATHS ${PROJECT_SOURCE_DIR}/dependencies/osx/frameworks)

	# Copy the resources into the app bundle
	file(GLOB MACOSX_RESOURCES ${PROJECT_SOURCE_DIR}/content/* )
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

    # Discover the include path and library for the OpenGL Extension Wrangler
    find_path(GLEW_INCLUDE_DIR NAMES GL/glew.h)
    include_directories(${GLEW_INCLUDE_DIR})
    find_library(GLEW_LIBRARY NAMES GLEW)
    target_link_libraries(Project2 ${GLEW_LIBRARY})

    # Discover the include path for the OpenGL Mathematics Library
    find_path(GLM_INCLUDE_DIR NAMES glm/glm.hpp)
    include_directories(${GLM_INCLUDE_DIR})

    # Make a define, only used with Linux.  Also, make sure we use c++11
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__LINUX__")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -Wall")

ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
