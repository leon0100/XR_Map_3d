QT += core gui quick widgets quickwidgets network gui-private qml sql concurrent positioning quickcontrols2 \
        bluetooth serialport  svg

CONFIG += c++17
CONFIG += debug
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    src/black_stripes_processor.h \
    src/console.h \
    src/console_list_model.h \
    src/converter_xtf.h \
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
    src/logger.h \
    src/map_defs.h \
    src/isobaths_defs.h \
    src/math_defs.h \
    src/mav_link_conf.h \
    src/proto_binnary.h \
    src/stream_list.h \
    src/stream_list_model.h \
    src/themes.h \
    src/tslw.h \
    src/xtf_conf.h \
    src/location_reader.h

### SOURCES
SOURCES += \
    src/black_stripes_processor.cpp \
    src/console.cpp \
    src/console_list_model.cpp \
    src/core.cpp \
    src/dataset.cpp \
    src/data_interpolator.cpp \
    src/data_horizon.cpp \
    src/epoch.cpp \
    src/hotkeys_manager.cpp \
    src/id_binnary.cpp \
    src/logger.cpp \
    src/main.cpp \
    src/map_defs.cpp \
    src/stream_list.cpp \
    src/stream_list_model.cpp \
    src/location_reader.cpp

FLASHER {
    DEFINES += FLASHER
    SOURCES += src/flasher/deviceflasher.cpp
    HEADERS += src/flasher/deviceflasher.h
    SOURCES += src/flasher/flasher.cpp
    HEADERS += src/flasher/flasher.h
}

TRANSLATIONS += \
    translations/translation_en.ts \
    translations/translation_ru.ts \
    translations/translation_pl.ts

RESOURCES += \
    qml/qml.qrc \
    resources/icons.qrc \

windows {
    message("Building for Windows with full OpenGL")
    LIBS += -lopengl32
    RESOURCES += shaders/windows/shaders.qrc
}


QML_IMPORT_PATH = $$PWD/qml
QML_DESIGNER_IMPORT_PATH = $$PWD/qml


DISTFILES += \
    qml/Common/MenuBlockEx.qml \
    qml/Scene3DToolbar.qml \
    qml/SceneObjectsControlBar/ActiveObjectParams.qml \
    qml/SceneObjectsControlBar/BottomTrackParams.qml \
    qml/SceneObjectsControlBar/SceneObjectsControlBar.qml \
    qml/SceneObjectsControlBar/SceneObjectsList.qml \
    qml/SceneObjectsControlBar/SceneObjectsListDelegate.qml \
    qml/SceneObjectsList.qml \
    qml/SceneObjectsListDelegate.qml \
    qml/AdjBox.qml \
    qml/AdjBoxBack.qml \
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
# DEPENDPATH += $$PWD/third_party/freetype/include

# Module includes
include($$PWD/src/data_processor/data_processor.pri)
include($$PWD/src/scene2d/scene2d.pri)
include($$PWD/src/scene3d/scene3d.pri)
include($$PWD/src/device/device.pri)
include($$PWD/src/link/link.pri)
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
}
