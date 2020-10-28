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
qt5_wrap_cpp(ptz_test_moc_uis ${ptz_test_headers})

add_executable(ptz_test MACOSX_BUNDLE WIN32
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

if(APPLE)

    set_target_properties(ptz_test PROPERTIES
            INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/bin
            INSTALL_RPATH_USE_LINK_PATH TRUE)


    # Set a custom plist file for the app bundle
    set_target_properties(ptz_test PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)


    # Will be used in Info.plist.in
    set(MACOSX_BUNDLE_INFO_STRING "ptz_test")
    set(MACOSX_BUNDLE_ICON_FILE "")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "ptz_test")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME "ptz_test")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${CPACK_PACKAGE_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "IntRolab/CDRV UdeS")


    add_custom_target(info_plist SOURCES Info.plist.in)
endif(APPLE)

