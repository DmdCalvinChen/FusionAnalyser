# FusionAnalyser Main Project File
# This is the root project file that includes all sub-projects

TEMPLATE = subdirs
CONFIG += ordered

# Core libraries - built first
SUBDIRS = common_base \
          common \
          common_ext \
          UI_Common

# Sample plugins
SUBDIRS += sampleplugins/EditMeasureAnaPlugin \
           sampleplugins/EditModelMarkPlugin \
           sampleplugins/decorateFusionPlugin \
           sampleplugins/overlay

# IO plugins
SUBDIRS += meshlabplugins/io_base

# Main application - built last
SUBDIRS += FusionAnalyser
				
# Platform-specific subdirectory settings
win32-msvc:FusionAnalyser.subdir = FusionAnalyser
macx:FusionAnalyser.subdir = FusionAnalyser

# Build dependencies - FusionAnalyser requires UI_Common to be built first
FusionAnalyser.depends = UI_Common
