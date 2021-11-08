set(SUBMMODULE_NAME "dshow")
message(STATUS "Building ${PLUGIN_NAME} : ${SUBMMODULE_NAME} submodule.")

SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}/submodules/${PLUGIN_NAME})

#Very important to get .lib files
SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Widgets REQUIRED)


add_subdirectory(VirtualCameraFilter)

include_directories(
    ${AVKYS_PLUGIN_INCLUDES}
    ./
    src
    VirtualCameraFilter/ipc/src
)

set (submodule_headers
    src/plugin.h
    src/cameraoutdshow.h
    ../cameraout.h
)

set (submodule_srcs
    src/plugin.cpp
    src/cameraoutdshow.cpp
    ../cameraout.cpp

)

set (submodule_qrc

)






#This will generate moc_* for Qt
QT5_WRAP_CPP(submodule_moc_srcs ${submodule_headers})

QT5_ADD_RESOURCES(submodule_qrc_srcs ${summodule_qrc})


set (dshow_additional_libs
    strmiids
    uuid
    ole32
    oleaut32
    shell32
)

add_library(${SUBMMODULE_NAME} SHARED ${submodule_srcs} ${submodule_headers} ${submodule_moc_srcs} ${submodule_qrc_srcs})
# qt5_use_modules(${SUBMMODULE_NAME} Core Concurrent Widgets Gui Qml)
target_link_libraries(${SUBMMODULE_NAME} Qt5::Core Qt5::Concurrent Qt5::Widgets Qt5::Gui Qt5::Qml)
target_link_libraries(${SUBMMODULE_NAME} ${AVKYS_PLUGIN_LIBRARIES} ${dshow_additional_libs} ipc)
install(TARGETS ${SUBMMODULE_NAME} DESTINATION ${AVKYS_LIBRARY_INSTALL_PATH}/submodules/${PLUGIN_NAME})
