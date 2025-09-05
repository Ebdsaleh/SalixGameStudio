@echo off
SETLOCAL EnableDelayedExpansion
cls
REM =================================================================================
REM Clean Build Script for Debug Configuration
REM This script performs a full clean, re-configures CMake, pre-copies DLLs,
REM builds, and installs all targets for the Debug configuration.
REM =================================================================================

echo [Clean Build - Debug] Starting clean build process...

REM --- Configuration ---
SET CONFIGURATION=Debug
SET BUILD_ROOT_DIR=%~dp0build
SET CONFIG_OUTPUT_DIR=%BUILD_ROOT_DIR%\%CONFIGURATION%

REM --- Step 1: Ensure main build directory exists and configure CMake ---
echo [Clean Build - Debug] Ensuring CMake is configured...
if not exist "%BUILD_ROOT_DIR%" (
    mkdir "%BUILD_ROOT_DIR%"
)
pushd "%BUILD_ROOT_DIR%"
cmake .. -G "Visual Studio 17 2022"
if %ERRORLEVEL% NEQ 0 (
    echo [Clean Build - Debug] ❌ CMake configuration failed. Aborting.
    popd
    exit /b %ERRORLEVEL%
)
popd
echo [Clean Build - Debug] CMake configuration successful.

REM --- Step 2: Clean and recreate the configuration-specific output directory ---
echo [Clean Build - Debug] Cleaning and recreating '%CONFIG_OUTPUT_DIR%'...
if exist "%CONFIG_OUTPUT_DIR%" (
    rmdir /s /q "%CONFIG_OUTPUT_DIR%"
    if %ERRORLEVEL% NEQ 0 (
        echo [Clean Build - Debug] ❌ Error removing '%CONFIG_OUTPUT_DIR%'. Aborting.
        exit /b %ERRORLEVEL%
    )
)
mkdir "%CONFIG_OUTPUT_DIR%"
if %ERRORLEVEL% NEQ 0 (
    echo [Clean Build - Debug] ❌ Error creating '%CONFIG_OUTPUT_DIR%'. Aborting.
    exit /b %ERRORLEVEL%
)
echo [Clean Build - Debug] '%CONFIG_OUTPUT_DIR%' is clean.

REM --- Step 2.5: Pre-copy Runtime Dependencies ---
echo [Clean Build - Debug] Pre-copying required DLLs for test runner...
SET SDL2_LIB_DIR=%~dp0vendor\SDL2\lib\x64
SET SDL2_IMAGE_LIB_DIR=%~dp0vendor\SDL2_image\lib\x64
SET SDL2_TTF_LIB_DIR=%~dp0vendor\SDL2_ttf\lib\x64

copy "%SDL2_LIB_DIR%\SDL2.dll" "%CONFIG_OUTPUT_DIR%"
copy "%SDL2_IMAGE_LIB_DIR%\SDL2_image.dll" "%CONFIG_OUTPUT_DIR%"
copy "%SDL2_IMAGE_LIB_DIR%\libpng16-16.dll" "%CONFIG_OUTPUT_DIR%"
copy "%SDL2_IMAGE_LIB_DIR%\zlib1.dll" "%CONFIG_OUTPUT_DIR%"
copy "%SDL2_TTF_LIB_DIR%\SDL2_ttf.dll" "%CONFIG_OUTPUT_DIR%"
copy "%SDL2_TTF_LIB_DIR%\libfreetype-6.dll" "%CONFIG_OUTPUT_DIR%"


REM --- Step 3: Build all targets for the specific configuration ---
echo [Clean Build - Debug] Building all targets for %CONFIGURATION%...
pushd "%BUILD_ROOT_DIR%"
cmake --build . --config %CONFIGURATION%
if %ERRORLEVEL% NEQ 0 (
    echo [Clean Build - Debug] ❌ Build failed. Aborting.
    popd
    exit /b %ERRORLEVEL%
)
popd
echo [Clean Build - Debug] Build successful for %CONFIGURATION%.

REM --- Step 4: Install/Package targets for the specific configuration ---
echo [Clean Build - Debug] Installing targets for %CONFIGURATION%...
pushd "%BUILD_ROOT_DIR%"
cmake --install . --config %CONFIGURATION%
if %ERRORLEVEL% NEQ 0 (
    echo [Clean Build - Debug] ❌ Install/packaging failed. Aborting.
    popd
    exit /b %ERRORLEVEL%
)
popd
echo [Clean Build - Debug] Install/packaging successful for %CONFIGURATION%.

echo [Clean Build - Debug] ✅ Full clean build process completed successfully for Debug!
ENDLOCAL
exit /b 0