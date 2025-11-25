INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/mygl.cpp \
    $$PWD/scene/crosshair.cpp \
    $$PWD/scene/noise.cpp \
    $$PWD/scene/world.cpp \
    $$PWD/shaderprogram.cpp \
    $$PWD/drawable.cpp \
    $$PWD/cameracontrolshelp.cpp \
    $$PWD/scene/cube.cpp \
    $$PWD/openglcontext.cpp \
    $$PWD/scene/terrain.cpp \
    $$PWD/scene/worldaxes.cpp \
    $$PWD/scene/entity.cpp \
    $$PWD/scene/player.cpp \
    $$PWD/scene/camera.cpp \
    $$PWD/playerinfo.cpp \
    $$PWD/scene/chunk.cpp \
    $$PWD/texture.cpp \
    $$PWD/framebuffer.cpp \
    $$PWD/scene/quad.cpp

HEADERS += \
    $$PWD/constants.h \
    $$PWD/debug.h \
    $$PWD/mainwindow.h \
    $$PWD/mygl.h \
    $$PWD/scene/crosshair.h \
    $$PWD/scene/noise.h \
    $$PWD/scene/world.h \
    $$PWD/shaderprogram.h \
    $$PWD/drawable.h \
    $$PWD/cameracontrolshelp.h \
    $$PWD/scene/cube.h \
    $$PWD/openglcontext.h \
    $$PWD/scene/terrain.h \
    $$PWD/scene/worldaxes.h \
    $$PWD/smartpointerhelp.h \
    $$PWD/glm_includes.h \
    $$PWD/scene/entity.h \
    $$PWD/scene/player.h \
    $$PWD/scene/camera.h \
    $$PWD/playerinfo.h \
    $$PWD/scene/chunk.h \
    $$PWD/texture.h \
    $$PWD/framebuffer.h \
    $$PWD/scene/quad.h

RESOURCES += \
    $$PWD/../glsl.qrc
