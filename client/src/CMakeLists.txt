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
    dialogs/LoginDialog.h
    ComManager.h
    WebSocketManager.h
    MainWindow.h
    Message.h
    GlobalMessageBox.h
    DownloadedFile.h
    GlobalEvent.h
    GlobalEventLogger.h
    dialogs/StartSessionDialog.h
    dialogs/SessionLobbyDialog.h
    dialogs/JoinSessionDialog.h
    dialogs/EmailInviteDialog.h
    dialogs/AboutDialog.h
    dialogs/AboutDialogPage.h
    dialogs/GeneratePasswordDialog.h
    dialogs/PasswordStrengthDialog.h
    dialogs/DownloadProgressDialog.h
    dialogs/BaseDialog.h
    dialogs/DeviceAssignDialog.h
    editors/DataEditorWidget.h
    editors/UserWidget.h
    editors/UserSummaryWidget.h
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
    editors/DeviceTypeWidget.h
    editors/UserGroupWidget.h
    editors/ServiceWidget.h
    editors/ServiceConfigWidget.h
    wizards/UserWizard.h
    wizards/BaseWizard.h
    services/BaseServiceWidget.h
    services/BaseServiceToolsWidget.h
    services/BaseServiceSetupWidget.h
    services/VideoRehabService/VideoRehabWidget.h
    services/VideoRehabService/VideoRehabWebPage.h
    services/VideoRehabService/VideoRehabSetupWidget.h
    services/VideoRehabService/VideoRehabToolsWidget.h
    services/VideoRehabService/VideoRehabVirtualCamSetupDialog.h
    services/VideoRehabService/VideoRehabPTZDialog.h
    services/VideoRehabService/WebSocket/SharedObject.h
    services/VideoRehabService/WebSocket/WebSocketClientWrapper.h
    services/VideoRehabService/WebSocket/WebSocketTransport.h
    widgets/InSessionWidget.h
    widgets/SessionInviteWidget.h
    widgets/OnlineManagerWidget.h
    widgets/ClickableLabel.h
    widgets/ConfigWidget.h
    widgets/HistoryCalendarWidget.h
    widgets/ProjectNavigator.h
    widgets/NotificationWindow.h
)

set(srcs
    main.cpp
    ClientApp.cpp
    ConfigManagerClient.cpp
    ComManager.cpp
    WebSocketManager.cpp
    MainWindow.cpp
    Message.cpp
    GlobalMessageBox.cpp
    DownloadedFile.cpp
    GlobalEvent.cpp
    GlobalEventLogger.cpp
    dialogs/LoginDialog.cpp
    dialogs/DeviceAssignDialog.cpp
    dialogs/DownloadProgressDialog.cpp
    dialogs/BaseDialog.cpp
    dialogs/StartSessionDialog.cpp
    dialogs/SessionLobbyDialog.cpp
    dialogs/JoinSessionDialog.cpp
    dialogs/EmailInviteDialog.cpp
    dialogs/AboutDialog.cpp
    dialogs/AboutDialogPage.cpp
    dialogs/GeneratePasswordDialog.cpp
    dialogs/PasswordStrengthDialog.cpp
    editors/DataEditorWidget.cpp
    editors/UserWidget.cpp
    editors/UserSummaryWidget.cpp
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
    editors/DeviceTypeWidget.cpp
    editors/UserGroupWidget.cpp
    editors/ServiceWidget.cpp
    editors/ServiceConfigWidget.cpp
    wizards/UserWizard.cpp
    wizards/BaseWizard.cpp
    services/BaseServiceWidget.cpp
    services/BaseServiceToolsWidget.cpp
    services/BaseServiceSetupWidget.cpp
    services/VideoRehabService/VideoRehabWidget.cpp
    services/VideoRehabService/VideoRehabWebPage.cpp
    services/VideoRehabService/VideoRehabSetupWidget.cpp
    services/VideoRehabService/VideoRehabToolsWidget.cpp
    services/VideoRehabService/VideoRehabVirtualCamSetupDialog.cpp
    services/VideoRehabService/VideoRehabPTZDialog.cpp
    services/VideoRehabService/WebSocket/SharedObject.cpp
    services/VideoRehabService/WebSocket/WebSocketClientWrapper.cpp
    services/VideoRehabService/WebSocket/WebSocketTransport.cpp
    widgets/ProjectNavigator.cpp
    widgets/NotificationWindow.cpp
    widgets/ConfigWidget.cpp
    widgets/HistoryCalendarWidget.cpp
    widgets/InSessionWidget.cpp
    widgets/SessionInviteWidget.cpp
    widgets/OnlineManagerWidget.cpp
    widgets/ClickableLabel.cpp
)

SET(uis
    dialogs/LoginDialog.ui
    MainWindow.ui
    dialogs/DeviceAssignDialog.ui
    dialogs/DownloadProgressDialog.ui
    dialogs/BaseDialog.ui
    dialogs/StartSessionDialog.ui
    dialogs/SessionLobbyDialog.ui
    dialogs/JoinSessionDialog.ui
    dialogs/EmailInviteDialog.ui
    dialogs/AboutDialog.ui
    dialogs/GeneratePasswordDialog.ui
    dialogs/PasswordStrengthDialog.ui
    editors/UserWidget.ui
    editors/UserSummaryWidget.ui
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
    editors/DeviceTypeWidget.ui
    editors/UserGroupWidget.ui
    editors/ServiceWidget.ui
    editors/ServiceConfigWidget.ui
    services/VideoRehabService/VideoRehabWidget.ui
    services/VideoRehabService/VideoRehabSetupWidget.ui
    services/VideoRehabService/VideoRehabToolsWidget.ui
    services/VideoRehabService/VideoRehabVirtualCamSetupDialog.ui
    services/VideoRehabService/VideoRehabPTZDialog.ui
    widgets/ProjectNavigator.ui
    widgets/notification.ui
    widgets/ConfigWidget.ui
    widgets/InSessionWidget.ui
    widgets/SessionInviteWidget.ui
    widgets/OnlineManagerWidget.ui
)

SET(qrcs
    ${TERACLIENT_RES_INCLUDES}/TeraClient.qrc
    services/VideoRehabService/resources/VideoRehabService.qrc
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
    ${PTZ_DRIVERS_INCLUDES}
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

# create an executable file named "OpenTeraPlus" from the source files
# Put qrc files after qm files
add_executable(OpenTeraPlus MACOSX_BUNDLE WIN32 ${icon} ${srcs} ${headers} ${moc_srcs} ${moc_uis} ${ICON_PATH} ${qm_files} ${client_qrc})
# qt5_use_modules(TeraClient Core Network Multimedia WebSockets WebEngine WebEngineWidgets WebSockets)

# Linking with Qt libraries and others
target_link_libraries(OpenTeraPlus ${OPENTERA_SHARED_LIBS} ${OPENTERA_MESSAGES_LIBS} Qt5::Core Qt5::Network Qt5::Multimedia Qt5::WebSockets Qt5::WebEngine Qt5::WebEngineWidgets)
target_link_libraries(OpenTeraPlus ${VIRTUAL_CAMERA_LIBS})
target_link_libraries(OpenTeraPlus ${PTZ_DRIVERS_LIBS})

set_target_properties(OpenTeraPlus PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/bin
        INSTALL_RPATH_USE_LINK_PATH TRUE)

# Automatic generation of ts files on target
# Remove if you don't want to generate translations every time.
# add_dependencies(TeraClient translations)

# Set a custom plist file for the app bundle
set_target_properties(OpenTeraPlus PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)

#This will copy icon to Resources directory
set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

# Install target
install(TARGETS OpenTeraPlus DESTINATION bin)

# Doesn't seem to be installed otherwise??
install(TARGETS opentera_messages DESTINATION bin)




