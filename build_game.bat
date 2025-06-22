@echo off
SETLOCAL EnableDelayedExpansion
REM =================================================================================
REM Build Script for the Game Logic DLL
REM =================================================================================

echo [Game Build] Starting...

REM --- Configuration ---
SET GAME_DLL_NAME=Game.dll
SET BUILD_DIR=build
SET GAME_SRC_DIR=src\Game

REM Library paths
SET SDL2_PATH=%cd%\vendor\SDL2
SET SDL2_IMAGE_PATH=%cd%\vendor\SDL2_image

REM --- Find Game Source Files ---
SET GAME_SOURCES=
FOR /R %GAME_SRC_DIR% %%f IN (*.cpp) DO (
    SET GAME_SOURCES=!GAME_SOURCES! %%f
)

REM --- Compile Game DLL ---
echo [Game Build] Compiling sources into %GAME_DLL_NAME%...
cl.exe ^
    /nologo /EHsc /W4 /std:c++17 ^
    /wd4251 ^
    /DSALIX_BUILD_GAME ^
    /Fo"%BUILD_DIR%\\" ^
    /I "%SDL2_PATH%\include" ^
    /I "%SDL2_IMAGE_PATH%\include" ^
    /I "%cd%\vendor\nlohmann_json" ^
    /I "src" ^
    /LD ^
    /Fe"%BUILD_DIR%\%GAME_DLL_NAME%" ^
    %GAME_SOURCES% ^
    /link ^
    /LIBPATH:"%SDL2_PATH%\lib\x64" ^
    /LIBPATH:"%SDL2_IMAGE_PATH%\lib\x64" ^
    /LIBPATH:"%BUILD_DIR%" ^
    SDL2.lib SDL2_image.lib SalixGameStudio.lib

REM --- Exit and Return Error Code ---
IF %ERRORLEVEL% EQU 0 (
    echo [Game Build] Succeeded!
) ELSE (
    echo [Game Build] FAILED!
)
exit /b %ERRORLEVEL%