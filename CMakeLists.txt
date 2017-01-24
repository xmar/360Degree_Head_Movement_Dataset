cmake_minimum_required(VERSION 3.7.0)
project(OSVRClientTest)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(LibAvWrapper)

find_package( Boost 1.63 REQUIRED program_options )
find_package(osvr REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(osvrRenderManager REQUIRED)
find_package(ZMQ REQUIRED)
osvrrm_find_SDL2()
osvrrm_find_GLEW()

find_package(Eigen3 REQUIRED)
MESSAGE( STATUS "BOOST_LIBRARIES:         " ${BOOST_LIBRARIES} )
MESSAGE( STATUS "Boost_LIBRARIES:         " ${Boost_LIBRARIES} )
MESSAGE( STATUS "Boost_LIBRARY_DIR:         " ${Boost_LIBRARY_DIR} )
MESSAGE( STATUS "Boost_INCLUDE_DIR:         " ${Boost_INCLUDE_DIR} )


LINK_DIRECTORIES(${OSVR_LIBRARY_DIRS} ${EIGEN3_LIBRARY_DIRS} ${Boost_LIBRARY_DIR})

FILE(GLOB OSVRClientTrackerSrc src/*.cpp)

add_executable(OSVRClientTest
  ${OSVRClientTrackerSrc}
)
target_include_directories(OSVRClientTest PUBLIC
    LibAvWrapper/inc
    ${Boost_INCLUDE_DIR}
    ${OSVR_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SOIL_INCLUDE_DIRS}
    ${ZMQ_INCLUDE_DIRS}
)
target_link_libraries(OSVRClientTest
    osvr::osvrClientKitCpp
    osvrRenderManager::osvrRenderManagerCpp
    LibAvWrapper
    /usr/lib/libboost_program_options.so
    ${Boost_LIBRARIES}
    ${EIGEN3_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${SOIL_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${OSVR_LIBRARIES}
    ${ZMQ_LIBRARIES}
  )
