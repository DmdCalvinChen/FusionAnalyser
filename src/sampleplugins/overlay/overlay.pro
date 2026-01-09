# Overlay Plugin Project File
# Provides overlay analysis and visualization for dental models

include(../../shared.pri)

TARGET = overlay

# Source files
HEADERS = edit_sample_factory.h \
          overlay.h \
          ui/overlapoverbitegui.h \
          ui/overlapoverbiteAnagui.h \
          ui/overlayoutlinepreview.h

SOURCES = edit_sample_factory.cpp \
          overlay.cpp \
          ui/overlapoverbitegui.cpp \
          ui/overlapoverbiteAnagui.cpp \
          ui/overlayoutlinepreview.cpp

# UI Forms
FORMS = ui/overlapoverbitegui.ui \
        ui/overlapoverbiteAnagui.ui

# Resources
RESOURCES = sampleedit.qrc

# Translation files
TRANSLATIONS += overlayCh.ts
TRANSLATIONS += overlayEn.ts

# Windows system libraries
# GDI32 and User32 are needed for GetDeviceCaps/GetDC used in OverlayOutlinePreview
win32-msvc:LIBS += Gdi32.lib User32.lib
