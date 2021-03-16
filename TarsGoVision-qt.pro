TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lgxiapi \
        -lpthread

unix: PKGCONFIG += opencv
INCLUDEPATH += /usr/local/include \
               /usr/local/include/opencv \
               /usr/local/include/opencv2 \
               TarsGoVision/GxCamera/GxSDK \
               TarsGoVision/

LIBS += /usr/local/lib/libopencv_calib3d.so \
        /usr/local/lib/libopencv_core.so    \
        /usr/local/lib/libopencv_highgui.so \
        /usr/local/lib/libopencv_imgproc.so \
        /usr/local/lib/libopencv_imgcodecs.so\
        /usr/local/lib/libopencv_objdetect.so\
        /usr/local/lib/libopencv_photo.so \
        /usr/local/lib/libopencv_dnn.so \
        /usr/local/lib/libopencv_shape.so\
        /usr/local/lib/libopencv_features2d.so \
        /usr/local/lib/libopencv_stitching.so \
        /usr/local/lib/libopencv_flann.so\
        /usr/local/lib/libopencv_superres.so \
        /usr/local/lib/libopencv_videoio.so \
        /usr/local/lib/libopencv_video.so\
        /usr/local/lib/libopencv_videostab.so \
        /usr/local/lib/libopencv_ml.so

DISTFILES += \
    TarsGoVision/General/123svm.xml \
    TarsGoVision/General/camera_params.xml

HEADERS += \
    TarsGoVision/AngleSolver/AngleSolver.h \
    TarsGoVision/Armor/Armor.h \
    TarsGoVision/General/General.h \
    TarsGoVision/GxCamera/GxCamera.h \
    TarsGoVision/GxCamera/GxSDK/DxImageProc.h \
    TarsGoVision/GxCamera/GxSDK/GxIAPI.h

SOURCES += \
    TarsGoVision/AngleSolver/AngleSolver.cpp \
    TarsGoVision/Armor/ArmorBox.cpp \
    TarsGoVision/Armor/ArmorDetector.cpp \
    TarsGoVision/Armor/ArmorNumClassifier.cpp \
    TarsGoVision/Armor/LightBar.cpp \
    TarsGoVision/Armor/findLights.cpp \
    TarsGoVision/Armor/matchArmors.cpp \
    TarsGoVision/GxCamera/GxCamera.cpp \
    TarsGoVision/Main/ArmorDetecting.cpp \
    TarsGoVision/Main/ImageUpdating.cpp \
    TarsGoVision/Main/main.cpp
