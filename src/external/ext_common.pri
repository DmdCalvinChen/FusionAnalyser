# External Libraries Common Configuration
# Shared settings for building external libraries

# Output directory configuration by platform and compiler
macx:DESTDIR = $$EXTERNAL_BASE_PATH/lib/macx64
win32-g++:DESTDIR = $$EXTERNAL_BASE_PATH/lib/win32-gcc
win32-msvc2005:DESTDIR = $$EXTERNAL_BASE_PATH/lib/win32-msvc2005
win32-msvc2008:DESTDIR = $$EXTERNAL_BASE_PATH/lib/win32-msvc2008
win32-msvc2010:DESTDIR = $$EXTERNAL_BASE_PATH/lib/win32-msvc2010
win32-msvc2012:DESTDIR = $$EXTERNAL_BASE_PATH/lib/win32-msvc2012
win32-msvc2013:DESTDIR = $$EXTERNAL_BASE_PATH/lib/win32-msvc2013
win32-msvc2015:DESTDIR = $$EXTERNAL_BASE_PATH/lib/win32-msvc2015
win32-msvc2017:DESTDIR = $$EXTERNAL_BASE_PATH/lib/win32-msvc2017
win32-msvc:DESTDIR = $$EXTERNAL_BASE_PATH/lib/win32-msvc
linux:DESTDIR = $$EXTERNAL_BASE_PATH/lib/linux

# Windows-specific defines
win32-msvc:DEFINES += _CRT_SECURE_NO_WARNINGS

# Release build configuration
unix:CONFIG(release, debug|release) {
    DEFINES *= NDEBUG
}

# Windows runtime library configuration
CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS_DEBUG += /MTd
}
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS_RELEASE += /MT
}