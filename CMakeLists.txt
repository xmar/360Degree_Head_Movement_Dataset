cmake_minimum_required(VERSION 2.8.12)
project(OSVRClientTracker)

set (CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(osvr REQUIRED)

find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})
LINK_DIRECTORIES(${Eigen3_LIBRARY_DIRS})

MESSAGE( STATUS "Eigen3 SDIR " ${Eigen_INCLUDE_DIRS})
MESSAGE( STATUS "Eigen3 SDIR " ${Eigen_LIBRARY_DIRS})

FILE(GLOB OSVRClientTrackerSrc src/*.cpp src/*.cpp)

add_executable(OSVRClientTracker
  ${OSVRClientTrackerSrc}
)
target_link_libraries(OSVRClientTracker osvr::osvrClientKitCpp ${Eigen3_LIBRARIES})
