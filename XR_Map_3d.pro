QT += core gui quick widgets quickwidgets network gui-private qml sql concurrent positioning quickcontrols2 \
        bluetooth serialport svg

CONFIG += c++17
CONFIG += debug
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
TARGET = XR-Viewer

# INCLUDEPATH += $$PWD/quazip
# LIBS += -L$$PWD/lib -lquazipd

#暂时注释
# win32:CONFIG(release, debug|release):    LIBS += -L$$PWD/quazip-msvc2019/lib/ -lquazip
# else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/quazip-msvc2019/lib/ -lquazipd

INCLUDEPATH += $$PWD/quazip-msvc2019/include
DEPENDPATH  += $$PWD/quazip-msvc2019/include


DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    src/console.h \
    src/console_list_model.h \
    src/core.h \
    src/dataset.h \
    src/dataset_defs.h \
    src/data_interpolator.h \
    src/data_horizon.h \
    src/dsp_defs.h \
    src/epoch.h \
    src/hotkeys_manager.h \
    src/id_binnary.h \
    src/delaunay.h \
    src/delaunay_defs.h \
    src/location.h \
    src/logger.h \
    src/map_defs.h \
    src/isobaths_defs.h \
    src/math_defs.h \
    src/themes.h \
    src/tsl3.h \
    src/tslw.h \


### SOURCES
SOURCES += \
    src/console.cpp \
    src/console_list_model.cpp \
    src/core.cpp \
    src/dataset.cpp \
    src/data_interpolator.cpp \
    src/data_horizon.cpp \
    src/epoch.cpp \
    src/hotkeys_manager.cpp \
    src/location.cpp \
    src/logger.cpp \
    src/main.cpp \
    src/map_defs.cpp \
    src/themes.cpp

FLASHER {
    DEFINES += FLASHER
    SOURCES += src/flasher/deviceflasher.cpp
    HEADERS += src/flasher/deviceflasher.h
    SOURCES += src/flasher/flasher.cpp
    HEADERS += src/flasher/flasher.h
}

TRANSLATIONS += \
    translations/translation_ch.ts \
    translations/translation_en.ts

RESOURCES += \
    qml/qml.qrc \
    resources/icons.qrc \
    resources/resources.qrc \

windows {
    message("Building for Windows with full OpenGL")
    LIBS += -lopengl32
    RC_FILE += resources/file.rc
    RESOURCES += shaders/windows/shaders.qrc
}


QML_IMPORT_PATH = $$PWD/qml
QML_DESIGNER_IMPORT_PATH = $$PWD/qml


DISTFILES += \
    android/src/.DS_Store \
    android/src/com/.DS_Store \
    android/src/com/nqc/Config.java \
    android/src/com/nqc/FileQtActivity.java \
    qml/Common/MenuBlockEx.qml \
    qml/Scene3DToolbar.qml \
    qml/SceneObjectsControlBar/ActiveObjectParams.qml \
    qml/SceneObjectsControlBar/BottomTrackParams.qml \
    qml/SceneObjectsControlBar/SceneObjectsControlBar.qml \
    qml/SceneObjectsControlBar/SceneObjectsList.qml \
    qml/SceneObjectsControlBar/SceneObjectsListDelegate.qml \
    qml/SceneObjectsList.qml \
    qml/SceneObjectsListDelegate.qml \
    qml/BackStyle.qml \
    qml/ButtonBackStyle.qml \
    qml/CButton.qml \
    qml/CCombo.qml \
    qml/CComboBox.qml \
    qml/CSlider.qml \
    qml/ComboBackStyle.qml \
    qml/ConnectionViewer.qml \
    qml/Console.qml \
    qml/CustomGroupBox.qml \
    qml/DeviceSettingsViewer.qml \
    qml/MenuBar.qml \
    qml/MenuFrame.qml \
    qml/MenuButton.qml \
    qml/MenuViewer.qml \
    qml/TabBackStyle.qml \
    qml/UpgradeBox.qml \
    qml/FlashBox.qml \
    qml/main.qml




INCLUDEPATH += $$PWD/src

# Module includes
include($$PWD/src/data_processor/data_processor.pri)
include($$PWD/src/scene2d/scene2d.pri)
include($$PWD/src/scene3d/scene3d.pri)
include($$PWD/src/device/device.pri)
include($$PWD/src/tile_engine/tile_engine.pri)



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/res/values/libs.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android


android {
    QT += androidextras

    include($$PWD/android_openssl-master/openssl.pri)

    message("Building for Android (ARM) with OpenGL ES")
    RESOURCES += shaders/android/shaders.qrc

SOURCES += \
    $$PWD/quazip-master/quazip/JlCompress.cpp \
    $$PWD/quazip-master/quazip/qioapi.cpp \
    $$PWD/quazip-master/quazip/quachecksum32.cpp \
    $$PWD/quazip-master/quazip/quaadler32.cpp \
    $$PWD/quazip-master/quazip/quacrc32.cpp \
    $$PWD/quazip-master/quazip/quagzipfile.cpp \
    $$PWD/quazip-master/quazip/quaziodevice.cpp \
    $$PWD/quazip-master/quazip/quazip.cpp \
    # $$PWD/quazip-master/quazip/quazip_textcodec.cpp \
    $$PWD/quazip-master/quazip/quazipdir.cpp \
    $$PWD/quazip-master/quazip/quazipfile.cpp \
    $$PWD/quazip-master/quazip/quazipfileinfo.cpp \
    $$PWD/quazip-master/quazip/quazipnewinfo.cpp\
    $$PWD/quazip-master/quazip/unzip.c \
    $$PWD/quazip-master/quazip/zip.c \

HEADERS += \
    $$PWD/quazip-master/quazip/ioapi.h \
    $$PWD/quazip-master/quazip/minizip_crypt.h \
    $$PWD/quazip-master/quazip/JlCompress.h \
    $$PWD/quazip-master/quazip/quaadler32.h \
    $$PWD/quazip-master/quazip/quachecksum32.h \
    $$PWD/quazip-master/quazip/quacrc32.h \
    $$PWD/quazip-master/quazip/quagzipfile.h \
    $$PWD/quazip-master/quazip/quaziodevice.h \
    $$PWD/quazip-master/quazip/quazip.h \
    $$PWD/quazip-master/quazip/quazip_global.h \
    $$PWD/quazip-master/quazip/quazip_qt_compat.h \
    # $$PWD/quazip-master/quazip/quazip_textcodec.h \
    $$PWD/quazip-master/quazip/quazipfile.h \
    $$PWD/quazip-master/quazip/quazipfileinfo.h \
    $$PWD/quazip-master/quazip/quazipnewinfo.h \
    $$PWD/quazip-master/quazip/unzip.h \
    $$PWD/quazip-master/quazip/zip.h \
}
