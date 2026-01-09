# Edit Measure Analysis Plugin Project File
# Provides measurement and analysis editing capabilities

include(../../shared.pri)

TEMPLATE = lib
DEFINES += EDITMEASUREANAPLUGIN_LIBRARY
CONFIG += c++11
QT += gui

# Source files
SOURCES += editMeasureAnaPlugin.cpp \
           editMeasurePluginFactory.cpp

HEADERS += EditMeasureAnaPlugin_global.h \
           editMeasureAnaPlugin.h \
           editMeasurePluginFactory.h

# Translation files
TRANSLATIONS += EditMeasureAnaPlugin_zh_CN.ts \
                EditMeasureAnaPlugin_en.ts

# Resources
RESOURCES += editMeasureAnaPlugin.qrc

# Unix installation path
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
