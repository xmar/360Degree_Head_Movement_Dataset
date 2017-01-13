cmake_minimum_required(VERSION 2.8.12)
project(OSVRClientTest)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(LibAvWrapper)

find_package(osvr REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(osvrRenderManager REQUIRED)
osvrrm_find_SDL2()
osvrrm_find_GLEW()

find_package(Eigen3 REQUIRED)

LINK_DIRECTORIES(${OSVR_LIBRARY_DIRS} ${EIGEN3_LIBRARY_DIRS})

FILE(GLOB OSVRClientTrackerSrc src/*.cpp)

add_executable(OSVRClientTest
  ${OSVRClientTrackerSrc}
)
target_include_directories(OSVRClientTest PUBLIC
    LibAvWrapper/inc
    ${OSVR_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SOIL_INCLUDE_DIRS}
)
target_link_libraries(OSVRClientTest
    osvr::osvrClientKitCpp
    osvrRenderManager::osvrRenderManagerCpp
    LibAvWrapper
    ${EIGEN3_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${SOIL_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${OSVR_LIBRARIES}
  )
