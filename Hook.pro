#-------------------------------------------------
#
# Project created by QtCreator 2017-02-14T13:51:52
#
#-------------------------------------------------

QT       += core gui macextras
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Hook

TEMPLATE = app

DEFINES += PRO_FILE_PWD=$$sprintf("\"\\\"%1\\\"\"", $$_PRO_FILE_PWD_)

CONFIG += c++11

SOURCES += main.cpp\
    mainwindow.cpp \
    Hook.cpp \
    Event.cpp \
    Client.cpp \
    DeviceHook.cpp \
    jsonclient.cpp

HEADERS  += mainwindow.h \
    Hook.h \
    Event.h \
    Client.h \
    DeviceHook.h \
    jsonclient.h

FORMS    += mainwindow.ui

OBJECTIVE_SOURCES += \
                mac_helper.mm

OTHER_FILES += \
    KeyCodes.txt \
    Problems.txt \
    Heap.txt \
    UsefulLinks.txt \
    HookOld.txt \
    About.txt

DISTFILES += \
    keyCodesOSX.txt
