cmake_minimum_required(VERSION 2.8.12)
project(OSVRClientTracker)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(osvr REQUIRED)

find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})
LINK_DIRECTORIES(${EIGEN3_LIBRARY_DIRS})

FILE(GLOB OSVRClientTrackerSrc src/*.cpp src/*.cpp)

add_executable(OSVRClientTracker
  ${OSVRClientTrackerSrc}
)
target_link_libraries(OSVRClientTracker osvr::osvrClientKitCpp ${EIGEN3_LIBRARIES})
