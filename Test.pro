QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    Test_vi_VN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /home/trang/Projects/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lgphoto2

INCLUDEPATH += $$PWD/../../sysroot/usr/include/gphoto2
DEPENDPATH += $$PWD/../../sysroot/usr/include/gphoto2

unix:!macx: PRE_TARGETDEPS += $$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/libgphoto2.a

unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lgphoto2_port

unix:!macx: PRE_TARGETDEPS += $$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/libgphoto2_port.a
