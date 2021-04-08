project(OpenTeraPlus)

include(CTest)
enable_testing()
#include(CheckFunctionExists)

#Look for minimum cmake version
cmake_minimum_required(VERSION 3.0.2)

#DEFINITIONS POLICY, NEW SINCE 3.0.2
cmake_policy(SET CMP0043 NEW)

# Software version
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "3")
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
    set(CMAKE_INSTALL_BINDIR OpenTeraPlus.app/Contents/bin)
    set(CMAKE_INSTALL_LIBDIR OpenTeraPlus.app/Contents/lib)
    set (CMAKE_INSTALL_INCLUDEDIR OpenTeraPlus.app/Contents/include)
endif(APPLE)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

message(STATUS "Using QMAKE: ${QT_QMAKE_EXECUTABLE}")

# This will enable to output everything in the bin directory for testing and debugging...
# SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# SET (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# External dependencies first
add_subdirectory(external)

# Shared data
add_subdirectory(shared)

# Client
add_subdirectory(client)

# Tests
add_subdirectory(tests)

# Package
add_subdirectory(package)

# Readme and other files
add_custom_target(readme SOURCES README.md LICENSE.TXT)

# Install LICENSE / README
install(FILES README.md LICENSE.TXT DESTINATION .)

