cmake_minimum_required (VERSION 3.0)

project(myhtml)

# ---[ Macro
MACRO(GET_MODULES_LIST result curdir)
FILE(GLOB children ${curdir}/ ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        string(REGEX MATCH "\\.[^/]+$" MATCHSTR ${child})
        IF(IS_DIRECTORY ${child} AND MATCHSTR STREQUAL "")
            string(REGEX MATCH "[^/]+$" MATCHSTR ${child})
            IF(NOT MATCHSTR STREQUAL "myport")
                LIST(APPEND dirlist ${MATCHSTR})
            ENDIF()
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

MACRO(CREATE_MODULES_SOURCE_PATH result dirpath modules)
    SET(dirlist "")
    FOREACH(module ${modules})
        message(STATUS ${module})
        LIST(APPEND dirlist ${dirpath}/${module}/*.c)
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

# ---[ Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE MYHTML_HEADERS include/*.h)

# ---[ Sources
GET_MODULES_LIST(MODULES ${CMAKE_CURRENT_SOURCE_DIR}/source)
CREATE_MODULES_SOURCE_PATH(MODULES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/source "${MODULES}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/source)
file(GLOB_RECURSE MYHTML_SOURCES ${MODULES_PATH})

# ---[ Include port based on OS
if (WIN32)
    file(GLOB_RECURSE MYHTML_SOURCES_PORT ../source/myport/windows_nt/*.c)
else ()
    file(GLOB_RECURSE MYHTML_SOURCES_PORT ../source/myport/posix/*.c)
endif (WIN32)

# export MYHTML_INCLUDE_DIRS and STXXLMYHTML_LIBRARIES to global CACHE
set(MYHTML_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE STRING "Include paths for Myhtml")
set(MYHTML_LIBRARIES myhtml CACHE STRING "Libraries to link for Myhtml")

# ---[ Create dynamic library
add_library(myhtml SHARED ${MYHTML_SOURCES} ${MYHTML_SOURCES_PORT} ${MYHTML_HEADERS})