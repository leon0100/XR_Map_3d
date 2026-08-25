INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD

HEADERS += \
    $$PWD/abstract_entity_data_filter.h \
    $$PWD/draw_utils.h \
    $$PWD/ray.h \
    $$PWD/ray_caster.h \
    $$PWD/text_renderer.h

SOURCES += \
    $$PWD/draw_utils.cpp \
    $$PWD/ray.cpp \
    $$PWD/ray_caster.cpp \
    $$PWD/text_renderer.cpp
