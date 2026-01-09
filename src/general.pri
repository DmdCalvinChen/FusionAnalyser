# General Project Configuration File
# This file contains common settings used by all sub-projects in FusionAnalyser

# Main coordinate type - using float for performance
# Can be changed to double if higher precision is needed
DEFINES += MESHLAB_SCALAR=float

# External library paths
VCGDIR = ../../vcglib
EXTERNAL = ../external
EIGENDIR = $$VCGDIR/eigenlib
GLEWDIR = ../external/glew-2.1.0
SPDLOG = ../external/SpdLog
BREAKCHECK = ../external/break_check

# Internal library paths
UI_COMMONDIR = ../UI_Common
COMMON_BASE = ../common_base
COMMON_EXT = ../common_ext

# C++11 standard required
CONFIG += c++11

# macOS compiler flags
macx:QMAKE_CXXFLAGS += -Wno-inconsistent-missing-override
macx:CONFIG(release, debug|release):QMAKE_CXXFLAGS += -O3 -DNDEBUG
macx:CONFIG(debug, debug|release):QMAKE_CXXFLAGS += -O0 -g

# macOS OpenGL framework configuration
# Override Qt's default to avoid deprecated AGL framework (removed in macOS SDK 26)
macx:QMAKE_LIBS_OPENGL = -framework OpenGL
macx:QMAKE_LIBS_OPENGL_ES2 = -framework OpenGLES
macx:QMAKE_INCDIR_OPENGL = /System/Library/Frameworks/OpenGL.framework/Headers

MACLIBDIR = ../../external/lib/macx64

# Windows-specific defines
# NOMINMAX prevents Windows.h from defining min/max macros that conflict with std::min/max
win32:DEFINES += NOMINMAX

# Linux compiler flags
linux-g++:QMAKE_CXXFLAGS += -Wno-unknown-pragmas
