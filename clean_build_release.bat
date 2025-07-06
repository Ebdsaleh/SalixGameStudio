@echo off
SETLOCAL EnableDelayedExpansion
cls
REM =================================================================================
REM Clean Build Script for Release Configuration
REM This script performs a full clean, re-configures CMake, builds,
REM and installs all targets for the Release configuration.
REM =================================================================================

echo [Clean Build - Release] Starting clean build process...

REM --- Configuration ---
SET CONFIGURATION=Release
SET BUILD_ROOT_DIR=%~dp0build
SET CONFIG_OUTPUT_DIR=%BUILD_ROOT_DIR%\%CONFIGURATION%

REM --- Step 1: Ensure main build directory exists and configure CMake ---
REM This step should only configure, not clean the whole build directory.
echo [Clean Build - Release] Ensuring CMake is configured...
if not exist "%BUILD_ROOT_DIR%" (
    mkdir "%BUILD_ROOT_DIR%"
)
pushd "%BUILD_ROOT_DIR%"
cmake .. -G "Visual Studio 17 2022"
if %ERRORLEVEL% NEQ 0 (
    echo [Clean Build - Release] ❌ CMake configuration failed. Aborting.
    popd
    exit /b %ERRORLEVEL%
)
popd
echo [Clean Build - Release] CMake configuration successful.

REM --- Step 2: Clean and recreate the configuration-specific output directory ---
echo [Clean Build - Release] Cleaning and recreating '%CONFIG_OUTPUT_DIR%'...
if exist "%CONFIG_OUTPUT_DIR%" (
    rmdir /s /q "%CONFIG_OUTPUT_DIR%"
    if %ERRORLEVEL% NEQ 0 (
        echo [Clean Build - Release] ❌ Error removing '%CONFIG_OUTPUT_DIR%'. Aborting.
        exit /b %ERRORLEVEL%
    )
)
mkdir "%CONFIG_OUTPUT_DIR%"
if %ERRORLEVEL% NEQ 0 (
    echo [Clean Build - Release] ❌ Error creating '%CONFIG_OUTPUT_DIR%'. Aborting.
    exit /b %ERRORLEVEL%
)
echo [Clean Build - Release] '%CONFIG_OUTPUT_DIR%' is clean.

REM --- Step 3: Build all targets for the specific configuration ---
echo [Clean Build - Release] Building all targets for %CONFIGURATION%...
pushd "%BUILD_ROOT_DIR%"
cmake --build . --config %CONFIGURATION%
if %ERRORLEVEL% NEQ 0 (
    echo [Clean Build - Release] ❌ Build failed. Aborting.
    popd
    exit /b %ERRORLEVEL%
)
popd
echo [Clean Build - Release] Build successful for %CONFIGURATION%.

REM --- Step 4: Install/Package targets for the specific configuration ---
echo [Clean Build - Release] Installing targets for %CONFIGURATION% (copying runtime dependencies)...
pushd "%BUILD_ROOT_DIR%"
cmake --install . --config %CONFIGURATION%
if %ERRORLEVEL% NEQ 0 (
    echo [Clean Build - Release] ❌ Install/packaging failed. Aborting.
    popd
    exit /b %ERRORLEVEL%
)
popd
echo [Clean Build - Release] Install/packaging successful for %CONFIGURATION%.

echo [Clean Build - Release] ✅ Full clean build process completed successfully for Release!
ENDLOCAL
exit /b 0