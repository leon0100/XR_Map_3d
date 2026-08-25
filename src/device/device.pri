INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD

HEADERS *= \
    $$PWD/device_manager.h \
    $$PWD/device_manager_wrapper.h \
    $$PWD/blemanager.h \
    $$PWD/serialportmanager.h \
    $$PWD/tmodem.h \
    $$PWD/udpmanager.h \
    $$PWD/minilzo.h \
    $$PWD/lzoconf.h \
    $$PWD/lzodefs.h


SOURCES *= \
    $$PWD/device_manager.cpp \
    $$PWD/device_manager_wrapper.cpp \
    $$PWD/blemanager.cpp \
    $$PWD/serialportmanager.cpp \
    $$PWD/udpmanager.cpp \
    $$PWD/minilzo.c
