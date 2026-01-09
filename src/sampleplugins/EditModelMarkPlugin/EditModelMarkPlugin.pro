# Edit Model Mark Plugin Project File
# Provides model marking and feature annotation capabilities

include(../../shared.pri)

TEMPLATE = lib
DEFINES += EDITMODELMARK_LIBRARY
CONFIG += c++11
QT += gui

# Source files
SOURCES += editModelMarkFactory.cpp \
           editModelMarkPlugin.cpp \
           ui/FeatureMarkGui/featuremarkgui.cpp

HEADERS += EditModelMark_global.h \
           editModelMarkFactory.h \
           editModelMarkPlugin.h \
           ui/FeatureMarkGui/featuremarkgui.h

# UI Forms
FORMS = ui/FeatureMarkGui/featuremarkgui.ui

# Resources
RESOURCES += editModelMarkPlugin.qrc

# Translation files
TRANSLATIONS += EditModelMarkPlugin_zh_CN.ts \
                EditModelMarkPlugin_en.ts

# Unix installation path
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
