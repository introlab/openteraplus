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
# No protoc, will download native
file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_BINARY_DIR}/${protoc_archive_name} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/protoc)

set(ABSL_PROPAGATE_CXX_STD OFF)
set(protobuf_PROTOC_EXE ${CMAKE_CURRENT_BINARY_DIR}/protoc/bin/protoc.exe)
set(protobuf_BUILD_EXAMPLES OFF)
set(protobuf_BUILD_PROTOC_BINARIES OFF) 
set(protobuf_BUILD_LIBPROTOC OFF)
set(protobuf_BUILD_SHARED_LIBS_DEFAULT OFF)
set(protobuf_BUILD_TESTS OFF)
set(protobuf_INSTALL OFF)

add_compile_options(-pthread)
add_subdirectory(../external/protobuf protobuf)

#Generate messages
#add_subdirectory(messages/cpp)
set (MESSAGE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../external/messages/proto)
FILE(GLOB_RECURSE messages RELATIVE "${MESSAGE_PATH}" "${MESSAGE_PATH}/*.proto")
FILE(GLOB_RECURSE messages_sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${MESSAGE_PATH}/*.proto")
add_custom_target(proto_files SOURCES ${messages_sources})

#Message files output directory, will create directory
set (output_directory ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY ${output_directory})

set (generated_headers "")
set (generated_srcs "")

# Include path
set(GOOGLE_PROTO_PATH ${PROJECT_SOURCE_DIR}/../external/protobuf/src)

message(STATUS "GOOGLE_PROTO_PATH: ${GOOGLE_PROTO_PATH}")

#Generate code using protoc compiler
foreach (file ${messages})

    message(STATUS "Processing : ${file}")

    get_filename_component (name_without_extension ${file} NAME_WE)

    message(STATUS "name_without_extension : ${name_without_extension}")

    set(output_header_file ${output_directory}/${name_without_extension}.pb.h)
    set(output_src_file ${output_directory}/${name_without_extension}.pb.cc)

    #Complete files to be compiled...
    list(APPEND generated_headers ${output_header_file})
    list(APPEND generated_srcs ${output_src_file})

    #Should execute protoc on each .proto files
    #Ex: protoc -I=$SRC_DIR --cpp_out=$DST_DIR $SRC_DIR/addressbook.proto
    add_custom_command(OUTPUT ${output_header_file} ${output_src_file}
        COMMAND ${protobuf_PROTOC_EXE} -I=${MESSAGE_PATH} -I${GOOGLE_PROTO_PATH} --cpp_out=${output_directory} ${file}
        DEPENDS ${MESSAGE_PATH}/${file} ${protobuf_PROTOC_EXE}
        WORKING_DIRECTORY ${MESSAGE_PATH}
        VERBATIM)

endforeach()

message (STATUS "Headers : ${generated_headers}")
message (STATUS "Sources : ${generated_srcs}")

include_directories(
    ${PROJECT_SOURCE_DIR}/../external/protobuf/src
    ${output_directory}
)

#compile library
add_library(opentera_messages STATIC ${generated_headers} ${generated_srcs})
#Link with protobuf
target_link_libraries(opentera_messages libprotobuf)
target_compile_definitions(opentera_messages PUBLIC OPENTERA_PROTOCOL_VERSION=1)
target_compile_options(opentera_messages PUBLIC -pthread)
get_target_property(all_includes opentera_messages INCLUDE_DIRECTORIES)

message (STATUS "includes : ${all_includes}")

set(OPENTERA_MESSAGES_INCLUDES ${all_includes} CACHE INTERNAL "doc string")
set(OPENTERA_MESSAGES_LIBS "opentera_messages" CACHE INTERNAL "doc string")

add_definitions(-DOPENTERA_WEBASSEMBLY=1)
set(OPENTERA_WEBASSEMBLY ON)

# Shared data
add_subdirectory(../shared shared)

# Client
add_subdirectory(../client client)

qt6_finalize_project()
