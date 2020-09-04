find_package(Qt5Core REQUIRED)
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5Multimedia  REQUIRED)
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

# Driverss first
add_subdirectory(drivers)


set(headers
    ClientApp.h
    ConfigManagerClient.h
    LoginDialog.h
    ComManager.h
    WebSocketManager.h
    MainWindow.h
    ProjectNavigator.h
    NotificationWindow.h
    Message.h
    GlobalMessageBox.h
    ConfigWidget.h
    HistoryCalendarWidget.h
    DeviceAssignDialog.h
    DownloadedFile.h
    DownloadProgressDialog.h
    BaseDialog.h
    GlobalEvent.h
    GlobalEventLogger.h
    InSessionWidget.h
    StartSessionDialog.h
    SessionLobbyDialog.h
    SessionInviteWidget.h
    editors/DataEditorWidget.h
    editors/UserWidget.h
    editors/TeraForm.h
    editors/DataListWidget.h
    editors/SiteWidget.h
    editors/DeviceWidget.h
    editors/ProjectWidget.h
    editors/GroupWidget.h
    editors/ParticipantWidget.h
    editors/SessionTypeWidget.h
    editors/SessionWidget.h
    editors/DeviceSubTypeWidget.h
    editors/UserGroupWidget.h
    editors/ServiceWidget.h
    editors/ServiceConfigWidget.h
    wizards/UserWizard.h
    wizards/BaseWizard.h
    services/BaseServiceWidget.h
    services/VideoRehabService/VideoRehabWidget.h
    services/VideoRehabService/VideoRehabWebPage.h
    services/VideoRehabService/VideoRehabSetupWidget.h
)

set(srcs
    main.cpp
    ClientApp.cpp
    ConfigManagerClient.cpp
    LoginDialog.cpp
    ComManager.cpp
    WebSocketManager.cpp
    MainWindow.cpp
    ProjectNavigator.cpp
    NotificationWindow.cpp
    Message.cpp
    GlobalMessageBox.cpp
    ConfigWidget.cpp
    HistoryCalendarWidget.cpp
    DeviceAssignDialog.cpp
    DownloadedFile.cpp
    DownloadProgressDialog.cpp
    BaseDialog.cpp
    GlobalEvent.cpp
    GlobalEventLogger.cpp
    InSessionWidget.cpp
    StartSessionDialog.cpp
    SessionLobbyDialog.cpp
    SessionInviteWidget.cpp
    editors/DataEditorWidget.cpp
    editors/UserWidget.cpp
    editors/TeraForm.cpp
    editors/DataListWidget.cpp
    editors/SiteWidget.cpp
    editors/DeviceWidget.cpp
    editors/ProjectWidget.cpp
    editors/GroupWidget.cpp
    editors/ParticipantWidget.cpp
    editors/SessionTypeWidget.cpp
    editors/SessionWidget.cpp
    editors/DeviceSubTypeWidget.cpp
    editors/UserGroupWidget.cpp
    editors/ServiceWidget.cpp
    editors/ServiceConfigWidget.cpp
    wizards/UserWizard.cpp
    wizards/BaseWizard.cpp
    services/BaseServiceWidget.cpp
    services/VideoRehabService/VideoRehabWidget.cpp
    services/VideoRehabService/VideoRehabWebPage.cpp
    services/VideoRehabService/VideoRehabSetupWidget.cpp
)

SET(uis
    LoginDialog.ui
    MainWindow.ui
    ProjectNavigator.ui
    notification.ui
    ConfigWidget.ui
    DeviceAssignDialog.ui
    DownloadProgressDialog.ui
    BaseDialog.ui
    InSessionWidget.ui
    StartSessionDialog.ui
    SessionLobbyDialog.ui
    SessionInviteWidget.ui
    editors/UserWidget.ui
    editors/TeraForm.ui
    editors/DataListWidget.ui
    editors/SiteWidget.ui
    editors/DeviceWidget.ui
    editors/ProjectWidget.ui
    editors/GroupWidget.ui
    editors/ParticipantWidget.ui
    editors/SessionTypeWidget.ui
    editors/SessionWidget.ui
    editors/DeviceSubTypeWidget.ui
    editors/UserGroupWidget.ui
    editors/ServiceWidget.ui
    editors/ServiceConfigWidget.ui
    services/VideoRehabService/VideoRehabWidget.ui
    services/VideoRehabService/VideoRehabSetupWidget.ui
)

SET(qrcs
    ${TERACLIENT_RES_INCLUDES}/TeraClient.qrc
)

#Generate .h files from the .ui files
QT5_WRAP_UI(moc_uis ${uis})

# generate rules for building source files from the resources
QT5_ADD_RESOURCES(client_qrc ${qrcs})

#This will generate moc_* for Qt
qt5_wrap_cpp(moc_srcs ${headers})

set(translation_files_srcs
    ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_fr.ts
    ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_en.ts
)

#Translation files, this is done manually to avoid removing the file when doing make clean
add_custom_target(openteraplus_en_ts
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -target-language en ${srcs} ${headers} ${moc_uis} -ts ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_en.ts
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${moc_uis}
)

add_custom_target(openteraplus_fr_ts
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -target-language fr ${srcs} ${headers} ${moc_uis} -ts ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_fr.ts
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${moc_uis}
)

#set qm files output directory
set_source_files_properties(${translation_files_srcs} PROPERTIES OUTPUT_LOCATION ${TERACLIENT_RES_INCLUDES}/translations/)

#Build this target to update translations
add_custom_target(translations DEPENDS openteraplus_en_ts openteraplus_fr_ts)
add_custom_target(translation_files SOURCES ${translation_files_srcs})


#Generate qm files from .ts files
qt5_add_translation(qm_files ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_en.ts ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_fr.ts)

message(STATUS "qm_files : ${qm_files}")

include_directories(
    ${TERACLIENT_RES_INCLUDES}
    ${OPENTERA_MESSAGES_INCLUDES}
    ${OPENTERA_SHARED_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/editors
    ${CMAKE_CURRENT_BINARY_DIR}
    # DL Temporary test for virtual cameras
    ${VIRTUAL_CAMERA_INCLUDES}
    ${AVKYS_INCLUDES}
    )

# Application icon
SET(icon openteraico.rc)

# Will be used in Info.plist.in
set(MACOSX_BUNDLE_INFO_STRING "OpenTeRa+")
set(MACOSX_BUNDLE_ICON_FILE "OpenTeraPlus.icns")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "OpenTeRa+")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_NAME "OpenTeRa")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${CPACK_PACKAGE_VERSION}")
set(MACOSX_BUNDLE_COPYRIGHT "IntRolab/CDRV UdeS")

set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE}")

# create an executable file named "TeraClient" from the source files
# Put qrc files after qm files
add_executable(TeraClient MACOSX_BUNDLE WIN32 ${icon} ${srcs} ${headers} ${moc_srcs} ${moc_uis} ${ICON_PATH} ${qm_files} ${client_qrc})
# qt5_use_modules(TeraClient Core Network Multimedia WebSockets WebEngine WebEngineWidgets WebSockets)

# Linking with Qt libraries and others
target_link_libraries(TeraClient ${OPENTERA_SHARED_LIBS}  Qt5::Core Qt5::Network Qt5::Multimedia Qt5::WebSockets Qt5::WebEngine Qt5::WebEngineWidgets)

# DL - Temporary for tests
target_link_libraries(TeraClient ${VIRTUAL_CAMERA_LIBS})

set_target_properties(TeraClient PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/bin
        INSTALL_RPATH_USE_LINK_PATH TRUE)

# Automatic generation of ts files on target
# Remove if you don't want to generate translations every time.
add_dependencies(TeraClient translations)

# Set a custom plist file for the app bundle
set_target_properties(TeraClient PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)

#This will copy icon to Resources directory
set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

# Install target
install(TARGETS TeraClient DESTINATION bin)




