find_package(Qt5Core REQUIRED)
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5Multimedia  REQUIRED)

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
    wizards/UserWizard.h
    wizards/BaseWizard.h
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
    wizards/UserWizard.cpp
    wizards/BaseWizard.cpp
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

include_directories(
    ${TERACLIENT_RES_INCLUDES}
    ${OPENTERA_MESSAGES_INCLUDES}
    ${OPENTERA_SHARED_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/editors
    ${CMAKE_CURRENT_BINARY_DIR})

# Application icon
SET(icon openteraico.rc)

# create an executable file named "TeraClient" from the source files
add_executable(TeraClient MACOSX_BUNDLE WIN32 ${icon} ${srcs} ${headers} ${moc_srcs} ${client_qrc} ${moc_uis})
qt5_use_modules(TeraClient Core Network Multimedia WebSockets WebEngine WebEngineWidgets WebSockets)

# Linking with Qt libraries and others
target_link_libraries(TeraClient ${OPENTERA_SHARED_LIBS})
set_target_properties(TeraClient PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/bin
        INSTALL_RPATH_USE_LINK_PATH TRUE)

# Install target
install(TARGETS TeraClient DESTINATION bin)




