# FusionAnalyser Main Application Project File
# The main executable application

include(../general.pri)

# Output directory configuration
CONFIG(debug, debug|release) {
    DESTDIR = ../distribD
} else {
    DESTDIR = ../distrib
}

# External library paths
EXIF_DIR = ../external/jhead-2.95

# Disable Qt deprecation errors (allow deprecated APIs)
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000

# Translation files
TRANSLATIONS += FusionAnalyser_en.ts
TRANSLATIONS += FusionAnalyser_ch.ts

# Include paths
INCLUDEPATH += . \
               .. \
               ../.. \
               $$VCGDIR \
               $$EIGENDIR \
               $$GLEWDIR/include \
               $$EXIF_DIR \
               $$SPDLOG/include \
               $$BREAKCHECK/include \
               $$COMMON_BASE \
               $$COMMON_EXT \
               $$UI_COMMONDIR

DEPENDPATH += $$VCGDIR \
              $$VCGDIR/vcg \
              $$VCGDIR/wrap

# Source files
HEADERS = mainwindow.h \
          glarea.h \
          multiViewer_Container.h \
          glarea_setting.h \
          saveSnapshotDialog.h \
          savemaskexporter.h \
          stdpardialog.h \
          additionalgui.h \
          snapshotsetting.h \
          ml_render_gui.h \
          ml_rendering_actions.h \
          ml_default_decorators.h \
          $$VCGDIR/wrap/gl/trimesh.h \
          fileimportdlg.h \
          colortable.h \
          patientinfo.h \
          shaderStructs.h \
          meshshaderrender.h \
          toolBar/fusionAlignToolBar.h

SOURCES = main.cpp \
          mainwindow_Init.cpp \
          mainwindow_RunTime.cpp \
          glarea.cpp \
          multiViewer_Container.cpp \
          saveSnapshotDialog.cpp \
          savemaskexporter.cpp \
          stdpardialog.cpp \
          additionalgui.cpp \
          ml_render_gui.cpp \
          ml_rendering_actions.cpp \
          ml_default_decorators.cpp \
          $$VCGDIR/wrap/gui/coordinateframe.cpp \
          $$GLEWDIR/src/glew.c \
          glarea_setting.cpp \
          fileimportdlg.cpp \
          colortable.cpp \
          meshshaderrender.cpp \
          toolBar/fusionAlignToolBar.cpp
		
    
# UI Forms
FORMS = ui/savesnapshotDialog.ui \
        ui/savemaskexporter.ui \
        ui/congratsDialog.ui \
        ui/fileimportdlg.ui \
        ui/colortable.ui

# Resources
RESOURCES += FusionAnalyser.qrc

# Platform-specific resources
win32:RC_FILE = FusionAnalyser.rc
QMAKE_INFO_PLIST = ../install/info.plist
ICON = images/meshlab.icns

# Required Qt modules
QT += opengl xml xmlpatterns network script printsupport

# Defines
win32:DEFINES += NOMINMAX
win32-msvc:DEFINES += GLEW_STATIC
win32-msvc:DEFINES += _CRT_SECURE_NO_DEPRECATE
CONFIG += stl

# Windows system libraries
win32-msvc:LIBS += Gdi32.lib User32.lib Advapi32.lib

# Library dependencies
EXTERNALLIB = -lopengl32 -lGLU32
COMMONLIB = -lcommon -lcommon_base -lcommon_ext -lUI_common

# macOS library configuration
macx:LIBS += -L../distrib -lcommon -lcommon_base -lcommon_ext -lUI_Common
macx:QMAKE_POST_LINK = "cp -P ../distrib/libcommon.1.dylib ../distrib/FusionAnalyser.app/Contents/MacOS; install_name_tool -change libcommon.1.dylib @executable_path/libcommon.1.dylib ../distrib/FusionAnalyser.app/Contents/MacOS/FusionAnalyser"

# Windows library configuration
CONFIG(debug, debug|release) {
    win32-msvc2015:LIBS += -L../distribD -lcommon -lopengl32 -lGLU32 -lUI_common
    win32-msvc:LIBS += $$EXTERNALLIB -L../distribD $$COMMONLIB
} else {
    win32-msvc2015:LIBS += -L../distrib -lcommon -lopengl32 -lGLU32 -lUI_common
    win32-msvc:LIBS += $$EXTERNALLIB -L../distrib $$COMMONLIB
}

win32-msvc:LIBS += -L../distrib -lcommon -lopengl32 -lGLU32
win32-g++:LIBS += -L../distrib -lcommon -lopengl32 -lGLU32

# Linux library configuration
linux-g++:LIBS += -L../distrib -lcommon -lGLU
linux-g++:QMAKE_RPATHDIR += ../distrib
linux-g++-32:LIBS += -L../distrib -lcommon -lGLU
linux-g++-32:QMAKE_RPATHDIR += ../distrib
linux-g++-64:LIBS += -L../distrib -lcommon -lGLU
linux-g++-64:QMAKE_RPATHDIR += ../distrib

# GCC-specific defines
win32-g++:DEFINES += NDEBUG
CONFIG(debug, debug|release):win32-g++:release:DEFINES -= NDEBUG
