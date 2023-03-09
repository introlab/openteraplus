find_package(Qt5Core REQUIRED)
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5Multimedia  REQUIRED)
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

# Drivers first
add_subdirectory(drivers)


set(headers
    ClientApp.h
    GlobalMessageBox.h
    # Data structures
    data/DownloadingFile.h
    data/GlobalEvent.h
    data/GlobalEventLogger.h
    data/Message.h
    data/TransferringFile.h
    data/UploadingFile.h
    data/IconMenuDelegate.h
    # Main Windows
    main/MainWindow.h
    main/MainKitWindow.h
    # Managers
    managers/ConfigManagerClient.h
    managers/ComManager.h
    managers/ParticipantComManager.h
    managers/WebSocketManager.h
    managers/BaseComManager.h
    managers/AssetComManager.h
    # Dialogs
    dialogs/LoginDialog.h
    dialogs/StartSessionDialog.h
    dialogs/SessionLobbyDialog.h
    dialogs/JoinSessionDialog.h
    dialogs/EmailInviteDialog.h
    dialogs/AboutDialog.h
    dialogs/AboutDialogPage.h
    dialogs/GeneratePasswordDialog.h
    dialogs/PasswordStrengthDialog.h
    dialogs/TransferProgressDialog.h
    dialogs/BaseDialog.h
    dialogs/DeviceAssignDialog.h
    dialogs/CleanUpDialog.h
    dialogs/FileUploaderDialog.h
    dialogs/QRCodeDialog.h
    # Editors
    editors/DataEditorWidget.h    
    editors/DataListWidget.h
    editors/DeviceSubTypeWidget.h
    editors/DeviceSummaryWidget.h
    editors/DeviceTypeWidget.h
    editors/DeviceWidget.h
    editors/ProjectWidget.h
    editors/GroupWidget.h
    editors/ParticipantWidget.h
    editors/SessionTypeWidget.h
    editors/SessionWidget.h
    editors/ServiceWidget.h
    editors/ServiceConfigWidget.h
    editors/SiteWidget.h
    editors/TeraForm.h
    editors/TestTypeWidget.h
    editors/UserGroupWidget.h
    editors/UserSummaryWidget.h
    editors/UserWidget.h
    # Wizards
    wizards/BaseWizard.h
    wizards/UserWizard.h
    # Services
    services/BaseServiceWidget.h
    services/BaseServiceToolsWidget.h
    services/BaseServiceSetupWidget.h
    services/VideoRehabService/ScreenController.h
    services/VideoRehabService/VideoRehabWidget.h
    services/VideoRehabService/VideoRehabWebPage.h
    services/VideoRehabService/VideoRehabSetupWidget.h
    services/VideoRehabService/VideoRehabToolsWidget.h
    services/VideoRehabService/VideoRehabVirtualCamSetupDialog.h
    services/VideoRehabService/VideoRehabPTZDialog.h
    services/VideoRehabService/WebSocket/SharedObject.h
    services/VideoRehabService/WebSocket/WebSocketClientWrapper.h
    services/VideoRehabService/WebSocket/WebSocketTransport.h
    services/DanceService/DanceConfigWidget.h
    services/DanceService/DanceWebAPI.h
    services/DanceService/DanceComManager.h
    # Widgets
    widgets/InSessionWidget.h
    widgets/SessionInviteWidget.h
    widgets/OnlineManagerWidget.h
    widgets/ClickableLabel.h
    widgets/ConfigWidget.h
    widgets/HistoryCalendarWidget.h
    widgets/ProjectNavigator.h
    widgets/ProjectNavigatorTree.h
    widgets/NotificationWindow.h
    widgets/TableDateWidgetItem.h
    widgets/TableNumberWidgetItem.h
    widgets/DashboardWidget.h
    widgets/AssetsWidget.h
    widgets/ResultMessageWidget.h
    widgets/TestsWidget.h
    widgets/QRWidget.h
    widgets/LogViewWidget.h
    widgets/SessionsListWidget.h
    # Kits
    kit/KitConfigDialog.h
    kit/KitConfigManager.h
    kit/KitInSessionDialog.h
    kit/KitVideoRehabWidget.h
)

set(srcs
    main.cpp
    ClientApp.cpp
    GlobalMessageBox.cpp
    # Data structures
    data/DownloadingFile.cpp
    data/GlobalEvent.cpp
    data/GlobalEventLogger.cpp
    data/Message.cpp
    data/TransferringFile.cpp
    data/UploadingFile.cpp
    data/IconMenuDelegate.cpp
    # Main Windows
    main/MainWindow.cpp
    main/MainKitWindow.cpp
    # Managers
    managers/ConfigManagerClient.cpp
    managers/ComManager.cpp
    managers/WebSocketManager.cpp
    managers/ParticipantComManager.cpp
    managers/BaseComManager.cpp
    managers/AssetComManager.cpp
    # Dialogs
    dialogs/LoginDialog.cpp
    dialogs/DeviceAssignDialog.cpp
    dialogs/TransferProgressDialog.cpp
    dialogs/BaseDialog.cpp
    dialogs/StartSessionDialog.cpp
    dialogs/SessionLobbyDialog.cpp
    dialogs/JoinSessionDialog.cpp
    dialogs/EmailInviteDialog.cpp
    dialogs/AboutDialog.cpp
    dialogs/AboutDialogPage.cpp
    dialogs/GeneratePasswordDialog.cpp
    dialogs/PasswordStrengthDialog.cpp
    dialogs/CleanUpDialog.cpp
    dialogs/FileUploaderDialog.cpp
     dialogs/QRCodeDialog.cpp
    # Editors
    editors/DataEditorWidget.cpp
    editors/DataListWidget.cpp    
    editors/DeviceWidget.cpp
    editors/DeviceSubTypeWidget.cpp
    editors/DeviceSummaryWidget.cpp
    editors/DeviceTypeWidget.cpp
    editors/GroupWidget.cpp
    editors/ParticipantWidget.cpp
    editors/ProjectWidget.cpp
    editors/SessionTypeWidget.cpp
    editors/SessionWidget.cpp
    editors/ServiceWidget.cpp
    editors/ServiceConfigWidget.cpp
    editors/SiteWidget.cpp
    editors/TeraForm.cpp
    editors/TestTypeWidget.cpp
    editors/UserGroupWidget.cpp
    editors/UserSummaryWidget.cpp
    editors/UserWidget.cpp

    # Wizards
    wizards/UserWizard.cpp
    wizards/BaseWizard.cpp
    # Services
    services/BaseServiceWidget.cpp
    services/BaseServiceToolsWidget.cpp
    services/BaseServiceSetupWidget.cpp
    services/VideoRehabService/ScreenController.cpp
    services/VideoRehabService/VideoRehabWidget.cpp
    services/VideoRehabService/VideoRehabWebPage.cpp
    services/VideoRehabService/VideoRehabSetupWidget.cpp
    services/VideoRehabService/VideoRehabToolsWidget.cpp
    services/VideoRehabService/VideoRehabVirtualCamSetupDialog.cpp
    services/VideoRehabService/VideoRehabPTZDialog.cpp
    services/VideoRehabService/WebSocket/SharedObject.cpp
    services/VideoRehabService/WebSocket/WebSocketClientWrapper.cpp
    services/VideoRehabService/WebSocket/WebSocketTransport.cpp
    services/DanceService/DanceConfigWidget.cpp
    services/DanceService/DanceComManager.cpp
    # Widgets
    widgets/ProjectNavigator.cpp
    widgets/ProjectNavigatorTree.cpp
    widgets/NotificationWindow.cpp
    widgets/ConfigWidget.cpp
    widgets/HistoryCalendarWidget.cpp
    widgets/InSessionWidget.cpp
    widgets/SessionInviteWidget.cpp
    widgets/OnlineManagerWidget.cpp
    widgets/ClickableLabel.cpp
    widgets/TableDateWidgetItem.cpp
    widgets/TableNumberWidgetItem.cpp
    widgets/DashboardWidget.cpp
    widgets/AssetsWidget.cpp
    widgets/ResultMessageWidget.cpp
    widgets/TestsWidget.cpp
    widgets/QRWidget.cpp
    widgets/LogViewWidget.cpp
    widgets/SessionsListWidget.cpp
    # Kits
    kit/KitConfigDialog.cpp
    kit/KitConfigManager.cpp
    kit/KitInSessionDialog.cpp
    kit/KitVideoRehabWidget.cpp
)

SET(uis
    # Main Windows
    main/MainWindow.ui
    main/MainKitWindow.ui
    # Dialogs
    dialogs/LoginDialog.ui
    dialogs/DeviceAssignDialog.ui
    dialogs/TransferProgressDialog.ui
    dialogs/BaseDialog.ui
    dialogs/StartSessionDialog.ui
    dialogs/SessionLobbyDialog.ui
    dialogs/JoinSessionDialog.ui
    dialogs/EmailInviteDialog.ui
    dialogs/AboutDialog.ui
    dialogs/GeneratePasswordDialog.ui
    dialogs/PasswordStrengthDialog.ui
    dialogs/CleanUpDialog.ui
    dialogs/FileUploaderDialog.ui
    dialogs/QRCodeDialog.ui
    # Editors
    editors/DataListWidget.ui
    editors/DeviceSubTypeWidget.ui
    editors/DeviceSummaryWidget.ui
    editors/DeviceTypeWidget.ui
    editors/DeviceWidget.ui
    editors/GroupWidget.ui
    editors/ParticipantWidget.ui
    editors/ProjectWidget.ui    
    editors/SessionTypeWidget.ui
    editors/SessionWidget.ui
    editors/ServiceWidget.ui
    editors/ServiceConfigWidget.ui
    editors/SiteWidget.ui
    editors/TeraForm.ui
    editors/TestTypeWidget.ui
    editors/UserGroupWidget.ui
    editors/UserSummaryWidget.ui
    editors/UserWidget.ui
    # Services
    services/VideoRehabService/VideoRehabWidget.ui
    services/VideoRehabService/VideoRehabSetupWidget.ui
    services/VideoRehabService/VideoRehabToolsWidget.ui
    services/VideoRehabService/VideoRehabVirtualCamSetupDialog.ui
    services/VideoRehabService/VideoRehabPTZDialog.ui
    services/DanceService/DanceConfigWidget.ui
    # Widgets
    widgets/ProjectNavigator.ui
    widgets/notification.ui
    widgets/ConfigWidget.ui
    widgets/InSessionWidget.ui
    widgets/SessionInviteWidget.ui
    widgets/OnlineManagerWidget.ui
    widgets/DashboardWidget.ui
    widgets/AssetsWidget.ui
    widgets/ResultMessageWidget.ui
    widgets/TestsWidget.ui
    widgets/LogViewWidget.ui
    widgets/SessionsListWidget.ui
    # Kits
    kit/KitConfigDialog.ui
    kit/KitInSessionDialog.ui
    kit/KitVideoRehabWidget.ui
)

SET(qrcs
    ${TERACLIENT_RES_INCLUDES}/TeraClient.qrc
    services/VideoRehabService/resources/VideoRehabService.qrc
)

#Generate .h files from the .ui files
QT5_WRAP_UI(moc_uis ${uis})

#This will generate moc_* for Qt
QT5_WRAP_CPP(moc_srcs ${headers})

# generate rules for building source files from the resources
QT5_ADD_RESOURCES(client_qrc ${qrcs})

set(translation_files_srcs
    ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_fr.ts
    ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_en.ts
)

#Translation files, this is done manually to avoid removing the file when doing make clean
# Add -noobsolete parameter to remove obsoleted translations
add_custom_target(openteraplus_en_ts
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -target-language en ${srcs} ${OPENTERA_SHARED_INCLUDES} ${headers} ${moc_uis} -ts ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_en.ts
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${moc_uis}
)

add_custom_target(openteraplus_fr_ts
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -target-language fr ${srcs} ${OPENTERA_SHARED_INCLUDES} ${headers} ${moc_uis} -ts ${TERACLIENT_RES_INCLUDES}/translations/openteraplus_fr.ts
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
set(MACOSX_BUNDLE_INFO_STRING "OpenTeraPlus")
set(MACOSX_BUNDLE_ICON_FILE "OpenTeraPlus.icns")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "OpenTeraPlus")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_NAME "OpenTeraPlus")
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
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/OpenTeraPlus.app/Contents/bin
        INSTALL_RPATH_USE_LINK_PATH TRUE)

# Automatic generation of ts files on target
# Remove if you don't want to generate translations every time.
# add_dependencies(TeraClient translations)

add_custom_target(plist_mac SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)

# Set a custom plist file for the app bundle
set_target_properties(OpenTeraPlus PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)

#This will copy icon to Resources directory
set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

if (APPLE)
    # Mac bundles...
    # Install targets
    install(TARGETS OpenTeraPlus DESTINATION .)
    # install(TARGETS opentera_messages DESTINATION .)

else(APPLE)
    # Every other systems
    # Install target to bin
    install(TARGETS OpenTeraPlus DESTINATION bin)
    install(TARGETS opentera_messages DESTINATION bin)
endif(APPLE)



