# Common Base Library Project File
# Provides base utilities, signal management, and logging functionality

include(../general.pri)

# Output directory configuration
CONFIG(debug, debug|release) {
    win32-msvc:DESTDIR = ../distribD
} else {
    win32-msvc:DESTDIR = ../distrib
}

mac:DESTDIR = ../distrib

# Library configuration
TARGET = common_base
TEMPLATE = lib
QT += widgets

# Include paths
INCLUDEPATH *= $$SPDLOG/include ../

# Library export define
DEFINES += COMMON_BASE_LIBRARY

# Qt deprecation warnings
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        common_base.cpp \
		SignalManager.cpp \
    logsingleton.cpp \
    util/fusionMessageBox.cpp \
    util/uitools.cpp

HEADERS += \
        common_base.h \
        common_base_global.h \
		SignalManager.h \
    logsingleton.h \
    util/fusionMessageBox.h \
    util/uitools.h
		
TRANSLATIONS += common_base_ch.ts
TRANSLATIONS += common_base_en.ts

RESOURCES += common_base.qrc

unix {
    target.path = /usr/local/lib
    INSTALLS += target
}
