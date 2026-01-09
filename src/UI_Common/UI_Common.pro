# UI_Common Library Project File
# Provides common UI components, dialogs, and widgets

include(../general.pri)

# Output directory configuration
CONFIG(debug, debug|release) {
    win32-msvc:DESTDIR = ../distribD
} else {
    win32-msvc:DESTDIR = ../distrib
}

macx:DESTDIR = ../distrib

# Library configuration
TARGET = UI_Common
TEMPLATE = lib
DEFINES += UI_COMMON_LIBRARY

# Required Qt modules
QT += widgets opengl xml xmlpatterns svg network printsupport

# Windows library dependencies
# GDI32 is needed for GetDeviceCaps used in FusionTitleBar / PdfReaderGui
win32-msvc:LIBS += opengl32.lib GLU32.lib Gdi32.lib
COMMONLIB = -lcommon_base -lcommon -lcommon_ext

CONFIG(debug, release | debug) {
    win32-msvc:LIBS += -L../distribD $$COMMONLIB
} else {
    win32-msvc:LIBS += -L../distrib $$COMMONLIB
}

# macOS library dependencies
macx:LIBS += -L../distrib/plugins -lcommon_base -L../distrib -lcommon -lcommon_ext

# Defines
win32-msvc:DEFINES += GLEW_STATIC
DEFINES += QT_DEPRECATED_WARNINGS

# Include paths
INCLUDEPATH += .. \
               $$VCGDIR \
               $$EIGENDIR \
               $$GLEWDIR/include \
               $$SPDLOG/include \
               $$COMMON_BASE \
               $$COMMON_EXT

SOURCES += \
    UserCenterUI/fusionChangeLanguageDlg.cpp \
    UserCenterUI/workDirSetingDlg.cpp \
           aboutDlg/QtAboutDialog.cpp \
    framelessWindow/fusionFramelessBaseDlg.cpp \
           framelessWindow/fusionTitlebar.cpp \
           qFusionProgressBar/QFusionProgressDlg.cpp \
           uicommon.cpp \
           fusionViewGui/fusionViewGui.cpp \
           boltonAna/BoltonAnaDlg.cpp \
           guicontrolcustomdesign/arrow.cpp \
           guicontrolcustomdesign/checkbox.cpp \
           guicontrolcustomdesign/collectfolded.cpp \
           guicontrolcustomdesign/label.cpp \
           guicontrolcustomdesign/lock.cpp \
           guicontrolcustomdesign/rectangle.cpp \
           guicontrolcustomdesign/rhombus.cpp \
           guicontrolcustomdesign/slider.cpp \
           guicontrolcustomdesign/texturemap.cpp \
           toothfdiguidegui/toothfdiguidegui.cpp \
           toothfdiguidegui/anomalybutton.cpp \
           gifViewLabel/gifviewgui.cpp \
           gifViewLabel/GifViewLabel.cpp \
           qFusionProgressBar/QFusionProgressBar.cpp \
           redoUndogui/redoUndoGui.cpp \
           ImportModelGui/importmodelgui.cpp \
           FusionAnalyserHomeGui/fusionanalyserhomegui.cpp \
           navAnabutton/navanabutton.cpp  \
           FusionAnalyserHomeGui/fusionanalyserbuttongui/tipswidget.cpp
    

HEADERS += \
    UserCenterUI/fusionChangeLanguageDlg.h \
    UserCenterUI/workDirSetingDlg.h \
    aboutDlg/QtAboutDialog.h \
    framelessWindow/fusionFramelessBaseDlg.h \
           framelessWindow/fusionTitlebar.h \
    qFusionProgressBar/QFusionProgressDlg.h \
           uicommon.h \
           uicommon_global.h \
           fusionViewGui/fusionViewGui.h \
           boltonAna/BoltonAnaDlg.h \
           guicontrolcustomdesign/arrow.h \
           guicontrolcustomdesign/checkbox.h \
           guicontrolcustomdesign/collectfolded.h \
           guicontrolcustomdesign/label.h \
           guicontrolcustomdesign/lock.h \
           guicontrolcustomdesign/rectangle.h \
           guicontrolcustomdesign/rhombus.h \
           guicontrolcustomdesign/slider.h \
           guicontrolcustomdesign/texturemap.h \
           gifViewLabel/gifviewgui.h \
           gifViewLabel/GifViewLabel.h \
           qFusionProgressBar/QFusionProgressBar.h \
           redoUndogui/redoUndoGui.h \
           toothfdiguidegui/toothfdiguidegui.h \
           toothfdiguidegui/anomalybutton.h \
           ImportModelGui/importmodelgui.h \
           FusionAnalyserHomeGui/fusionanalyserhomegui.h \
           navAnabutton/navanabutton.h \
           FusionAnalyserHomeGui/fusionanalyserbuttongui/tipswidget.h
		
FORMS = \
    UserCenterUI/fusionchangelanguagedlg.ui \
    UserCenterUI/workDirSetingDlg.ui \
    qFusionProgressBar/qfusionprogressdlg.ui \
    fusionViewGui/fusionViewGui.ui \
    toothfdiguidegui/toothfdiguidegui.ui \
    ImportModelGui/importmodelgui.ui \
    FusionAnalyserHomeGui/fusionanalyserhomegui.ui \
    FusionAnalyserHomeGui/fusionanalyserbuttongui/tipswidget.ui \
    gifViewLabel/gifviewgui.ui \
    redoUndogui/redoUndoGui.ui \
    boltonAna/BoltonAnaDlg.ui \
    analysisgroup/AnalysisReportGui/analysisreportgui.ui \
    analysisgroup/PdfReaderGui/pdfreadergui.ui \
    analysisgroup/PdfReaderGui/PdfReader/pdfreader.ui \
    analysisgroup/analysisreport/analysisreport.ui
		

RESOURCES += UI_Common.qrc aboutDlg/aboutdlg.qrc

TRANSLATIONS += ui_common_ch.ts
TRANSLATIONS += ui_common_en.ts

# Unix installation path
unix {
    target.path = /usr/local/lib
    INSTALLS += target
}
