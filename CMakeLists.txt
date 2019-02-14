cmake_minimum_required (VERSION 3.0)

project(myhtml)

################
## options
#########################
option(MyHTML_BUILD_WITHOUT_THREADS "Build without POSIX Threads" OFF)
option(MyCORE_BUILD_WITHOUT_THREADS "Build MyCORE without POSIX Threads" OFF)
option(MyHTML_BUILD_SHARED "Build shared library" ON)
option(MyHTML_BUILD_STATIC "Build static library" ON)
option(MyHTML_INSTALL_HEADER "Install header files" ON)

################
## dependencies
#########################
if(NOT MyHTML_BUILD_WITHOUT_THREADS)
    set(CMAKE_THREAD_PREFER_PTHREAD 1)
    find_package(Threads REQUIRED)
    if(NOT CMAKE_USE_PTHREADS_INIT)
        message(FATAL_ERROR "Could NOT find pthreads (missing: CMAKE_USE_PTHREADS_INIT)")
    endif()
endif()

################
## version and path
#########################
set(PROJECT_NAME "myhtml")
set(PROJECT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(PROJECT_DIR_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(PROJECT_DIR_HEADER "${PROJECT_SOURCE}")
set(PROJECT_DIR_API_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECT_SOURCE_PORT_DIR "${PROJECT_SOURCE}/myport")

set(PROJECT_LIB_NAME "${PROJECT_NAME}")
set(PROJECT_LIB_NAME_STATIC "${PROJECT_NAME}_static")

set(PROJECT_INSTALL_DLL_EXE_DIR "bin")

################
## Project Version
#########################
set(PROJECT_VERSION_HEADER_FILE "${PROJECT_DIR_API_HEADER}/myhtml/api.h")

file(STRINGS ${PROJECT_VERSION_HEADER_FILE} PROJECT_VERSION_PARTS
REGEX "^#define[ \t]+MyHTML_VERSION_(MAJOR|MINOR|PATCH)[ \t]+[0-9]+$")

list(GET PROJECT_VERSION_PARTS 0 PROJECT_VERSION_MAJOR_PART)
list(GET PROJECT_VERSION_PARTS 1 PROJECT_VERSION_MINOR_PART)
list(GET PROJECT_VERSION_PARTS 2 PROJECT_VERSION_PATCH_PART)

string(REGEX REPLACE "#define[ \t]+MyHTML_VERSION_MAJOR[ \t]+([0-9]+).*" "\\1" PROJECT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR_PART})
string(REGEX REPLACE "#define[ \t]+MyHTML_VERSION_MINOR[ \t]+([0-9]+).*" "\\1" PROJECT_VERSION_MINOR ${PROJECT_VERSION_MINOR_PART})
string(REGEX REPLACE "#define[ \t]+MyHTML_VERSION_PATCH[ \t]+([0-9]+).*" "\\1" PROJECT_VERSION_PATCH ${PROJECT_VERSION_PATCH_PART})

set(PROJECT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
message(STATUS "Project version: ${PROJECT_VERSION_STRING}")

################
## Macro
#########################
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

################
## Detect OS
#########################
set(PROJECT_PLATFORM "UNDEF")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(EXISTS "/etc/debian_version")
    set(PROJECT_PLATFORM "Debian")
  endif(EXISTS "/etc/debian_version")
  
  if(EXISTS "/etc/redhat-release")
    set(PROJECT_PLATFORM "Redhat")
  endif(EXISTS "/etc/redhat-release")
  
  if(EXISTS "/etc/fedora-release")
    set(PROJECT_PLATFORM "Redhat")
  endif(EXISTS "/etc/fedora-release")
  
  if(EXISTS "/etc/SuSE-release")
    set(PROJECT_PLATFORM "SuSe")
  endif(EXISTS "/etc/SuSE-release")
  
  if(EXISTS "/etc/gentoo-release")
    set(PROJECT_PLATFORM "Gentoo")
  endif(EXISTS "/etc/gentoo-release")
endif()

################
## lib param
#########################
set(PROJECT_LIB_NAME "${PROJECT_NAME}")
set(PROJECT_LIB_NAME_STATIC "${PROJECT_NAME}_static")

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
set(CMAKE_INSTALL_LIBDIR "lib")
endif()

if(NOT DEFINED PROJECT_OPTIMIZATION_LEVEL)
set(PROJECT_OPTIMIZATION_LEVEL "-O2")
endif()

################
## ARGS
#########################
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PROJECT_OPTIMIZATION_LEVEL}")

if (!UNIX AND WIN32)
  if(${CMAKE_CL_64})
    add_definitions(-D_WIN64)
  else()
    add_definitions(-D_WIN32)
  endif()
endif()

if(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
  add_definitions(/wd4100 /wd4255 /wd4820 /wd4668)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  
  message(STATUS "Windows set definitions")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall --std=c99 -fPIC")
  
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-D_POSIX_C_SOURCE=199309L)
  endif()
endif()

if(MyCORE_BUILD_WITHOUT_THREADS)
  message(STATUS "Build without POSIX Threads")
  add_definitions(-DMyCORE_BUILD_WITHOUT_THREADS)
else()
  message(STATUS "Build with POSIX Threads")
endif()

################
## Includes
#########################
include_directories(${PROJECT_DIR_API_HEADER})
file(GLOB_RECURSE PROJECT_HEADERS ${PROJECT_DIR_API_HEADER}/*.h)

################
## Sources
#########################
GET_MODULES_LIST(MODULES ${PROJECT_SOURCE})
CREATE_MODULES_SOURCE_PATH(MODULES_PATH ${PROJECT_SOURCE} "${MODULES}")

file(GLOB_RECURSE PROJECT_SOURCES ${MODULES_PATH})

################
## Include port based on OS
#########################
if (WIN32)
    file(GLOB_RECURSE PROJECT_SOURCES_PORT ${PROJECT_SOURCE_PORT_DIR}/windows_nt/*.c)
else ()
    file(GLOB_RECURSE PROJECT_SOURCES_PORT ${PROJECT_SOURCE_PORT_DIR}/posix/*.c)
endif (WIN32)

################
## export MYHTML_INCLUDE_DIRS and STXXLMYHTML_LIBRARIES to global CACHE
#########################
set(MYHTML_INCLUDE_DIRS ${PROJECT_DIR_API_HEADER} CACHE STRING "Include paths for Myhtml")
set(MYHTML_LIBRARIES myhtml CACHE STRING "Libraries to link for Myhtml")

################
## Create static/dynamic library
#########################
if(MyHTML_BUILD_SHARED)
    add_library(${PROJECT_LIB_NAME} SHARED ${PROJECT_SOURCES} ${PROJECT_SOURCES_PORT})
    target_link_libraries(${PROJECT_LIB_NAME} ${CMAKE_THREAD_LIBS_INIT})
    set_target_properties(${PROJECT_LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_LIB_NAME})
    set_target_properties(${PROJECT_LIB_NAME} PROPERTIES VERSION ${PROJECT_VERSION_STRING} SOVERSION ${PROJECT_VERSION_MAJOR})
endif()

if(MyHTML_BUILD_STATIC)
    add_library(${PROJECT_LIB_NAME_STATIC} STATIC ${PROJECT_SOURCES} ${PROJECT_SOURCES_PORT})
    target_link_libraries(${PROJECT_LIB_NAME_STATIC} ${CMAKE_THREAD_LIBS_INIT})
    set_target_properties(${PROJECT_LIB_NAME_STATIC} PROPERTIES OUTPUT_NAME ${PROJECT_LIB_NAME_STATIC})
    set_target_properties(${PROJECT_LIB_NAME_STATIC} PROPERTIES VERSION ${PROJECT_VERSION_STRING} SOVERSION ${PROJECT_VERSION_MAJOR})
endif()

################
## install
#########################
if(MyHTML_INSTALL_HEADER)
    file(GLOB PROJECT_HEADER_DIRS "${PROJECT_DIR_API_HEADER}/*")
    foreach(ITEM ${PROJECT_HEADER_DIRS})                                                                                                                                                
        if(IS_DIRECTORY "${ITEM}")
            install(DIRECTORY "${ITEM}" DESTINATION "include" FILES_MATCHING PATTERN "*.h")
        endif()
    endforeach()
endif()

if(MyHTML_BUILD_SHARED)
    install(TARGETS ${PROJECT_LIB_NAME}
    RUNTIME DESTINATION "${PROJECT_INSTALL_DLL_EXE_DIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

if(MyHTML_BUILD_STATIC)
    install(TARGETS ${PROJECT_LIB_NAME_STATIC}
    RUNTIME DESTINATION "${PROJECT_INSTALL_DLL_EXE_DIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

################
## Build an RPM.
#########################
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION_STRING})

if(PROJECT_PLATFORM STREQUAL "Debian")
  set(CPACK_GENERATOR "DEB")
elseif(PROJECT_PLATFORM STREQUAL "Redhat")
  set(CPACK_GENERATOR "RPM")
elseif(PROJECT_PLATFORM STREQUAL "SuSe")
  set(CPACK_GENERATOR "RPM")
else()
  set(CPACK_GENERATOR "TGZ")
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fast C/C++ HTML 5 Parser. Using threads.")
set(CPACK_PACKAGE_CONTACT "Alexander Borisov")
set(CPACK_PACKAGE_VENDOR "Alexander Borisov")

if(32BIT)
  set(CPACK_RPM_PACKAGE_ARCHITECTURE i686)
else()
  set(CPACK_RPM_PACKAGE_ARCHITECTURE x86_64)
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CPACK_RPM_PACKAGE_ARCHITECTURE}")

include(CPack)
