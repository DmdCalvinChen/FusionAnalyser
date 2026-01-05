# Third-Party Licenses

This document lists the third-party libraries actually used in FusionAlign and their licenses.

## Framework Dependencies

### Qt Framework
- **Version**: 5.x 
- **Modules Used**: QtCore, QtGui, QtWidgets, QtOpenGL, QtXml, QtXmlPatterns, QtNetwork, QtScript, QtPrintSupport, QtSvg, QtSql
- **License**: LGPL v3
- **Website**: https://www.qt.io/
- **Note**: Qt is the primary GUI framework. When using LGPL version, dynamic linking is required.

## Core Graphics Libraries

### vcglib
- **Location**: `vcglib/`
- **License**: GNU General Public License v3.0
- **Copyright**: Visual Computing Lab, ISTI-CNR
- **Website**: https://github.com/cnr-isti-vclab/vcglib
- **Note**: Must be cloned separately at the same level as the `src` directory
- **License File**: `vcglib/LICENSE.txt`

### Eigen
- **Location**: `vcglib/eigenlib/`
- **License**: MPL 2.0 (Mozilla Public License 2.0)
- **Website**: https://eigen.tuxfamily.org/
- **Note**: Included as part of vcglib, header-only linear algebra library

### GLEW (OpenGL Extension Wrangler Library)
- **Version**: 2.1.0
- **Location**: `src/external/glew-2.1.0/`
- **License**: Modified BSD License / Mesa 3-D License / Khronos License
- **Copyright**: 
  - Copyright (C) 2002-2007, Milan Ikits, Marcelo E. Magallon, Lev Povalahev
  - Copyright (C) 1999-2007 Brian Paul (Mesa)
  - Copyright (c) 2007 The Khronos Group Inc.
- **Website**: http://glew.sourceforge.net/
- **License File**: `src/external/glew-2.1.0/LICENSE.txt`

## Utility Libraries

### SpdLog
- **Location**: `src/external/SpdLog/`
- **License**: MIT License
- **Website**: https://github.com/gabime/spdlog
- **Note**: Fast C++ logging library, header-only

### Google Breakpad
- **Location**: `src/external/break_check/`
- **License**: Apache License 2.0
- **Copyright**: Copyright (c) Google Inc.
- **Website**: https://chromium.googlesource.com/breakpad/breakpad
- **Note**: Crash reporting system

## License Files Location

License files are in their respective directories:
- `vcglib/LICENSE.txt` - GPL v3
- `src/external/glew-2.1.0/LICENSE.txt` - Modified BSD/Mesa/Khronos
- Individual library headers contain license information
