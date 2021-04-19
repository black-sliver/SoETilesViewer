QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
#CONFIG += static # static QT build environment required

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    finddialog.cpp \
    frameview.cpp \
    main.cpp \
    mainwindow.cpp \
    scriptparser.cpp \
    tileview.cpp \
    util.cpp

HEADERS += \
    abstracttile.h \
    characterdata.h \
    colormap.h \
    finddialog.h \
    frameview.h \
    mainwindow.h \
    rom.h \
    scriptparser.h \
    spriteblock.h \
    spriteinfo.h \
    tile.h \
    tileview.h \
    util.h \
    SoEScriptDumper/data.h \
    SoEScriptDumper/list-rooms.cpp

FORMS += \
    finddialog.ui \
    mainwindow.ui

RC_ICONS = icon.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    mainwindow.qrc

OTHER_FILES += \
    setup.iss

DISTFILES += \
    README.md

DEFINES += QUAZIP_STATIC
include(zlib-min-q/zlib.pri)
include(quazip/quazip.pri)
