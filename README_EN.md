# FusionAnalyser (WebUI Edition) 🦷

> **⚠️ Architecture Update Notice (2026)**:
> This project was originally a desktop orthodontic analysis software developed using C++ and Qt5. Due to cross-platform compatibility issues (especially macOS frameless window focus bugs), we have **completely refactored the project into a pure frontend WebUI architecture**.
> The massive Qt UI codebase has been removed, but the core **C++ orthodontic mathematical algorithms** are preserved in the `src/` directory as open-source medical computing reference materials.

## 🌟 Project Overview

FusionAnalyser is now an out-of-the-box **pure frontend digital orthodontic model analysis system**. It runs entirely in the browser, requiring no backend services or complex runtime environments. Dentists or researchers can directly import intraoral scanned STL models and quickly calculate key orthodontic clinical indicators.

## ✨ Core Features

*   **🦷 3D Model Rendering Engine (Powered by Three.js)**
    *   Native support for importing and rendering high-precision upper/lower jaw `.stl` models.
    *   Full support for rotation, scaling, and panning.
    *   Medical-grade advanced dark theme lighting and material rendering.
*   **📊 Automatic Bolton Ratio Analysis**
    *   **Anterior Ratio (3-3)**: Automatically calculates and evaluates if it falls within the 78.8% standard range.
    *   **Overall Ratio (6-6)**: Automatically calculates and evaluates if it falls within the 91.5% standard range.
    *   Provides exact millimeter measurements for upper/lower jaw tooth mass discrepancies.
*   **📏 Crowding & Spacing Assessment**
    *   Automatically calculates model crowding/spacing based on input tooth widths and available arch length.
    *   Automatic grading: Spacing, Normal, Mild (Class I), Moderate (Class II), Severe (Class III) crowding.
*   **📋 Medical-Grade Cross Dental Chart**
    *   Standard FDI tooth numbering layout (18-28, 48-38), with support for quick missing tooth marking.

## 🚀 Quick Start

Thanks to the pure frontend, zero-dependency architecture, you only need a simple static HTTP server to run this project.

1. Clone this project:
   ```bash
   git clone https://github.com/your-username/FusionAnalyser.git
   cd FusionAnalyser/orthodontic-analyzer
   ```

2. Start any static file server (e.g., using macOS built-in Python):
   ```bash
   python3 -m http.server 8765
   ```

3. Access it in your browser:
   **http://localhost:8765**

## 📂 Directory Structure

```text
FusionAnalyser/
├── orthodontic-analyzer/    # 🎯 New pure frontend WebUI application
│   ├── index.html           # Main entry point
│   ├── style.css            # Medical-grade dark theme styles
│   ├── js/
│   │   ├── main.js          # Event binding and initialization
│   │   ├── viewer.js        # Three.js 3D rendering engine
│   │   ├── analyzer.js      # Bolton / Crowding calculation logic
│   │   └── ui.js            # Cross dental chart and result card interactions
│   └── example/             # Local STL demo models for testing
│
└── src/                     # 📚 Legacy C++ core algorithms (for reference)
    ├── common_ext/bolton/   # Native C++ Bolton algorithm implementation
    ├── common_ext/data/     # Native crowding, Spee curve, molar relation algorithms
    ├── common_ext/meshExt/  # Tooth OBB bounding box and arch curve fitting logic
    └── common_ext/machine/  # Delaunay, GJK collision, Convex hull 3D algorithms
```

## 📜 License
This project is released under the original open-source license (see `LICENSE` file for details). All newly refactored WebUI code follows the same open-source spirit.
