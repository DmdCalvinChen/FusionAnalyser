#!/bin/bash

# macOS 构建脚本 for FusionAnalyser

set -e  # 遇到错误立即退出

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 查找 Qt 5
QT_BIN=""
QT_SEARCH_PATHS=(
    "/opt/homebrew/opt/qt@5/bin"
    "/usr/local/opt/qt@5/bin"
    "/opt/homebrew/opt/qt/bin"
    "/usr/local/opt/qt/bin"
    "$HOME/Qt/5.15.2/clang_64/bin"
)

for path in "${QT_SEARCH_PATHS[@]}"; do
    if [ -f "$path/qmake" ]; then
        QT_BIN="$path"
        break
    fi
done

if [ -z "$QT_BIN" ]; then
    echo_error "Qt 5 not found! Searched paths:"
    for path in "${QT_SEARCH_PATHS[@]}"; do
        echo "  - $path"
    done
    echo ""
    echo_info "Please install Qt 5.15 using Homebrew:"
    echo "  brew install qt@5"
    exit 1
fi

# 设置 Qt 环境
export QTDIR="$QT_BIN/.."
export PATH="$QT_BIN:$PATH"

# 检查 qmake 版本
QMAKE_VERSION=$("$QT_BIN/qmake" -v | grep "Qt version" | sed 's/.*Qt version \([0-9.]*\).*/\1/' || echo "")
if [[ -z "$QMAKE_VERSION" ]]; then
    echo_error "Failed to detect Qt version"
    exit 1
fi

if [[ ! "$QMAKE_VERSION" =~ ^5\. ]]; then
    echo_error "Qt 5.x is required, but found Qt $QMAKE_VERSION"
    echo_info "Qt path: $QT_BIN"
    exit 1
fi

echo_info "Qt $QMAKE_VERSION found at: $QTDIR"

# 检查 vcglib
VCGLIB_DIR="$SCRIPT_DIR/../vcglib"
if [ ! -d "$VCGLIB_DIR" ]; then
    echo_error "vcglib not found at: $VCGLIB_DIR"
    echo_info "vcglib should be at the same level as src directory"
    exit 1
fi
echo_info "vcglib found at: $VCGLIB_DIR"

# 切换到 src 目录
cd "$SCRIPT_DIR/src"
if [ $? -ne 0 ]; then
    echo_error "src directory not found"
    exit 1
fi

# 检查构建模式
BUILD_MODE="incremental"
if [ "$1" == "clean" ] || [ "$1" == "rebuild" ]; then
    BUILD_MODE="clean"
fi

echo ""
echo "========================================"
echo "Building FusionAnalyser Project"
echo "Qt Path: $QTDIR"
echo "Build Mode: $BUILD_MODE"
echo "========================================"
echo ""

# 清理构建文件
if [ "$BUILD_MODE" == "clean" ]; then
    echo_info "[0/4] Cleaning build files for full rebuild..."
    echo "Removing old build artifacts..."

    # 清理 Makefile
    echo "  - Removing Makefiles..."
    find . -name "Makefile" -type f -delete 2>/dev/null || true
    find . -name ".qmake.stash" -type f -delete 2>/dev/null || true

    # 清理 Qt 生成的中间文件
    echo "  - Removing Qt generated files (moc, qrc, ui)..."
    find . -name "moc_*.cpp" -type f -delete 2>/dev/null || true
    find . -name "qrc_*.cpp" -type f -delete 2>/dev/null || true
    find . -name "ui_*.h" -type f -delete 2>/dev/null || true

    # 清理编译生成的中间文件
    echo "  - Removing object files..."
    find . -name "*.o" -type f -delete 2>/dev/null || true
    find . -name "*.a" -type f -delete 2>/dev/null || true

    echo_info "Cleanup completed"
    echo ""
else
    echo_info "[0/4] Incremental build mode - keeping existing build artifacts"
    echo "Tip: Use './build.sh clean' for full rebuild if needed"
    echo ""
fi

# 检测 CPU 核心数
CPU_CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo "4")
echo_info "Detected $CPU_CORES CPU cores"
echo "Parallel compilation enabled: using -j$CPU_CORES"
echo ""

# 运行 qmake 生成 Makefile
echo_info "[1/4] Running qmake..."
"$QT_BIN/qmake" FusionAnalyser.pro
if [ $? -ne 0 ]; then
    echo_error "qmake execution failed"
    exit 1
fi

# qmake has a bug: it doesn't generate UIC rules for TEMPLATE=lib with FORMS in subdirs
# Workaround: manually run uic for missing ui_*.h files
echo_info "[1.5/4] Generating UI headers (qmake workaround)..."
find . -name "*.ui" -type f | while read ui_file; do
    dir=$(dirname "$ui_file")
    base=$(basename "$ui_file" .ui)
    base_lower=$(echo "$base" | tr 'A-Z' 'a-z')
    ui_header="$dir/ui_${base_lower}.h"
    if [ ! -f "$ui_header" ] || [ "$ui_file" -nt "$ui_header" ]; then
        "$QT_BIN/uic" "$ui_file" -o "$ui_header"
    fi
done

# 编译项目
echo_info "[2/4] Starting build (this may take some time)..."
make -j"$CPU_CORES"
if [ $? -ne 0 ]; then
    echo ""
    echo_error "Build failed"
    exit 1
fi

# 构建完成
echo ""
echo_info "[3/4] Build completed successfully!"
echo ""
echo "Executable locations:"
echo "  Release: src/distrib/FusionAnalyser.app"
echo "  Debug:   src/distribD/FusionAnalyser.app"
echo ""

# 提示如何运行
echo_info "To run the application:"
echo "  cd src/distrib"
echo "  open FusionAnalyser.app"
echo "  # or"
echo "  ./FusionAnalyser.app/Contents/MacOS/FusionAnalyser"
echo ""
