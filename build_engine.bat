@echo off
SETLOCAL EnableDelayedExpansion
REM =================================================================================
REM Build Script for the Engine Executable
REM =================================================================================

echo [Engine Build] Starting...

REM --- Configuration ---
SET ENGINE_EXE_NAME=SalixGameStudio.exe
SET BUILD_DIR=build
SET ENGINE_SRC_DIR=src\Salix

REM Library paths
SET SDL2_PATH=%cd%\vendor\SDL2
SET SDL2_IMAGE_PATH=%cd%\vendor\SDL2_image

REM --- Find Engine Source Files ---
SET ENGINE_SOURCES=%cd%\src\main.cpp
FOR /R %ENGINE_SRC_DIR% %%f IN (*.cpp) DO (
    SET ENGINE_SOURCES=!ENGINE_SOURCES! %%f
)

REM --- Compile Engine Executable ---
echo [Engine Build] Compiling and linking %ENGINE_EXE_NAME%...
cl.exe ^
    /nologo /EHsc /W4 /std:c++17 ^
    /wd4251 ^
    /DSALIX_BUILD_ENGINE ^
    /Fo"%BUILD_DIR%\\" ^
    /Fe"%BUILD_DIR%\%ENGINE_EXE_NAME%" ^
    /I "%SDL2_PATH%\include" ^
    /I "%SDL2_IMAGE_PATH%\include" ^
    /I "src" ^
    %ENGINE_SOURCES% ^
    /link ^
    /IMPLIB:"%BUILD_DIR%\SalixGameStudio.lib" ^
    /LIBPATH:"%SDL2_PATH%\lib\x64" ^
    /LIBPATH:"%SDL2_IMAGE_PATH%\lib\x64" ^
    /LIBPATH:"%BUILD_DIR%" ^
    SDL2.lib SDL2main.lib SDL2_image.lib

REM --- Exit and Return Error Code ---
IF %ERRORLEVEL% EQU 0 (
    echo [Engine Build] Succeeded!
) ELSE (
    echo [Engine Build] FAILED!
)
exit /b %ERRORLEVEL%