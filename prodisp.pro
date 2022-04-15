QT += core gui widgets
TARGET = prodisp
QMAKE_CXXFLAGS += -std=c++11 -fPIC

COMMIT_NUMBER = 1
win32: COMPILE_DATE = $$system(date /T)
unix: COMPILE_DATE = $$system(date +%d.%m.%Y)
VERSION = $$sprintf("%1.%2.%3.%4", $$section(COMPILE_DATE, ., 2, 2), $$section(COMPILE_DATE, ., 1, 1), $$section(COMPILE_DATE, ., 0, 0), $$COMMIT_NUMBER)

CONFIG(release, debug|release){
    DESTDIR = release
    MOC_DIR = release/moc
    OBJECTS_DIR = release/obj
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    DESTDIR = debug
    MOC_DIR = debug/moc
    OBJECTS_DIR = debug/obj
}

DEFINES += PRODISPVERSION=\\\"$$VERSION\\\"

INCLUDEPATH += \
        3rdparty/ \
        3rdparty/jsoncons \
        3rdparty/nonstd

SOURCES += \
    application.cpp \
    functions.cpp \
    main.cpp \
    mainwindow.cpp \
    task.cpp \
    taskexecutor.cpp \
    taskscreator.cpp

HEADERS += \
    application.h \
    functions.h \
    mainwindow.h \
    task.h \
    taskexecutor.h \
    taskinfo.h \
    taskscreator.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    images.qrc
