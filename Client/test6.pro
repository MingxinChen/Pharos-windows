#-------------------------------------------------
#
# Project created by QtCreator 2018-01-06T15:12:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test6
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
    function/connectioncs.cpp \
    function/system.cpp \
    interface/addchatcell.cpp \
    interface/addfriendcell.cpp \
    interface/addresslistitem.cpp \
    interface/chatcell.cpp \
    interface/log.cpp \
    interface/mainwindow.cpp \
    interface/reg.cpp \
    interface/settingcell.cpp \
    interface/userinfocell.cpp \
    dataType/ChatInfo.cpp \
    dataType/ClientInfo.cpp \
    dataType/PeerList.cpp \
    dataType/FriendInfo.cpp \
    interface/informcell.cpp

HEADERS  += \
    function/connectioncs.h \
    function/system.h \
    interface/addchatcell.h \
    interface/addfriendcell.h \
    interface/addresslistitem.h \
    interface/chatcell.h \
    interface/log.h \
    interface/mainwindow.h \
    interface/reg.h \
    interface/settingcell.h \
    interface/userinfocell.h \
    dataType/ChatInfo.h \
    dataType/ClientInfo.h \
    dataType/CommonDefine.h \
    dataType/PeerList.h \
    dataType/FriendInfo.h \
    interface/informcell.h

FORMS    += \
    interface/addchatcell.ui \
    interface/addfriendcell.ui \
    interface/addresslistitem.ui \
    interface/chatcell.ui \
    interface/log.ui \
    interface/mainwindow.ui \
    interface/reg.ui \
    interface/settingcell.ui \
    interface/userinfocell.ui \
    interface/informcell.ui

RESOURCES += \
    images.qrc

LIBS += -lws2_32

RC_FILE += icon.rc

# QMAKE_LFLAGS =- static
