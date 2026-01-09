# IO Base Plugin Project File
# Provides basic mesh file I/O functionality (STL, OBJ, PLY, OFF, PTX)

include(../../shared.pri)

TARGET = io_base
TEMPLATE = lib
DEFINES += IO_BASE_LIBRARY

# Source files
HEADERS += baseio.h \
           $$VCGDIR/wrap/io_trimesh/import_obj.h \
           $$VCGDIR/wrap/io_trimesh/import_off.h \
           $$VCGDIR/wrap/io_trimesh/import_ptx.h \
           $$VCGDIR/wrap/io_trimesh/import_stl.h \
           $$VCGDIR/wrap/io_trimesh/export_ply.h \
           $$VCGDIR/wrap/io_trimesh/export_obj.h \
           $$VCGDIR/wrap/io_trimesh/export_off.h \
           $$VCGDIR/wrap/ply/plylib.h \
           $$VCGDIR/wrap/io_trimesh/io_material.h

SOURCES += baseio.cpp \
           $$VCGDIR/wrap/ply/plylib.cpp

# macOS library dependencies
macx:LIBS += -L../../distrib -lcommon -lcommon_base
mac:DESTDIR = ../../distrib/plugins

# Unix installation path
unix {
    target.path = /usr/lib
    INSTALLS += target
}
