@echo off
:: build.bat - A simple build script for the Salix Game Studio project
:: Clear the console first.
cls
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

cl /EHsc /std:c++17 /I vendor\SDL2\include /I vendor\SDL2_image\include /Fo:build\ /Fe:build\salix.exe src\main.cpp src\core\Engine.cpp src\rendering\SDLRenderer.cpp src\rendering\SDLTexture.cpp src\ecs\Transform.cpp src\ecs\Sprite2D.cpp src\ecs\Scene.cpp src\assets\AssetManager.cpp src\math\Vector2.cpp src\math\Vector3.cpp src\math\Color.cpp src\management\ProjectManager.cpp src\management\Project.cpp src\management\SceneManager.cpp vendor\SDL2\lib\x64\SDL2.lib vendor\SDL2\lib\x64\SDL2main.lib vendor\SDL2_image\lib\x64\SDL2_image.lib Shell32.lib /link /SUBSYSTEM:CONSOLE

:: Check if the build was successful before copying the DLL
if exist build\salix.exe (
    echo --- Copying DLL Files...
    echo --- SDL2.dll 
    copy vendor\SDL2\lib\x64\SDL2.dll build\
    echo --- SDL2_image.dll 
    copy vendor\SDL2_image\lib\x64\SDL2_image.dll build\
    echo --- Build Successful.
    echo --- Running salix.exe...
    build\salix.exe
) else (
    echo --- Build FAILED.
    exit/B
)
