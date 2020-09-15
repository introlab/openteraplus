find_package(Qt5Core REQUIRED)
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5Multimedia  REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)


include_directories(
    ${OPENTERA_MESSAGES_INCLUDES}
    ${OPENTERA_SHARED_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${VIRTUAL_CAMERA_INCLUDES}
    ${AVKYS_INCLUDES}
    ${PTZ_DRIVERS_INCLUDES}
)

set(ptz_test_headers
    ptz_test.h
)

set(ptz_test_srcs
    ptz_test.cpp
)

SET(ptz_test_uis
    ptz_test.ui
)

SET(ptz_test_qrcs

)

#Generate .h files from the .ui files
QT5_WRAP_UI(ptz_test_moc_uis ${ptz_test_uis})

# generate rules for building source files from the resources
QT5_ADD_RESOURCES(ptz_test_qrc_res ${ptz_test_qrcs})

#This will generate moc_* for Qt
qt5_wrap_cpp(ptz_test_moc_srcs ${ptz_test_headers})

add_executable(ptz_test
    ${ptz_test_headers}
    ${ptz_test_moc_srcs}
    ${ptz_test_qrc_res}
    ${ptz_test_moc_uis}
    ${ptz_test_srcs})


# Linking with Qt libraries and others
target_link_libraries(ptz_test
    ${OPENTERA_SHARED_LIBS}
    ${VIRTUAL_CAMERA_LIBS}
    ${PTZ_DRIVERS_LIBS}
    Qt5::Core
    Qt5::Network
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::WebSockets
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    )

