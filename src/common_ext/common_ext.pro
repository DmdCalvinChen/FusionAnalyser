# Common Extended Library Project File
# Extended functionality including dental analysis, mesh processing, and UI components

include(../general.pri)

# Library configuration
TEMPLATE = lib
TARGET = common_ext
DEFINES += COMMON_EXT_LIBRARY
CONFIG += c++11

# Required Qt modules
QT += xml gui opengl script network svg sql

# macOS configuration
macx:INCLUDEPATH += .. $$VCGDIR $$EIGENDIR $$GLEWDIR/include $$COMMON_BASE $$SPDLOG/include $$UI_COMMONDIR
macx:LIBS += -L../distrib/ -lcommon -lcommon_base
macx:DESTDIR = ../distrib

# Windows configuration
win32-msvc:INCLUDEPATH += .. $$VCGDIR $$GLEWDIR/include $$COMMON_BASE $$SPDLOG/include $$UI_COMMONDIR ../
win32-msvc:LIBSDIR =
win32-msvc:DEPENDSLIB = -ldxgi -lVersion

CONFIG(debug, release | debug) {
    win32-msvc:LIBS += -L../distribD/ -lcommon -lcommon_base opengl32.lib glu32.lib $$LIBSDIR $$DEPENDSLIB
    win32-msvc:DESTDIR = ../distribD
} else {
    win32-msvc:LIBS += -L../distrib/ -lcommon -lcommon_base opengl32.lib glu32.lib $$LIBSDIR $$DEPENDSLIB
    win32-msvc:DESTDIR = ../distrib
}

# Defines
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
win32-msvc:DEFINES += GLEW_STATIC

# Source files - organized by functional area
SOURCES += \
    # Auto alignment tools
    autoAlign/longaxislocationtool.cpp \
    autoAlign/toothmodelmanager.cpp \
    \
    # Data management
    data/fusionAnalyserData.cpp \
    # Mesh processing
    meshExt/toothexpand.cpp \
    \
    # Project parsing
    projectParser/ProjectParser.cpp \
    projectParser/fusionProjectParser.cpp \
    \
    # Trackball and interaction tools
    trackBallTool/ZoomHandle.cpp \
    trackBallTool/customadjustinghandle.cpp \
    trackBallTool/customtrackball.cpp \
    trackBallTool/rotatehandle.cpp \
    trackBallTool/trackballtool.cpp \
    trackBallTool/translatehandle.cpp \
    trackBallTool/dragmovehandle.cpp \
    \
    # Analysis tools
    bolton/BoltonAna.cpp \
    \
    # Command system
    commandmode/commandcommon.cpp \
    commandmode/commandcommonvector.cpp \
    commandmode/commandmanager.cpp \
    commandmode/macrocommand.cpp \
    \
    # Core library file
    common_ext.cpp \
    \
    # Data structures
    data/fusionaligndata.cpp \
    data/dentalmanager.cpp \
    data/separationmanager.cpp \
    data/dentalanalysisdata.cpp \
    data/dentalanalysisdata_expand.cpp \
    data/towablectrlsystem.cpp \
    data/interactive2Dlabel.cpp \
    data/interactive2Dframe.cpp \
    data/CutFace.cpp \
    \
    # Computational geometry algorithms
    machine/archlinemachine.cpp \
    machine/boolean2Dmachine.cpp \
    machine/booleanmachine.cpp \
    machine/combededgemachine.cpp \
    machine/convexhellmachine.cpp \
    machine/delaunaymachine.cpp \
    machine/gjkmachine.cpp \
    machine/triangleintersectmachine.cpp \
    machine/melkman_convexhull2D.cpp \
    \
    # Mesh data structures
    meshExt/AbsMesh.cpp \
    meshExt/Dental.cpp \
    meshExt/Tooth.cpp \
    meshExt/dentalexpand.cpp \
    meshExt/dentalfeatures.cpp \
    \
    # Physics simulation
    physics/rigidbody.cpp \
    \
    # Utility functions
    util/mesh_bounding_box.cpp \
    util/mesh_vertex.cpp \
    util/custom_vector_3d.cpp \
    util/utility_tools.cpp \
    util/coordinate_system_handles.cpp \
    util/definite_intersection.cpp \
    util/mass_point.cpp \
    util/mesh_generator.cpp \
    util/mesh_edge.cpp \
    util/oriented_bounding_box.cpp \
    util/custom_bounding_box.cpp \
    util/custom_plane.cpp \
    util/wave_point.cpp \
    util/oriented_bounding_box_collider.cpp \
    util/skip_list.cpp \
    \
    # Widgets and UI components
    widget/glareaBase.cpp \
    \
    # Point cloud processing
    pointcloud/cloudkdtree.cpp \
    pointcloud/cloudnode.cpp \
    pointcloud/cloudshader.cpp \
    pointcloud/cloudsurfacereconstruction.cpp \
    pointcloud/bilateralfiltercsr.cpp \
    \
    # File management
    fileManager/filemanager.cpp
   


# Header files - organized by functional area
HEADERS += \
    # Auto alignment
    autoAlign/longaxislocationtool.h \
    autoAlign/toothmodelmanager.h \
    \
    # Data structures
    data/fusionAnalyserData.h \
    data/fusionDataCommon.h \
    data/CutFace.h \
    data/fusionaligndata.h \
    data/dentalmanager.h \
    data/separationmanager.h \
    data/dentalanalysisdata.h \
    data/towablectrlsystem.h \
    data/interactive2Dlabel.h \
    data/interactive2Dframe.h \
    data/fusion/segmentedstatusrecord.h \
    \
    # Project parsing
    projectParser/ProjectParser.h \
    projectParser/fusionProjectParser.h \
    \
    # Trackball and interaction
    trackBallTool/ZoomHandle.h \
    trackBallTool/customadjustinghandle.h \
    trackBallTool/customtrackball.h \
    trackBallTool/rotatehandle.h \
    trackBallTool/trackballtool.h \
    trackBallTool/translatehandle.h \
    trackBallTool/dragmovehandle.h \
    \
    # Analysis tools
    bolton/BoltonAna.h \
    \
    # Command system
    commandmode/basecommand.h \
    commandmode/basecommandmanagerinterface.h \
    commandmode/commandcommon.h \
    commandmode/commandcommonvector.h \
    commandmode/commandmanager.h \
    commandmode/macrocommand.h \
    \
    # Core headers
    common_ext_global.h \
    common_ext.h \
    \
    # Computational geometry
    machine/archlinemachine.h \
    machine/boolean2Dmachine.h \
    machine/booleanmachine.h \
    machine/combededgemachine.h \
    machine/convexhellmachine.h \
    machine/delaunaymachine.h \
    machine/gjkmachine.h \
    machine/triangleintersectmachine.h \
    machine/melkman_convexhull2D.h \
    \
    # Mesh data structures
    meshExt/AbsMesh.h \
    meshExt/Dental.h \
    meshExt/Tooth.h \
    meshExt/dentalfeatures.h \
    \
    # Physics
    physics/rigidbody.h \
    \
    # Utilities
    util/mesh_bounding_box.h \
    util/mesh_vertex.h \
    util/custom_vector_3d.h \
    util/utility_tools.h \
    util/VectorAssist.h \
    util/assist_geometry.h \
    util/coordinate_system_handles.h \
    util/definite_intersection.h \
    util/mass_point.h \
    util/mesh_generator.h \
    util/mesh_edge.h \
    util/oriented_bounding_box.h \
    util/custom_bounding_box.h \
    util/custom_plane.h \
    util/wave_point.h \
    util/oriented_bounding_box_collider.h \
    util/skip_list.h \
    \
    # GUI components
    guiAttributeCustom/baseattributedefaultgui.h \
    widget/glareaBase.h \
    \
    # Point cloud processing
    pointcloud/cloudkdtree.h \
    pointcloud/cloudnode.h \
    pointcloud/cloudshader.h \
    pointcloud/cloudsurfacereconstruction.h \
    pointcloud/bilateralfiltercsr.h \
    pointcloud/camerastate.h \
    \
    # File management
    fileManager/filemanager.h

TRANSLATIONS += resources/languages/common_ext_ch.ts
TRANSLATIONS += resources/languages/common_ext_en.ts

RESOURCES += common_ext.qrc

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
