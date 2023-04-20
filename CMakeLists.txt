if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
endif(APPLE)

project(OpenTeraPlus)

#include(CTest)
#enable_testing()
#include(CheckFunctionExists)

#Look for minimum cmake version
cmake_minimum_required(VERSION 3.21)

#DEFINITIONS POLICY, NEW SINCE 3.0.2
cmake_policy(SET CMP0043 NEW)

# Software version
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "4")
SET(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
add_definitions(-DOPENTERAPLUS_VERSION="${CPACK_PACKAGE_VERSION}" )
add_definitions(-DOPENTERAPLUS_VERSION_MAJOR="${CPACK_PACKAGE_VERSION_MAJOR}" )
add_definitions(-DOPENTERAPLUS_VERSION_MINOR="${CPACK_PACKAGE_VERSION_MINOR}" )
add_definitions(-DOPENTERAPLUS_VERSION_PATCH="${CPACK_PACKAGE_VERSION_PATCH}" )
add_definitions(-DOPENTERAPLUS_CLIENT_NAME="${CMAKE_PROJECT_NAME}")

# Compile in debug or release mode
if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
  #set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME openteraplus)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/deploy)

if (APPLE)
    set(CMAKE_INSTALL_BINDIR OpenTeraPlus.app/Contents/MacOS)
    set(CMAKE_INSTALL_LIBDIR OpenTeraPlus.app/Contents/Frameworks)
    set(CMAKE_INSTALL_INCLUDEDIR OpenTeraPlus.app/Contents/Helpers)
endif(APPLE)

if (UNIX AND NOT APPLE)
    # Fix QWebEngine linking issues on Linux (https://bugreports.qt.io/browse/QTBUG-80964)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
endif(UNIX AND NOT APPLE)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

message(STATUS "Using QMAKE: ${QT_QMAKE_EXECUTABLE}")

# This will enable to output everything in the bin directory for testing and debugging...
# SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# SET (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# External dependencies first
#add_subdirectory(external)

# Shared data
add_subdirectory(shared)

# Client
#add_subdirectory(client)

# Tests
#add_subdirectory(tests)

# Readme and other files
add_custom_target(readme SOURCES README.md LICENSE.TXT)

# Install LICENSE / README
install(FILES README.md LICENSE.TXT DESTINATION .)

# Package
#add_subdirectory(package)
