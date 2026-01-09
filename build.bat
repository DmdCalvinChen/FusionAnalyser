@echo off
chcp 65001 >nul 2>&1
cls
setlocal enabledelayedexpansion

REM 设置 Qt 路径
set QT_DIR=D:\Qt\5.15.2\msvc2019_64
set QTDIR=%QT_DIR%

REM 检查 Qt 是否存在
if not exist "%QT_DIR%" (
    echo [ERROR] Qt directory does not exist: %QT_DIR%
    echo Please check if Qt is installed at this path.
    exit /b 1
)

if not exist "%QT_DIR%\bin\qmake.exe" (
    echo [ERROR] qmake.exe not found at: %QT_DIR%\bin\qmake.exe
    echo Please check if Qt is installed at this path.
    echo.
    echo Current Qt path: %QT_DIR%
    echo Expected location: %QT_DIR%\bin\qmake.exe
    exit /b 1
)

REM 设置 PATH（在确认 Qt 存在后）
set PATH=%QT_DIR%\bin;%PATH%
echo [OK] Qt found at: %QT_DIR%

REM 检查是否在 Visual Studio 环境中
where cl >nul 2>&1
if errorlevel 1 (
    echo [WARN] MSVC compiler environment not detected
    echo Attempting to setup Visual Studio environment...
    echo.
    
    REM 优先查找 VS2022（常见路径）
    set VS_FOUND=0
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        echo 找到 Visual Studio 2022 Community
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
        set VS_FOUND=1
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        echo 找到 Visual Studio 2022 Professional
        call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
        set VS_FOUND=1
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
        echo 找到 Visual Studio 2022 Enterprise
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
        set VS_FOUND=1
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
        echo 找到 Visual Studio 2019 Community
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
        set VS_FOUND=1
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        echo 找到 Visual Studio 2019 Professional
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
        set VS_FOUND=1
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
        echo 找到 Visual Studio 2019 Enterprise
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
        set VS_FOUND=1
    )
    
    if !VS_FOUND!==0 (
        echo [ERROR] Visual Studio environment not found
        echo Please run Visual Studio Developer Command Prompt manually, then re-run this script
        exit /b 1
    )
    
    REM 再次检查编译器是否可用
    where cl >nul 2>&1
    if errorlevel 1 (
        echo [ERROR] Failed to setup Visual Studio environment
        exit /b 1
    )
    echo [OK] MSVC compiler environment configured
    echo.
)

REM 切换到 src 目录
cd /d "%~dp0src"
if errorlevel 1 (
    echo [ERROR] src directory not found
    exit /b 1
)

REM 检查构建模式
set BUILD_MODE=incremental
if /i "%~1"=="clean" set BUILD_MODE=clean
if /i "%~1"=="rebuild" set BUILD_MODE=clean

echo ========================================
echo Building FusionAnalyser Project
echo Qt Path: %QT_DIR%
echo Build Mode: %BUILD_MODE%
echo ========================================
echo.

REM 只在明确指定 clean 时才清理（否则使用增量编译）
if "%BUILD_MODE%"=="clean" (
    echo [0/4] Cleaning build files for full rebuild...
    echo Removing old build artifacts...
    
    REM 递归清理所有子目录中的旧 Makefile
    echo   - Removing Makefiles...
    for /f "delims=" %%f in ('dir /s /b /a-d Makefile 2^>nul') do del /q "%%f" >nul 2>&1
    for /f "delims=" %%f in ('dir /s /b /a-d Makefile.Debug 2^>nul') do del /q "%%f" >nul 2>&1
    for /f "delims=" %%f in ('dir /s /b /a-d Makefile.Release 2^>nul') do del /q "%%f" >nul 2>&1
    
    REM 清理 Qt 生成的文件
    echo   - Removing Qt generated files...
    for /f "delims=" %%f in ('dir /s /b /a-d .qmake.stash 2^>nul') do del /q "%%f" >nul 2>&1
    for /f "delims=" %%f in ('dir /s /b /a-d moc_*.cpp 2^>nul') do del /q "%%f" >nul 2>&1
    for /f "delims=" %%f in ('dir /s /b /a-d qrc_*.cpp 2^>nul') do del /q "%%f" >nul 2>&1
    
    REM 清理编译生成的中间文件
    echo   - Removing object files and debug symbols...
    for /f "delims=" %%f in ('dir /s /b /a-d *.obj 2^>nul') do del /q "%%f" >nul 2>&1
    for /f "delims=" %%f in ('dir /s /b /a-d *.pdb 2^>nul') do del /q "%%f" >nul 2>&1
    for /f "delims=" %%f in ('dir /s /b /a-d *.exp 2^>nul') do del /q "%%f" >nul 2>&1
    for /f "delims=" %%f in ('dir /s /b /a-d *.ilk 2^>nul') do del /q "%%f" >nul 2>&1
    for /f "delims=" %%f in ('dir /s /b /a-d *.idb 2^>nul') do del /q "%%f" >nul 2>&1
    
    REM 清理 debug 和 release 目录中的中间文件
    if exist "debug" (
        echo   - Cleaning debug directory...
        for /f "delims=" %%f in ('dir /s /b /a-d debug\*.obj 2^>nul') do del /q "%%f" >nul 2>&1
        for /f "delims=" %%f in ('dir /s /b /a-d debug\*.pdb 2^>nul') do del /q "%%f" >nul 2>&1
    )
    if exist "release" (
        echo   - Cleaning release directory...
        for /f "delims=" %%f in ('dir /s /b /a-d release\*.obj 2^>nul') do del /q "%%f" >nul 2>&1
        for /f "delims=" %%f in ('dir /s /b /a-d release\*.pdb 2^>nul') do del /q "%%f" >nul 2>&1
    )
    
    echo Cleanup completed
    echo.
) else (
    echo [0/4] Incremental build mode - keeping existing build artifacts
    echo Tip: Use 'build.bat clean' for full rebuild if needed
    echo.
)

REM 运行 qmake 生成 Makefile
echo [1/4] Running qmake...
call "%QT_DIR%\bin\qmake.exe" FusionAnalyser.pro
if errorlevel 1 (
    echo [ERROR] qmake execution failed
    exit /b 1
)

REM 检测 CPU 核心数用于并行编译
for /f "tokens=2 delims==" %%i in ('wmic cpu get NumberOfCores /value ^| findstr "="') do set CPU_CORES=%%i
if "%CPU_CORES%"=="" set CPU_CORES=4

REM 设置并行编译环境变量
REM /MP 让 cl.exe 并行编译多个源文件，数字指定最大并行数
set _CL_=/MP%CPU_CORES%

echo Detected %CPU_CORES% CPU cores
echo Parallel compilation enabled:
echo   - nmake parallel tasks: Auto (using all available cores)
echo   - compiler parallel tasks: %CPU_CORES% files simultaneously
echo.

REM 编译项目（使用 nmake，实际并行由 cl.exe /MP 完成）
echo [2/4] Starting build (this may take some time)...
call nmake
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed
    echo Tip: If you encounter linking errors, you may need to build external libraries first:
    echo       cd external ^&^& qmake external.pro ^&^& nmake
    exit /b 1
)

REM 清理临时文件（可选）
echo.
echo [3/4] Build completed!
echo.
echo Executable locations:
echo   Release: src\distrib\FusionAnalyser.exe
echo   Debug:   src\distribD\FusionAnalyser.exe
echo.

endlocal

