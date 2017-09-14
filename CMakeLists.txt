cmake_minimum_required (VERSION 3.0)

project(myhtml)

# ---[ Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE MYHTML_HEADERS include/*.h)

# ---[ Sources
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/source)
file(GLOB_RECURSE MYHTML_SOURCES 
    source/mycore/*.c source/mycore/*.h
    source/myencoding/*.c source/myencoding/*.h
    source/myhtml/*.c source/myhtml/*.h
)

# ---[ Include port based on OS
if (WIN32)
    file(GLOB_RECURSE MYHTML_SOURCES_PORT source/myport/windows_nt/*.c source/myport/windows_nt/*.h)
else ()
    file(GLOB_RECURSE MYHTML_SOURCES_PORT source/myport/posix/*.c source/myport/posix/*.h)
endif (WIN32)

# export MYHTML_INCLUDE_DIRS and STXXLMYHTML_LIBRARIES to global CACHE
set(MYHTML_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE STRING "Include paths for Myhtml")
set(MYHTML_LIBRARIES myhtml CACHE STRING "Libraries to link for Myhtml")

# ---[ Create dynamic library
add_library(myhtml SHARED ${MYHTML_SOURCES} ${MYHTML_SOURCES_PORT} ${MYHTML_HEADERS})