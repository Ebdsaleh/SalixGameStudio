@echo off
cls
REM =================================================================================
REM Master Build and Run Script for the Salix Game Studio
REM This script orchestrates the build process for all components.
REM =================================================================================

REM --- Configuration ---
SET ENGINE_EXE_NAME=SalixGameStudio.exe
SET BUILD_DIR=build

REM Library paths
SET SDL2_PATH=%cd%\vendor\SDL2
SET SDL2_IMAGE_PATH=%cd%\vendor\SDL2_image

REM --- Prepare Build Environment ---
if not exist %BUILD_DIR% mkdir %BUILD_DIR%



REM =================================================================================
REM === STAGE 1: Build the Engine EXE                                         ===
REM =================================================================================
echo.
echo [Master Build] Executing build_engine.bat...
call build_engine.bat
IF %ERRORLEVEL% NEQ 0 (
    echo. & echo ***************************************
    echo * Master Build HALTED: Engine failed to compile. *
    echo ***************************************
    goto :eof
)

REM =================================================================================
REM === STAGE 2: Build the Game DLL                                           ===
REM =================================================================================
echo.
echo [Master Build] Executing build_game.bat...
call build_game.bat
IF %ERRORLEVEL% NEQ 0 (
    echo. & echo ***************************************
    echo * Master Build HALTED: Game DLL failed to compile. *
    echo ***************************************
    goto :eof
)
REM =================== TESTING BUILDING THE PROJECT ================================
REM =================================================================================
REM === STAGE 3: Build the Sandbox Project DLL (THE NEW STEP)                   ===
REM =================================================================================
echo.
echo [Master Build] Executing build_sandbox.bat...
call build_sandbox.bat
IF %ERRORLEVEL% NEQ 0 (
    echo. & echo ***************************************
    echo * Master Build HALTED: Sandbox DLL failed to compile. *
    echo ***************************************
    goto :eof
)
REM ================== END TESTING OF BUILDING THE PROJECT ==========================
REM =================================================================================
REM === STAGE 4: POST-BUILD AND RUN                                           ===
REM =================================================================================
echo.
echo [Master Build] All components built successfully!
echo.
echo Copying required DLLs...
copy "%SDL2_PATH%\lib\x64\SDL2.dll" "%BUILD_DIR%" > nul
copy "%SDL2_IMAGE_PATH%\lib\x64\*.dll" "%BUILD_DIR%" > nul
echo.
echo =============================
echo  Launching the application...
echo =============================
    
"%BUILD_DIR%\%ENGINE_EXE_NAME%"

:eof