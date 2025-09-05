@echo off
SETLOCAL EnableDelayedExpansion
cls
echo [Code Coverage] Starting code coverage analysis...

REM --- Configuration ---
SET BUILD_DIR=%~dp0build
SET CONFIG=Debug
SET TEST_EXECUTABLE=%BUILD_DIR%\%CONFIG%\SalixEngine_tests.exe
SET COVERAGE_OUTPUT_DIR=%BUILD_DIR%\CodeCoverageReport
SET SOURCES_TO_COVER=%~dp0src

REM --- Step 1: Run a clean build to ensure coverage flags are applied ---
echo [Code Coverage] Performing a clean debug build...
call "%~dp0clean_build_debug.bat"
if %ERRORLEVEL% NEQ 0 (
    echo [Code Coverage] ❌ Build failed. Aborting coverage run.
    exit /b 1
)

REM --- Step 2: Run the test executable with OpenCppCoverage ---
echo [Code Coverage] Running tests and generating coverage data...
OpenCppCoverage.exe --source "%SOURCES_TO_COVER%" --export_type "html:%COVERAGE_OUTPUT_DIR%" -- "%TEST_EXECUTABLE%"

if %ERRORLEVEL% NEQ 0 (
    echo [Code Coverage] ❌ OpenCppCoverage failed to run.
    exit /b 1
)

echo [Code Coverage] ✅ Code coverage report generated successfully!
echo [Code Coverage] Open the following file in your browser:
echo %COVERAGE_OUTPUT_DIR%\index.html

ENDLOCAL
exit /b 0