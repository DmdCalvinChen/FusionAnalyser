# Shared Configuration for Plugins
# This file is included by all plugin projects

include(general.pri)

# Adjust paths for plugin subdirectories
VCGDIR = ../$$VCGDIR
EIGENDIR = ../$$EIGENDIR

# Plugin configuration
TEMPLATE = lib
CONFIG += plugin

# Required Qt modules for plugins
QT += opengl xml xmlpatterns script svg

# macOS library dependencies and output directory
macx:LIBS += -L../../distrib -lcommon -lcommon_base -lcommon_ext -lUI_Common
macx:DESTDIR = ../../distrib/plugins

# Windows library dependencies
DEPENDLIBS = -lcommon -lcommon_base -lcommon_ext -lUI_Common -lopengl32 -lGLU32

CONFIG(debug, release | debug) {
    win32-msvc:LIBS += -L../../distribD $$DEPENDLIBS
} else {
    win32-msvc:LIBS += -L../../distrib $$DEPENDLIBS
}

win32-g++:LIBS += -L../../distrib -lcommon -lopengl32 -lGLU32

# Windows-specific defines
win32-msvc:DEFINES += GLEW_STATIC _USE_MATH_DEFINES

# Include paths for plugins
INCLUDEPATH += ../.. \
               $$VCGDIR \
               $$EIGENDIR \
               ../$$GLEWDIR/include \
               ../$$SPDLOG/include \
               ../$$UI_COMMONDIR \
               ../$$COMMON_BASE \
               ../$$COMMON_EXT \
               ../$$EXTERNAL \
               ../$$EXTERNAL/third_party

DEPENDPATH += ../.. $$VCGDIR

# Windows-specific defines to suppress deprecated function warnings
win32-msvc2013:DEFINES += _CRT_SECURE_NO_DEPRECATE
win32-msvc2015:DEFINES += _CRT_SECURE_NO_DEPRECATE
win32-msvc:DEFINES += _CRT_SECURE_NO_DEPRECATE
# Output directory configuration based on architecture and build type
if(contains(QT_ARCH, i386)) {
    CONFIG(debug, debug | release) {
        DESTDIR = ../../distribD/plugins_x86
    } else {
        DESTDIR = ../../distrib/plugins_x86
    }
} else {
    CONFIG(debug, debug | release) {
        DESTDIR = ../../distribD/plugins
    } else {
        DESTDIR = ../../distrib/plugins
    }
}

# Debug build target naming - append '_debug' or 'd' suffix
contains(TEMPLATE, lib) {
    CONFIG(debug, debug|release) {
        unix:TARGET = $$member(TARGET, 0)_debug
        else:TARGET = $$member(TARGET, 0)d
    }
}
