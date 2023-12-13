set(PLUGIN_NAME MultiSrc)
message(STATUS "Building ${PLUGIN_NAME} Plugin.")


find_package(Qt6Core REQUIRED)
find_package(Qt6Concurrent REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Qml REQUIRED)

include_directories(
    ${AVKYS_PLUGIN_INCLUDES}
    src
)

set (plugin_headers
    src/multisrc.h
    src/multisrcelement.h
    src/mediasource.h
    src/multisrcglobals.h
)

set (plugin_srcs
    src/multisrc.cpp
    src/multisrcelement.cpp
    src/mediasource.cpp
    src/multisrcglobals.cpp
)

set (plugin_qrc
    MultiSrc.qrc
    translations.qrc
)


#This will generate moc_* for Qt
QT_WRAP_CPP(plugin_moc_srcs ${plugin_headers})

QT6_ADD_RESOURCES(plugin_qrc_srcs ${plugin_qrc})

add_library(${PLUGIN_NAME} SHARED ${plugin_srcs} ${plugin_headers} ${plugin_moc_srcs} ${plugin_qrc_srcs})
# qt5_use_modules(${PLUGIN_NAME} Core Concurrent Gui Qml)
target_link_libraries(${PLUGIN_NAME} Qt6::Core Qt6::Concurrent Qt6::Gui Qt6::Qml)
target_link_libraries(${PLUGIN_NAME} ${AVKYS_PLUGIN_LIBRARIES})
install(TARGETS ${PLUGIN_NAME} DESTINATION ${AVKYS_LIBRARY_INSTALL_PATH})

#submodule(s)
FILE(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH}/submodules/${PLUGIN_NAME})
add_subdirectory(src/ffmpeg)
