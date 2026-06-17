INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD

HEADERS *= \
    $$PWD/dev_driver.h\
    # $$PWD/dev_q_property.h \
    $$PWD/device_manager.h \
    $$PWD/device_manager_wrapper.h \
    $$PWD/blemanager.h \
    $$PWD/tmodem.h \
    $$PWD/udpmanager.h \
    $$PWD/minilzo.h \
    $$PWD/lzoconf.h \
    $$PWD/lzodefs.h


SOURCES *= \
    $$PWD/dev_driver.cpp \
    $$PWD/device_manager.cpp \
    $$PWD/device_manager_wrapper.cpp \
    $$PWD/blemanager.cpp \
    $$PWD/udpmanager.cpp \
    $$PWD/minilzo.c
