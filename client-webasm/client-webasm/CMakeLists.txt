cmake_minimum_required(VERSION 3.5)

project(client-webasm VERSION 0.1 LANGUAGES CXX)

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

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DWEBASSEMBLY_COMPILATION)
add_definitions(-DOPENTERAMESSAGES_EXPORT=)
add_definitions(-DOPENTERA_PROTOCOL_VERSION=1)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WebSockets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui REQUIRED)


#Protobuf

#Will verbose protobuf CMake.
option(protobuf_VERBOSE "" ON)

#Will build shared libs
set(BUILD_SHARED_LIBS OFF)

# Build position independant code (-fPIC)
# set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#Disable tests for now...
option(protobuf_BUILD_TESTS "" OFF)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../external/protobuf/cmake external/protobuf)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src/managers/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../shared/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../shared/src/data/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/protobuf/src
    ${CMAKE_CURRENT_SOURCE_DIR}/messages
)


#TEMP, this should be generated from proto files
FILE(GLOB_RECURSE messages_sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "messages/*.cc")
message(STATUS "MESSAGES ---- ${messages_sources}")


set(PROJECT_SOURCES
        ${messages_sources}
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ../../client/src/managers/ComManager.cpp
        ../../client/src/managers/ConfigManagerClient.cpp
        ../../client/src/managers/ParticipantComManager.cpp
        ../../client/src/managers/WebSocketManager.cpp
        ../../shared/src/Utils.cpp
        ../../shared/src/Logger.cpp
        ../../shared/src/data/TeraData.cpp
        ../../client/src/managers/ComManager.h
        ../../client/src/managers/ConfigManagerClient.h
        ../../client/src/managers/ParticipantComManager.h
        ../../client/src/managers/WebSocketManager.h
        ../../shared/src/Utils.h
        ../../shared/src/Logger.h
        ../../shared/src/data/TeraData.h
)


add_executable(client-webasm
    ${PROJECT_SOURCES}
)


target_link_libraries(client-webasm PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebSockets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    libprotobuf
)

