@echo off
setlocal EnableExtensions DisableDelayedExpansion
REM Diagnose the already-built Editor test executable without overwriting Desktop logs.
REM No build, coverage, cleanup or Git mutation. Each invocation has its own CTest scope.
set "ROOT=%~dp0"
set "BUILD=%ROOT%build"
set "EXE=%BUILD%\Debug\SalixEditor_tests.exe"
set "REGISTRY=%BUILD%\src\tests\CTestTestfile.cmake"

if not exist "%EXE%" (
    echo [Editor diagnostics] ERROR: Build SalixEditor_tests first.
    exit /b 1
)
if not exist "%REGISTRY%" (
    echo [Editor diagnostics] ERROR: CTest registration missing.
    exit /b 1
)
where ctest.exe >nul 2>&1
if errorlevel 1 (
    echo [Editor diagnostics] ERROR: ctest.exe missing from PATH.
    exit /b 1
)
set "INCLUDE="
set "COUNT=0"
for %%F in ("%BUILD%\src\tests\SalixEditor_tests_include-*.cmake") do if exist "%%~fF" call :consider "%%~fF"
if not "%COUNT%"=="1" (
    echo [Editor diagnostics] ERROR: Expected exactly one current Editor discovery include; found %COUNT%.
    exit /b 1
)
set "DISCOVERED=%INCLUDE:_include-=_tests-%"
if not exist "%DISCOVERED%" (
    echo [Editor diagnostics] ERROR: Discovered Editor test file missing.
    exit /b 1
)

:choose_dir
set "OUT=%BUILD%\editor-diagnostics\run-%RANDOM%-%RANDOM%"
if exist "%OUT%\" goto :choose_dir
mkdir "%OUT%"
if errorlevel 1 exit /b 1
set "INCLUDE_CMAKE=%INCLUDE:\=/%"
> "%OUT%\CTestTestfile.cmake" echo include^("%INCLUDE_CMAKE%"^)
if errorlevel 1 exit /b 1

pushd "%ROOT%"
if errorlevel 1 exit /b 1
call :run > "%OUT%\diagnostics.txt" 2>&1
set "RC=%ERRORLEVEL%"
>> "%OUT%\diagnostics.txt" echo Finished: %DATE% %TIME% Exit: %RC%
popd
echo [Editor diagnostics] Exit: %RC%
echo [Editor diagnostics] Log: "%OUT%\diagnostics.txt"
echo [Editor diagnostics] Send this entire new directory, including Testing\Temporary\LastTest.log.
endlocal & exit /b %RC%

:consider
findstr /I /L /C:"%~nx1" "%REGISTRY%" >nul 2>&1
if errorlevel 1 exit /b 0
set "INCLUDE=%~1"
set /a COUNT+=1 >nul
exit /b 0

:run
echo Started: %DATE% %TIME%
echo Revision:
git rev-parse HEAD
git log -1 --oneline
echo Binary:
dir /a-d "%EXE%"
echo Registry: "%DISCOVERED%"
echo.
echo ==== Actual executable test listing ====
"%EXE%" --list-test-cases --no-colors
if errorlevel 1 exit /b 1
echo.
echo ==== Editor-only CTest registration ====
ctest --test-dir "%OUT%" -C Debug -N
if errorlevel 1 exit /b 1
echo.
echo ==== Editor-only isolated test execution ====
ctest --test-dir "%OUT%" -C Debug -j 1 --timeout 60 --output-on-failure --no-tests=error
exit /b %ERRORLEVEL%
