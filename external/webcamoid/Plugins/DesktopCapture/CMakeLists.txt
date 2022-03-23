set(PLUGIN_NAME DesktopCapture)
message(STATUS "Building ${PLUGIN_NAME} Plugin.")

add_definitions(-DUNICODE)

find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)

include_directories(
    ${AVKYS_PLUGIN_INCLUDES}
    src
)

set (plugin_headers
    src/desktopcapture.h
    src/desktopcaptureelement.h
    src/desktopcaptureglobals.h
    src/screendev.h
)

set (plugin_srcs
    src/desktopcapture.cpp
    src/desktopcaptureelement.cpp
    src/desktopcaptureglobals.cpp
    src/screendev.cpp
)

set (plugin_qrc
    DesktopCapture.qrc
    #translations.qrc
)


#This will generate moc_* for Qt
QT5_WRAP_CPP(plugin_moc_srcs ${plugin_headers})

QT5_ADD_RESOURCES(plugin_qrc_srcs ${plugin_qrc})

add_library(${PLUGIN_NAME} SHARED ${plugin_srcs} ${plugin_headers} ${plugin_moc_srcs} ${plugin_qrc_srcs})
# qt5_use_modules(${PLUGIN_NAME} Core Concurrent Gui Qml)
target_link_libraries(${PLUGIN_NAME} Qt5::Core Qt5::Concurrent Qt5::Gui Qt5::Qml)
target_link_libraries(${PLUGIN_NAME} ${AVKYS_PLUGIN_LIBRARIES})
install(TARGETS ${PLUGIN_NAME} DESTINATION ${AVKYS_LIBRARY_INSTALL_PATH})



#submodule(s)
FILE(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH}/submodules/${PLUGIN_NAME})

add_subdirectory(src/qtscreen)
