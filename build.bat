@echo off
:: build.bat - A simple build script for the Salix Game Studio project

echo --- Checking for build directory
if not exist build mkdir build

echo --- Starting build...
:: Command Breakdown:
:: cl -> The compiler
:: /EHsc -> Enable C++ Exceptions
:: /I vendor\SDL2\include -> Tell the compiler where to find SDL's header files
:: /Fo:build\ -> Tell the compiler where to the object file (main.obj)
:: /Fe:build\salix.exe -> Tell the compiler what to name the final executable
:: src\main.cpp -> The source file to compile
:: /link -> Anything after this a command for the linker
:: /LIBPATH:vendor\SDL2\lib\x64 -> Tell the linker where to find the .lib files
:: SDL2.lib SDL2main.lib -> the specific library files to link against

cl /EHsc /I vendor\SDL2\include /Fo:build\ /Fe:build\salix.exe src\main.cpp src\core\Engine.cpp src\rendering\SDLRenderer.cpp vendor\SDL2\lib\x64\SDL2.lib vendor\SDL2\lib\x64\SDL2main.lib Shell32.lib /link /SUBSYSTEM:CONSOLE

:: Check if the build was successful before copying the DLL
if exist build\salix.exe (
    echo --- Copying DLL...
    copy vendor\SDL2\lib\x64\SDL2.dll build\
    echo --- Build Successful.
) else (
    echo --- Build FAILED.
)
