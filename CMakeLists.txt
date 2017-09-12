cmake_minimum_required (VERSION 3.0)

project(myhtml)

# ---[ Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE MYHTML_HEADERS include/*.h)

# ---[ Sources
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/source)
file(GLOB_RECURSE MYHTML_SOURCES source/mycore/*.c source/mycore/*.h)
file(GLOB_RECURSE MYHTML_SOURCES source/myencoding/*.c source/myencoding/*.h)
file(GLOB_RECURSE MYHTML_SOURCES source/myhtml/*.c source/myhtml/*.h)

# ---[ Include port based on OS
if (WIN32)
    file(GLOB_RECURSE MYHTML_SOURCES source/myport/windows_nt/*.c source/myport/windows_nt/*.h)
else ()
    file(GLOB_RECURSE MYHTML_SOURCES source/myport/posix/*.c source/myport/posix/*.h)
endif (WIN32)

# ---[ Create dynamic library
add_library(myhtml SHARED ${MYHTML_SOURCES} ${MYHTML_HEADERS})
add_library(myhtml_static STATIC ${MYHTML_SOURCES} ${MYHTML_HEADERS})