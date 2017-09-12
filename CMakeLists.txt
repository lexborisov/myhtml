cmake_minimum_required (VERSION 3.0)

project(myhtml)

# ---[ Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE MYHTML_HEADERS include/*.h)

# ---[ Sources
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/source)
file(GLOB_RECURSE MYHTML_SOURCES "*.cpp" "*.hpp")

add_library(myhtml SHARED ${MYHTML_SOURCES} ${MYHTML_HEADERS})

