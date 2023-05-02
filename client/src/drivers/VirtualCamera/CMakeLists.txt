find_package(Qt6Core REQUIRED)
find_package(Qt6LinguistTools REQUIRED)
find_package(Qt6Qml REQUIRED)
find_package(Qt6Concurrent REQUIRED)
find_package(Qt6Quick REQUIRED)

set(VirtualCamera_headers
    VirtualCamera.h
    VirtualCameraThread.h
)

set(VirtualCamera_srcs
    VirtualCamera.cpp
    VirtualCameraThread.cpp
)

SET(VirtualCamera_qrcs

)

include_directories(
    ${OPENTERA_SHARED_INCLUDES}
    ${AVKYS_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

message(STATUS "PLUGIN_DIRECTORY: ${AVKYS_PLUGIN_DIRECTORY}")

# generate rules for building source files from the resources
QT6_ADD_RESOURCES(virtual_camera_qrc ${VirtualCamera_qrcs})

#This will generate moc_* for Qt
qt6_wrap_cpp(virtual_camera_moc_srcs ${VirtualCamera_headers})
add_definitions(-DAVKYS_PLUGIN_DIRECTORY="${AVKYS_PLUGIN_DIRECTORY}")

add_library(VirtualCameraDriver STATIC ${VirtualCamera_headers} ${VirtualCamera_srcs} ${virtual_camera_qrc} ${virtual_camera_moc_srcs})
target_link_libraries(VirtualCameraDriver Qt6::Core Qt6::Qml Qt6::Concurrent Qt6::Quick ${AVKYS_LIBS})

set(VIRTUAL_CAMERA_LIBS VirtualCameraDriver CACHE INTERNAL "doc string")
set(VIRTUAL_CAMERA_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "doc string")

# Install
#install(TARGETS VirtualCameraDriver
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

