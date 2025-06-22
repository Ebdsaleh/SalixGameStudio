@echo off
SETLOCAL EnableDelayedExpansion
REM =================================================================================
REM Build Script for a Sandbox Game Project DLL
REM This script emulates the "Build" button in the future Salix Editor.
REM =================================================================================

echo [Sandbox Build] Starting...

REM --- Configuration ---
REM We will hard-code the project name for our consistent testing.
SET PROJECT_NAME=TestProject
SET SANDBOX_ROOT=src\Sandbox
SET PROJECT_DIR=%SANDBOX_ROOT%\%PROJECT_NAME%
SET SCRIPTS_DIR=%PROJECT_DIR%\Assets\Scripts
SET DLL_NAME=%PROJECT_NAME%.dll
SET BUILD_DIR=build

REM Library paths (needed to link against the engine)
SET SDL2_PATH=%cd%\vendor\SDL2
SET SDL2_IMAGE_PATH=%cd%\vendor\SDL2_image

REM =================================================================================
REM === STAGE 1: VALIDATE PROJECT STRUCTURE                                     ===
REM =================================================================================
echo [Sandbox Build] Validating project structure at '%PROJECT_DIR%'...

REM Check for the main project directory.
if not exist "%PROJECT_DIR%" (
    echo [Sandbox Build] FAILED: Project directory not found at '%PROJECT_DIR%'.
    exit /b 1
)

REM Check for the main project file.
if not exist "%PROJECT_DIR%\%PROJECT_NAME%.salixproj" (
    echo [Sandbox Build] FAILED: Project file '%PROJECT_NAME%.salixproj' not found.
    exit /b 1
)

REM Check for the core asset folders.
if not exist "%PROJECT_DIR%\Assets" ( echo [Sandbox Build] FAILED: 'Assets' folder not found. & exit /b 1)
if not exist "%PROJECT_DIR%\Assets\Scenes" ( echo [Sandbox Build] FAILED: 'Assets\Scenes' folder not found. & exit /b 1)
if not exist "%SCRIPTS_DIR%" ( echo [Sandbox Build] FAILED: 'Assets\Scripts' folder not found. & exit /b 1)

echo [Sandbox Build] Project structure is valid.

REM =================================================================================
REM === STAGE 2: COMPILE THE SANDBOX SCRIPTS INTO A DLL                       ===
REM =================================================================================

REM Find all C++ source files in the project's scripts folder.
SET SANDBOX_SOURCES=
FOR /R "%SCRIPTS_DIR%" %%f IN (*.cpp) DO (
    SET SANDBOX_SOURCES=!SANDBOX_SOURCES! %%f
)

REM Check if there are any scripts to compile.
IF NOT DEFINED SANDBOX_SOURCES (
    echo [Sandbox Build] WARNING: No .cpp files found in '%SCRIPTS_DIR%'. Nothing to compile.
    exit /b 0
)

REM --- Compile the DLL ---
echo [Sandbox Build] Compiling sources into %DLL_NAME%...
cl.exe ^
    /nologo /EHsc /W4 /std:c++17 ^
    /wd4251 ^
    /Fo"%BUILD_DIR%\\" ^
    /I "%SDL2_PATH%\include" ^
    /I "%SDL2_IMAGE_PATH%\include" ^
    /I "src" ^
    /LD ^
    /Fe"%BUILD_DIR%\%DLL_NAME%" ^
    %SANDBOX_SOURCES% ^
    /link ^
    /LIBPATH:"%BUILD_DIR%" ^
    SalixGameStudio.lib

REM --- Exit and Return Error Code ---
IF %ERRORLEVEL% EQU 0 (
    echo [Sandbox Build] Succeeded!
) ELSE (
    echo [Game Build] FAILED!
)
exit /b %ERRORLEVEL%