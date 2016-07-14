cmake_minimum_required( VERSION 2.8.8 )

################
## version and path
#########################
set(MyHTML_PROJECT_NAME "myhtml")
set(MyHTML_PROJECT_DIR_ROOT "..")
set(MyHTML_PROJECT_DIR_LIB "${MyHTML_PROJECT_DIR_ROOT}/lib")
set(MyHTML_PROJECT_API_HEADER "${MyHTML_PROJECT_DIR_ROOT}/include/myhtml")
set(MyHTML_PROJECT_SOURCE "${MyHTML_PROJECT_DIR_ROOT}/source")

set(MyHTML_VERSION_MAJOR 1)
set(MyHTML_VERSION_MINOR 0)
set(MyHTML_VERSION_PATCH 2)
set(MyHTML_VERSION_STRING "${MyHTML_VERSION_MAJOR}.${MyHTML_VERSION_MINOR}.${MyHTML_VERSION_PATCH}")

project(${MyHTML_PROJECT_NAME})

################
## Detect OS
#########################
set(MyHTML_PLATFORM "UNDEF")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(EXISTS "/etc/debian_version")
    set(MyHTML_PLATFORM "Debian")
  endif(EXISTS "/etc/debian_version")
  
  if(EXISTS "/etc/redhat-release")
    set(MyHTML_PLATFORM "Redhat")
  endif(EXISTS "/etc/redhat-release")
  
  if(EXISTS "/etc/fedora-release")
    set(MyHTML_PLATFORM "Redhat")
  endif(EXISTS "/etc/fedora-release")
  
  if(EXISTS "/etc/SuSE-release")
    set(MyHTML_PLATFORM "SuSe")
  endif(EXISTS "/etc/SuSE-release")
  
  if(EXISTS "/etc/gentoo-release")
    set(MyHTML_PLATFORM "Gentoo")
  endif(EXISTS "/etc/gentoo-release")
endif()

################
## lib param
#########################
if(POLICY CMP0042)
  cmake_policy(SET CMP0042 OLD)
endif()

set(MyHTML_PROJECT_LIB_NAME "${MyHTML_PROJECT_NAME}")
set(MyHTML_PROJECT_LIB_NAME_STATIC "${MyHTML_PROJECT_NAME}_static")

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()

if(NOT DEFINED MyHTML_OPTIMIZATION_LEVEL)
  set(MyHTML_OPTIMIZATION_LEVEL "-O2")
endif()

if(NOT DEFINED MyHTML_BUILD_WITHOUT_THREADS)
    set(MyHTML_BUILD_WITHOUT_THREADS "NO")
endif()

################
## extern mem functions
#########################
if(DEFINED MyHTML_EXTERN_MALLOC)
    add_definitions(-DMyHTML_EXTERN_MALLOC=${MyHTML_EXTERN_MALLOC})
endif()

if(DEFINED MyHTML_EXTERN_REALLOC)
    add_definitions(-DMyHTML_EXTERN_REALLOC=${MyHTML_EXTERN_REALLOC})
endif()

if(DEFINED MyHTML_EXTERN_CALLOC)
    add_definitions(-DMyHTML_EXTERN_CALLOC=${MyHTML_EXTERN_CALLOC})
endif()

if(DEFINED MyHTML_EXTERN_FREE)
    add_definitions(-DMyHTML_EXTERN_FREE=${MyHTML_EXTERN_FREE})
endif()

################
## ARGS
#########################
set(INSTALL_DLL_EXE_DIR "bin")

option(MyHTML_BUILD_SHARED "Build shared library" ON)
option(MyHTML_BUILD_STATIC "Build static library" ON)
option(MyHTML_INSTALL_HEADER "Install header files" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MyHTML_OPTIMIZATION_LEVEL}")

include_directories(${MyHTML_PROJECT_SOURCE})
FILE(GLOB ALL_SOURCE_LIST "${MyHTML_PROJECT_SOURCE}/myhtml/*.c" "${MyHTML_PROJECT_SOURCE}/myhtml/utils/*.c")

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
  
  if(MyHTML_BUILD_WITHOUT_THREADS STREQUAL "NO")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
  endif()
endif()

if(MyHTML_BUILD_WITHOUT_THREADS STREQUAL "YES")
  message(STATUS "Build without POSIX Threads")
  add_definitions(-DMyHTML_BUILD_WITHOUT_THREADS)
else()
  message(STATUS "Build with POSIX Threads")
endif()

################
## build
#########################
if(MyHTML_BUILD_SHARED)
  add_library(${MyHTML_PROJECT_NAME} SHARED ${ALL_SOURCE_LIST})
  set_target_properties(${MyHTML_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MyHTML_PROJECT_NAME})
  set_target_properties(${MyHTML_PROJECT_NAME} PROPERTIES VERSION ${MyHTML_VERSION_STRING} SOVERSION ${MyHTML_VERSION_MAJOR})
endif()

if(MyHTML_BUILD_STATIC)
  add_library(${MyHTML_PROJECT_LIB_NAME_STATIC} STATIC ${ALL_SOURCE_LIST})
  set_target_properties(${MyHTML_PROJECT_LIB_NAME_STATIC} PROPERTIES OUTPUT_NAME ${MyHTML_PROJECT_LIB_NAME_STATIC})
  set_target_properties(${MyHTML_PROJECT_LIB_NAME_STATIC} PROPERTIES VERSION ${MyHTML_VERSION_STRING} SOVERSION ${MyHTML_VERSION_MAJOR})
endif()

################
## install
#########################
if(MyHTML_INSTALL_HEADER)
  install(DIRECTORY "${MyHTML_PROJECT_API_HEADER}" DESTINATION "include" FILES_MATCHING PATTERN "*.h")
endif()

if(MyHTML_BUILD_SHARED)
  install(TARGETS ${MyHTML_PROJECT_LIB_NAME}
  RUNTIME DESTINATION "${INSTALL_DLL_EXE_DIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

if(MyHTML_BUILD_STATIC)
  install(TARGETS ${MyHTML_PROJECT_LIB_NAME_STATIC}
  RUNTIME DESTINATION "${INSTALL_DLL_EXE_DIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

################
## Build an RPM.
#########################
set(CPACK_PACKAGE_VERSION ${MyHTML_VERSION_STRING})

if(MyHTML_PLATFORM STREQUAL "Debian")
  set(CPACK_GENERATOR "DEB")
elseif(MyHTML_PLATFORM STREQUAL "Redhat")
  set(CPACK_GENERATOR "RPM")
elseif(MyHTML_PLATFORM STREQUAL "SuSe")
  set(CPACK_GENERATOR "RPM")
else()
  set(CPACK_GENERATOR "TGZ")
endif()

set(CPACK_PACKAGE_NAME "myhtml")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fast C/C++ HTML 5 Parser. Using threads.")
set(CPACK_PACKAGE_CONTACT "Alexander Borisov")
set(CPACK_PACKAGE_VENDOR "Alexander Borisov")

if(32BIT)
  set(CPACK_RPM_PACKAGE_ARCHITECTURE i686)
else()
  set(CPACK_RPM_PACKAGE_ARCHITECTURE x86_64)
endif()

if(MyHTML_BUILD_WITHOUT_THREADS STREQUAL "YES")
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}-without-threads.${CPACK_RPM_PACKAGE_ARCHITECTURE}")
else()
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CPACK_RPM_PACKAGE_ARCHITECTURE}")
endif()

include(CPack)
