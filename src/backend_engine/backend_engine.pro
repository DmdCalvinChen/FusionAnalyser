include(../general.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
DESTDIR = ../../orthodontic-analyzer

# Disable Qt deprecation errors
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000

INCLUDEPATH += . \
               .. \
               ../.. \
               $$VCGDIR \
               $$EIGENDIR \
               $$GLEWDIR/include \
               $$SPDLOG/include \
               $$COMMON_BASE \
               $$COMMON_EXT

DEPENDPATH += $$VCGDIR \
              $$VCGDIR/vcg \
              $$VCGDIR/wrap

SOURCES += main.cpp

QT -= gui
QT += core network xml gui widgets opengl svg script

# Core libraries
COMMONLIB = -lcommon -lcommon_base -lcommon_ext -lGLU

macx:LIBS += -L../distrib -lcommon -lcommon_base -lcommon_ext
linux:LIBS += -L../distrib -lcommon -lcommon_base -lcommon_ext -lGLU

macx:QMAKE_POST_LINK = "install_name_tool -change libcommon.1.dylib @executable_path/../src/distrib/libcommon.1.dylib ../../orthodontic-analyzer/backend_engine; install_name_tool -change libcommon_base.1.dylib @executable_path/../src/distrib/libcommon_base.1.dylib ../../orthodontic-analyzer/backend_engine; install_name_tool -change libcommon_ext.1.dylib @executable_path/../src/distrib/libcommon_ext.1.dylib ../../orthodontic-analyzer/backend_engine"
linux:QMAKE_POST_LINK = "patchelf --set-rpath \\$$ORIGIN/../src/distrib ../../orthodontic-analyzer/backend_engine"
