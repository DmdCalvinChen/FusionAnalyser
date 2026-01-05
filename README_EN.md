# FusionAnalyser

<div align="center">

<img src="images/logo.png" alt="FusionAnalyser Logo" width="200"/>

**Professional, Open-Source Digital Dental Model Measurement Analysis Software**

[Website](https://www.ai-align.cn/fusionanalyser/) • [Quick Start](#build) • [Documentation](#usage)

English | [简体中文](./README.md)

</div>

FusionAnalyser is a professional, open-source digital dental model measurement and analysis software designed for orthodontic clinical scenarios, providing efficient and accurate model analysis capabilities. Built on Qt and OpenGL, it offers a complete digital model analysis solution for orthodontists.

**Source Code Repository**: https://github.com/aialign2025/FusionAnalyser

## Highlights

- **Simpler**: Step-by-step guided workflow for easy operation
- **More Accurate**: Strictly follow textbook-defined measurement items with precise point measurements
- **More Intuitive**: Clinical reference value display, abnormal value alerts, clear and complete analysis reports
- **More Open**: Free and open data interfaces, compatible with intraoral scanners and desktop scanners from all brands

## Build

The project is developed based on Qt 5 and OpenGL, supporting Windows platform.

### System Requirements

- **Compiler**: MSVC 2019/2022 (Visual Studio)
- **Qt**: Qt 5.12+ (requires QtOpenGL, QtXml, QtXmlPatterns, QtScript, QtNetwork, QtSvg, QtPrintSupport modules)
- **vcglib**: Visualization and Computer Graphics Library (must be cloned in project root, at same level as `src`)
- **Third-party libraries** (included in `src/external/`): GLEW, FreeType, jhead, SpdLog, QuaZip, Google Breakpad

### Project Structure

```
fusionalign/
├── build.bat              # Windows quick build script
├── vcglib/                # VCG library (manually cloned, at same level as src)
├── src/
│   ├── external/          # Third-party libraries
│   ├── common_base/       # Basic utility libraries
│   ├── common/            # Core framework
│   ├── common_ext/        # Dental extensions
│   ├── UI_Common/         # Shared UI components
│   ├── FusionAnalyser/    # Main application
│   ├── sampleplugins/     # Dental analysis plugins
│   └── distrib/           # Build output directory
└── LICENSE.txt
```

### Build Steps

1. Ensure Visual Studio 2019/2022 and Qt 5.15.2 are installed
2. Clone vcglib to the project root directory:
   ```cmd
   git clone https://github.com/cnr-isti-vclab/vcglib.git
   ```
3. Modify the Qt path in `build.bat` if needed:
   ```bat
   set QT_DIR=D:\Qt\5.15.2\msvc2019_64
   ```
4. Run the build script:
   ```cmd
   build.bat
   ```
5. Clean rebuild (optional):
   ```cmd
   build.bat clean
   ```

**Output Location:**
- Release version: `src\distrib\FusionAnalyser.exe`
- Debug version: `src\distribD\FusionAnalyser.exe`
- Plugins directory: `src\distrib\plugins\`

## Technical Architecture

FusionAnalyser adopts a plugin-based architecture design, including the following main modules:

- **Main Application**: FusionAnalyser (3D rendering viewport and user interface)
- **Core Plugins**: Measurement analysis, model marking, rendering decoration, file import/export
- **Common Libraries**: Basic utilities, 3D processing libraries, dental extensions, UI components

## License

This project is licensed under **GNU General Public License v3.0** (GPL v3)

- See `LICENSE.txt` file for details
- See `THIRD_PARTY_LICENSES.md` for third-party library licenses

## Contact

- **Official Website**: https://www.ai-align.cn/fusionanalyser/
- **Source Code Repository**: https://github.com/aialign2025/FusionAnalyser
- **Issue Tracker**: [GitHub Issues](https://github.com/aialign2025/FusionAnalyser/issues)

---

**Note**: This software is for research and educational purposes only. Please consult a professional doctor for clinical diagnosis.
