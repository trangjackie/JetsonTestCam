QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
QMAKE_LFLAGS += -Wl,-rpath,"'$$ORIGIN'"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    imagethread.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    alpr_utils.h \
    alpr_utils.h \
    imagethread.h \
    mainwindow.h \
    stb_image.h
    stb_image.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    Test_vi_VN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /home/trang/Projects/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lgphoto2
unix:!macx: PRE_TARGETDEPS += $$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/libgphoto2.a
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lgphoto2_port
unix:!macx: PRE_TARGETDEPS += $$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/libgphoto2_port.a

INCLUDEPATH += $$PWD/../../sysroot/usr/include/gphoto2
DEPENDPATH += $$PWD/../../sysroot/usr/include/gphoto2


unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_core
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_highgui
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_ccalib
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_imgcodecs
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_imgproc
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_cudafeatures2d
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_cudacodec
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_cudaimgproc
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_cudafilters
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_dnn
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_dnn_objdetect
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_dnn_superres
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_face
unix:!macx: LIBS += -L$$PWD/../../sysroot/usr/lib/aarch64-linux-gnu/ -lopencv_plot



INCLUDEPATH += $$PWD/../../sysroot/usr/include/opencv4
DEPENDPATH += $$PWD/../../sysroot/usr/include/opencv4

unix:!macx: LIBS += -L$$PWD/../../sysroot/sysroot/usr/lib/aarch64-linux-gnu/ -lultimate_alpr-sdk


INCLUDEPATH += $$PWD/../../sysroot/home/trang/Desktop/ultimateALPR-SDK/c++
DEPENDPATH += $$PWD/../../sysroot/home/trang/Desktop/ultimateALPR-SDK/c++
