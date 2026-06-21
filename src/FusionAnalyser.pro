# FusionAnalyser Main Project File
# This is the root project file that includes all sub-projects

TEMPLATE = subdirs
CONFIG += ordered

# Core libraries - built first
SUBDIRS = common_base \
          common \
          common_ext

# Backend Engine Application
SUBDIRS += backend_engine

# Build dependencies
backend_engine.depends = common_ext
