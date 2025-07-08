@echo off
SETLOCAL EnableDelayedExpansion
cls
REM =================================================================================
REM Run Script for Salix Game Studio (Debug)
REM This script displays a header and launches the Debug executable.
REM =================================================================================

REM --- Console Art Header ---
echo.
echo +---------------------------------------------------------------------------------+
echo ^|                                                                                 ^|
echo ^|                        Salix Game Studio (Debug)                                ^|
echo ^|                                                                                 ^|
echo +---------------------------------------------------------------------------------+
echo.

REM --- Define the executable path ---
SET "EXECUTABLE_PATH=build\Debug\SalixGameStudio.exe"

REM --- Check if the executable exists ---
IF NOT EXIST "%EXECUTABLE_PATH%" (
    echo [ERROR] Executable not found: "%EXECUTABLE_PATH%"
    echo Please ensure you have run the clean_build_debug.bat script first.
    pause
    ENDLOCAL
    exit /b 1
)

REM --- Launch the Executable ---
echo Launching SalixGameStudio.exe (Debug)...
start "" "%EXECUTABLE_PATH%"

REM Optional: Add a pause if you want the console window to stay open after the app exits
REM pause

ENDLOCAL
exit /b 0