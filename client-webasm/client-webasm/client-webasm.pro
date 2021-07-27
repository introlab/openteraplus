QT       += core gui network widgets websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += WEBASSEMBLY_COMPILATION=1
DEFINES += OPENTERAPLUS_VERSION_MAJOR=\\\"1\\\"
DEFINES += OPENTERAPLUS_VERSION_MINOR=\\\"0\\\"
DEFINES += OPENTERAPLUS_VERSION_PATCH=\\\"3\\\"
DEFINES += OPENTERAPLUS_VERSION=\\\"1.0.3\\\"
DEFINES += OPENTERAPLUS_CLIENT_NAME=\\\"OPENTERAPLUS\\\"


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ../../client/src/DownloadedFile.cpp \
    ../../client/src/managers/ComManager.cpp \
    ../../client/src/managers/ConfigManagerClient.cpp \
    ../../client/src/managers/ParticipantComManager.cpp \
    ../../client/src/managers/WebSocketManager.cpp \
    ../../shared/src/Utils.cpp \
    ../../shared/src/Logger.cpp \
    ../../shared/src/ConfigManager.cpp \
    ../../shared/src/data/TeraData.cpp \
    ../../shared/src/data/TeraPreferences.cpp

HEADERS += \
    mainwindow.h \
    ../../client/src/DownloadedFile.h \
    ../../client/src/managers/ComManager.h \
    ../../client/src/managers/ConfigManagerClient.h \
    ../../client/src/managers/ParticipantComManager.h \
    ../../client/src/managers/WebSocketManager.h \
    ../../shared/src/Utils.h \
    ../../shared/src/Logger.h \
    ../../shared/src/ConfigManager.h \
    ../../shared/src/data/TeraData.h \
    ../../shared/src/data/TeraPreferences.h

FORMS += \
    mainwindow.ui


INCLUDEPATH = ../../client/src ../../client/src/managers ../../shared/src ../../shared/src/data

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
