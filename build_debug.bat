@echo off
SETLOCAL EnableDelayedExpansion
cls
REM =================================================================================
REM Incremental Build Script for Debug Configuration
REM This script configures CMake (if needed) and builds any changed files.
REM =================================================================================

echo [Build - Debug] Starting incremental build process...

REM --- Configuration ---
SET CONFIGURATION=Debug
SET BUILD_ROOT_DIR=%~dp0build

REM --- Step 1: Ensure CMake is configured ---
if not exist "%BUILD_ROOT_DIR%\CMakeCache.txt" (
    echo [Build - Debug] CMake not configured. Running configuration...
    if not exist "%BUILD_ROOT_DIR%" (
        mkdir "%BUILD_ROOT_DIR%"
    )
    pushd "%BUILD_ROOT_DIR%"
    cmake .. -G "Visual Studio 17 2022"
    if %ERRORLEVEL% NEQ 0 (
        echo [Build - Debug] ❌ CMake configuration failed. Aborting.
        popd
        exit /b %ERRORLEVEL%
    )
    popd
)

REM --- Step 2: Build all targets ---
echo [Build - Debug] Building targets for %CONFIGURATION%...
pushd "%BUILD_ROOT_DIR%"
cmake --build . --config %CONFIGURATION%
if %ERRORLEVEL% NEQ 0 (
    echo [Build - Debug] ❌ Build failed. Aborting.
    popd
    exit /b %ERRORLEVEL%
)
popd
echo [Build - Debug] ✅ Build successful for %CONFIGURATION%.
ENDLOCAL
exit /b 0