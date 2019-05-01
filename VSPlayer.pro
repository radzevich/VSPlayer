#-------------------------------------------------
#
# Project created by QtCreator 2019-03-25T21:42:22
#
#-------------------------------------------------

QT       += core gui \
            multimedia \
            multimediawidgets \
            widgets \
            concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VSPlayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    main.cpp \
    frontend/videowidget.cpp \
    frontend/player.cpp \
    frontend/playercontrols.cpp \
    frontend/playlistmodel.cpp \
    backend/audiosearchengine.cpp \
    app.cpp \
    backend/exceptions/audiosearchengineexception.cpp \
    backend/audiodecoder.cpp \
    backend/utilities/helpers/wavfilereader.cpp \
    backend/models/wavfile.cpp \
    backend/exceptions/audiodecoderexception.cpp

HEADERS += \
    frontend/videowidget.h \
    frontend/player.h \
    frontend/playercontrols.h \
    frontend/playlistmodel.h \
    backend/audiosearchengine.h \
    app.h \
    backend/exceptions/audiosearchengineexception.h \
    backend/audiodecoder.h \
    backend/utilities/helpers/wavfilereader.h \
    backend/models/wavfile.h \
    backend/exceptions/audiodecoderexception.h

FORMS += \
    frontend/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
