# TODO verify if submodules are initialized.
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/protobuf)
    message(FATAL_ERROR "Please init git submodules first.")
endif()

if (MSVC)
    add_definitions(
      /wd4910 # Ignore export warnings (C4910) with MSVC
      /wd4661 # Ignore no suitable definition for explicit template (C4661)
      /wd4251 # Ignore needs to have dll-interface
      /wd4146 # Ignore unary minus operator applied to unsigned type, result still unsigned
      /wd4267 # Ignore size_t to uint32_t conversion warnings
    )
endif (MSVC)

#Will verbose protobuf CMake.
option(protobuf_VERBOSE "" ON)

#Will build shared libs
set(BUILD_SHARED_LIBS ON)

# Build position independant code (-fPIC)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#Disable tests for now...
option(protobuf_BUILD_TESTS "" OFF)
set(ABSL_PROPAGATE_CXX_STD OFF)
set(protobuf_BUILD_EXAMPLES OFF)
set(protobuf_BUILD_LIBPROTOC OFF)
set(protobuf_BUILD_TESTS OFF)


#Use dynamic MSVC runtime
if (MSVC)
    option(protobuf_MSVC_STATIC_RUNTIME "" OFF)
endif(MSVC)

add_subdirectory(protobuf)

#Generate messages
add_subdirectory(messages/cpp)

#Webcamoid
add_subdirectory(webcamoid)


