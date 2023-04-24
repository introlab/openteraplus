project(OpenTeraPlusWebAssembly)

#Look for minimum cmake version
cmake_minimum_required(VERSION 3.21)

# Software version
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
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

# Building protobuf webasm
# https://github.com/protocolbuffers/protobuf/releases/download/v22.3/protoc-22.3-win64.zip

set(protoc_archive_name protoc-22.3-win64.zip)
set(protc_binaries_url https://github.com/protocolbuffers/protobuf/releases/download/v22.3/${protoc_archive_name})

# Download Archive
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${protoc_archive_name})
    file(DOWNLOAD
        ${protc_binaries_url}
        ${CMAKE_CURRENT_BINARY_DIR}/${protoc_archive_name}
        TIMEOUT 300 #seconds
        TLS_VERIFY ON
        SHOW_PROGRESS)
endif()

# Extract Archive
file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_BINARY_DIR}/${protoc_archive_name} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/protoc)

set(protobuf_PROTOC_EXE ${CMAKE_CURRENT_BINARY_DIR}/protoc/bin/protoc.exe)
set(protobuf_BUILD_EXAMPLES OFF)
# No protoc, will download native
set(protobuf_BUILD_PROTOC_BINARIES OFF) 
set(protobuf_BUILD_LIBPROTOC OFF)
set(protobuf_BUILD_SHARED_LIBS_DEFAULT OFF)



add_subdirectory(../external/protobuf/cmake protobuf)

#Generate messages
#add_subdirectory(messages/cpp)
