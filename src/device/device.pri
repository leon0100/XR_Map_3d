INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD

HEADERS *= \
    $$PWD/device_manager.h \
    $$PWD/blemanager.h \
    $$PWD/serialportmanager.h \
    $$PWD/tmodem.h \
    $$PWD/tsly.h \
    $$PWD/udpmanager.h \
    $$PWD/minilzo.h \
    $$PWD/lzoconf.h \
    $$PWD/lzodefs.h \
    $$PWD/tsl3.h    \
    $$PWD/tslw.h    \


SOURCES *= \
    $$PWD/device_manager.cpp \
    $$PWD/blemanager.cpp \
    $$PWD/serialportmanager.cpp \
    $$PWD/udpmanager.cpp \
    $$PWD/minilzo.c
