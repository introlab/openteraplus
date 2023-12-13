set(SUBMMODULE_NAME "ffmpeg")
message(STATUS "Building ${PLUGIN_NAME} : ${SUBMMODULE_NAME} submodule.")

#FFMPEG DEFINITIONS
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DHAVE_FREECONTEXT)
add_definitions(-DHAVE_PACKETREF)
add_definitions(-DHAVE_SENDRECV)
add_definitions(-DHAVE_SUBTITLEDATA)
add_definitions(-DHAVE_CODECPAR)
add_definitions(-DHAVE_FRAMEALLOC)
add_definitions(-DHAVE_SAMPLEFORMAT64)

SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}/submodules/${PLUGIN_NAME})

find_package(Qt6Core REQUIRED)
find_package(Qt6Concurrent REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Qml REQUIRED)
find_package(Qt6Widgets REQUIRED)

include_directories(
    ${AVKYS_PLUGIN_INCLUDES}
    ${FFMPEG_INCLUDES}
    ./
    src
)

set (submodule_headers
    src/plugin.h
    src/mediasourceffmpeg.h
    src/abstractstream.h
    src/audiostream.h
    src/subtitlestream.h
    src/videostream.h
    src/clock.h
    ../mediasource.h
)

set (submodule_srcs
    src/plugin.cpp
    src/mediasourceffmpeg.cpp
    src/abstractstream.cpp
    src/audiostream.cpp
    src/subtitlestream.cpp
    src/videostream.cpp
    src/clock.cpp
    ../mediasource.cpp
)

set (submodule_qrc

)


#This will generate moc_* for Qt
QT_WRAP_CPP(submodule_moc_srcs ${submodule_headers})

QT6_ADD_RESOURCES(submodule_qrc_srcs ${summodule_qrc})

add_library(${SUBMMODULE_NAME} SHARED ${submodule_srcs} ${submodule_headers} ${submodule_moc_srcs} ${submodule_qrc_srcs})
# qt5_use_modules(${SUBMMODULE_NAME} Core Concurrent Widgets Gui Qml)
target_link_libraries(${SUBMMODULE_NAME} Qt6::Core Qt6::Concurrent Qt6::Widgets Qt6::Gui Qt6::Qml)
target_link_libraries(${SUBMMODULE_NAME} ${AVKYS_PLUGIN_LIBRARIES} ${FFMPEG_LIBS})
install(TARGETS ${SUBMMODULE_NAME} DESTINATION ${AVKYS_LIBRARY_INSTALL_PATH}/submodules/${PLUGIN_NAME})


