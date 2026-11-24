@echo off
setlocal EnableExtensions DisableDelayedExpansion

REM Configure before starting MSBuild, even for incremental builds.
REM A newly discovered test suite may otherwise be added to the solution
REM during ZERO_CHECK, too late for that invocation's original build graph.
REM Optional argument: an explicit CMake target, e.g. SalixEditor_tests.
REM No cache removal, clean, installation or Git operations.

if not "%~2"=="" (
    echo Usage: build_debug.bat [CMake-target]
    exit /b 1
)

set "BUILD_ROOT_DIR=%~dp0build"
set "SOURCE_ROOT_DIR=%~dp0."
set "CONFIGURATION=Debug"

where cmake.exe >nul 2>&1
if errorlevel 1 (
    echo [Build - Debug] ERROR: cmake.exe not found on PATH.
    exit /b 1
)

echo [Build - Debug] Configuring before building...
if exist "%BUILD_ROOT_DIR%\CMakeCache.txt" (
    cmake -S "%SOURCE_ROOT_DIR%" -B "%BUILD_ROOT_DIR%"
) else (
    cmake -S "%SOURCE_ROOT_DIR%" -B "%BUILD_ROOT_DIR%" -G "Visual Studio 17 2022" -A x64 -T host=x64
)
if errorlevel 1 (
    echo [Build - Debug] ERROR: CMake configuration failed.
    exit /b 1
)

if "%~1"=="" goto :build_all
echo [Build - Debug] Building target "%~1"...
cmake --build "%BUILD_ROOT_DIR%" --config "%CONFIGURATION%" --target "%~1" --parallel 4
goto :build_result

:build_all
echo [Build - Debug] Building all targets...
cmake --build "%BUILD_ROOT_DIR%" --config "%CONFIGURATION%" --parallel 4

:build_result
if errorlevel 1 (
    echo [Build - Debug] ERROR: Build failed.
    exit /b 1
)
echo [Build - Debug] Build successful.
exit /b 0
