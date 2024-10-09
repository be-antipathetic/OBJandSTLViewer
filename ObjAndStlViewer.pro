QT += core gui opengl widgets

CONFIG += c++17

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
win32: LIBS += -lglu32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32: LIBS += -L$$PWD/freeglut/lib/x64/ -lfreeglut

INCLUDEPATH += $$PWD/freeglut/include/GL
DEPENDPATH += $$PWD/freeglut/include/GL

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/freeglut/lib/x64/freeglut.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/freeglut/lib/x64/libfreeglut.a
