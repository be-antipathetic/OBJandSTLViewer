

QT       += core gui
QT       += core gui opengl
QT += opengl widgets core
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    model.cpp \
    openglscene.cpp \
    trackball.cpp

HEADERS += \
    mainwindow.h \
    model.h \
    openglscene.h \
    point3d.h \
    trackball.h

FORMS += \
    mainwindow.ui

unix|win32: LIBS += -lOpenGL32
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/freeglut/lib/x64/ -lfreeglut
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/freeglut/lib/x64/ -lfreeglut

INCLUDEPATH += $$PWD/freeglut/lib/x64
DEPENDPATH += $$PWD/freeglut/lib/x64
