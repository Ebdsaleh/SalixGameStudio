@echo off
SETLOCAL EnableDelayedExpansion
REM Clear the console.
cls
REM =================================================================================
REM Build script for the SalixGameStudio Project
REM =================================================================================

REM --- Configuration ---
SET EXECUTABLE_NAME=SalixGameStudio.exe
SET BUILD_DIR=build

REM Set paths to your SDL2 libraries.
REM !!! IMPORTANT: Change version numbers to match your folders in 'vendor' !!!
SET SDL2_PATH=%cd%\vendor\SDL2
SET SDL2_IMAGE_PATH=%cd%\vendor\SDL2_image


REM --- Prepare Build Environment ---
echo Cleaning up old build files...
if exist %BUILD_DIR% (
    echo Build directory already exists.
) else (
    mkdir %BUILD_DIR%
)


REM --- Find all C++ source files ---
echo Finding source files...
SET SOURCES=src\main.cpp
FOR /R src\Salix\ %%f IN (*.cpp) DO (
    SET SOURCES=!SOURCES! %%f
)
echo Sources to compile: %SOURCES%
echo.

REM --- Compile and Link ---
echo Compiling and linking...

REM The main compiler command.
cl.exe ^
    /nologo /EHsc /W4 /std:c++17 ^
    /Fo"%BUILD_DIR%\\" ^
    /Fe%BUILD_DIR%\%EXECUTABLE_NAME% ^
    /I "%SDL2_PATH%\include" ^
    /I "%SDL2_IMAGE_PATH%\include" ^
    /I "src" ^
    %SOURCES% ^
    /link ^
    /LIBPATH:"%SDL2_PATH%\lib\x64" ^
    /LIBPATH:"%SDL2_IMAGE_PATH%\lib\x64" ^
    SDL2.lib SDL2main.lib SDL2_image.lib

REM --- Post-Build ---
IF %ERRORLEVEL% EQU 0 (
    echo.
    echo =============================
    echo  Build Succeeded!
    echo  Executable is at: %BUILD_DIR%\%EXECUTABLE_NAME%
    echo =============================
    echo Now copying required DLLs to the build directory...
    copy "%SDL2_PATH%\lib\x64\SDL2.dll" "%BUILD_DIR%"
    copy "%SDL2_IMAGE_PATH%\lib\x64\SDL2_image.dll" "%BUILD_DIR%"
    copy "%SDL2_IMAGE_PATH%\lib\x64\*.dll" "%BUILD_DIR%"
    
    REM --- RUN THE GAME ---
    "%BUILD_DIR%\%EXECUTABLE_NAME%"
) ELSE (
    echo.
    echo ****************************
    echo * Build Failed!            *
    echo ****************************
)