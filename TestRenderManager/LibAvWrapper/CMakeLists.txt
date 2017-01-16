set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

LIST(APPEND CMAKE_MODULE_PATH "." "..")

find_package( FFMPEG REQUIRED )
MESSAGE( STATUS "FFMPEG found:         " ${FFMPEG_FOUND} )

find_package(GLEW)
find_package(OpenGL)
# if (${OpenCV_FOUND} AND ${GLEW_FOUND})
#   message(STATUS "Set USE_OPENGl")
#   add_definitions(-DUSE_OPENGl)
# endif()


FILE(GLOB LibAvWrapperSrc src/*.cpp)

add_library(LibAvWrapper ${LibAvWrapperSrc})
target_include_directories(LibAvWrapper PUBLIC inc ${FFMPEG_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
#target_compile_features(LibAvWrapper PRIVATE cxx_range_for cxx_nullptr cxx_auto_type)
target_link_libraries( LibAvWrapper -pthread  ${FFMPEG_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
