# Decorate Fusion Plugin Project File
# Provides decoration and visualization features for dental models

include(../../shared.pri)

TARGET = decorateFusionPlugin

# macOS library dependencies
macx:LIBS += -L../../distrib -lcommon -lcommon_base -lUI_Common
mac:DESTDIR = ../../distrib/plugins

# Source files
HEADERS += decoratefusionplugin.h

SOURCES += decoratefusionplugin.cpp

# Include paths
INCLUDEPATH += $$COMMON_BASE

# Translation files
TRANSLATIONS += decorateFusionPlugin_ch.ts
TRANSLATIONS += decorateFusionPlugin_en.ts

# Resources
RESOURCES += decorateFusionPlugin.qrc
