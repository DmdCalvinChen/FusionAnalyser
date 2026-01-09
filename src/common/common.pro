# Common Library Project File
# Core library providing mesh handling, plugin management, and basic utilities

include(../general.pri)

# External library paths
EXIF_DIR = ../external/jhead-2.95
GLEWCODE = $$GLEWDIR/src/glew.c

# Output directory configuration for Windows
CONFIG(debug, debug|release) {
    win32-msvc2015:DESTDIR = ../distribD
    win32-msvc:DESTDIR = ../distribD
} else {
    win32-msvc2015:DESTDIR = ../distrib
    win32-msvc:DESTDIR = ../distrib
}

win32-g++:DLLDESTDIR = ../distrib
mac:DESTDIR = ../distrib

# Clean old plugin libraries to avoid conflicts
macx:QMAKE_CLEAN += ../distrib/plugins/*.dylib
win32:QMAKE_CLEAN += ../distrib/plugins/*.dll
linux-g++:QMAKE_CLEAN += ../distrib/plugins/*.so

# Include paths
INCLUDEPATH += ../.. \
               ../ \
               $$VCGDIR \
               $$EIGENDIR \
               $$GLEWDIR/include \
               $$SPDLOG/include \
               $$COMMON_BASE \
               $$UI_COMMONDIR \
               . \
               $$EXIF_DIR

DEPENDPATH += .

# Library template and platform-specific configuration
TEMPLATE = lib
TARGET = common

# Linux configuration
linux-g++:CONFIG += dll
linux-g++:DESTDIR = ../distrib
linux-g++-32:CONFIG += dll
linux-g++-32:DESTDIR = ../distrib
linux-g++-64:CONFIG += dll
linux-g++-64:DESTDIR = ../distrib
linux-g++:QMAKE_CXXFLAGS += -Wno-unknown-pragmas

# Windows static library configuration
win32-msvc:CONFIG += staticlib

# macOS library dependencies
macx:LIBS += -L../distrib -L../distrib/plugins -lcommon_base

# Required Qt modules
QT += opengl xml xmlpatterns script

# Defines
DEFINES += GLEW_STATIC
win32-msvc:DEFINES += _CRT_SECURE_NO_WARNINGS

# Input
HEADERS += 	filterparameter.h \
			filterscript.h \
			GLLogStream.h \
			interfaces.h \
			ml_mesh_type.h \
			meshmodel.h \
			pluginmanager.h \
			scriptinterface.h \
			xmlfilterinfo.h \
			mlexception.h \
			mlapplication.h \
			scriptsyntax.h \	
			ml_shared_data_context.h \
                        planeview.h \
                        config.h \
			$$VCGDIR/wrap/gui/trackball.h \
            $$VCGDIR/wrap/gui/trackmode.h \
                        ml_selection_buffers.h

			
SOURCES += 	filterparameter.cpp \
			interfaces.cpp \
			filterscript.cpp \
			GLLogStream.cpp \
			$$GLEWCODE\
			meshmodel.cpp \
			pluginmanager.cpp \
			scriptinterface.cpp \
			xmlfilterinfo.cpp \
			mlapplication.cpp \
			scriptsyntax.cpp \
			searcher.cpp \
			ml_shared_data_context.cpp \
                        planeview.cpp \
			$$VCGDIR/wrap/gui/trackball.cpp \
            $$VCGDIR/wrap/gui/trackmode.cpp \
                        ml_selection_buffers.cpp
		
TRANSLATIONS += common_ch.ts
TRANSLATIONS += common_en.ts

RESOURCES += common.qrc
