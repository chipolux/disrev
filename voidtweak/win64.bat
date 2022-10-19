@echo off
set PROJECT_DIR=voidtweak
set PROJECT_EXE=voidtweak.exe

set SCRIPT_DIR=%~dp0
set VC_BIN=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build
set QT_BIN=C:\Qt\6.3.2\msvc2019_64\bin
set QT_TOOLCHAIN=%QT_BIN%\..\lib\cmake\Qt6\qt.toolchain.cmake
set CMAKE_BIN=C:\Qt\Tools\CMake_64
set QT_INSTALLER_BIN=C:\Qt\Tools\QtInstallerFramework\4.4\bin
set PATH=%QT_BIN%;%CMAKE_BIN%;%QT_INSTALLER_BIN%;%PATH%

echo Cleaning up...
rmdir build-win64 /s /q  2> nul
mkdir build-win64
cd build-win64

echo Setting up MSVC environment...
call "%VC_BIN%\vcvarsall.bat" x64

echo Compiling...
cmake -DCMAKE_TOOLCHAIN_FILE="%QT_TOOLCHAIN%" -DCMAKE_BUILD_TYPE=MinSizeRel ..\%PROJECT_DIR%
cmake --build . --config MinSizeRel

echo Pulling in dependencies...
del /f /q MinSizeRel\*.pdb
del /f /q MinSizeRel\*.lib
del /f /q MinSizeRel\*.exp
del /f /q MinSizeRel\*.manifest
windeployqt.exe ^
    --release ^
    --no-compiler-runtime ^
    --no-virtualkeyboard ^
    --no-translations ^
    --no-bluetooth ^
    --no-concurrent ^
    --no-designer ^
    --no-designercomponents ^
    --no-gamepad ^
    --no-qthelp ^
    --no-multimedia ^
    --no-multimediawidgets ^
    --no-multimediaquick ^
    --no-nfc ^
    --no-openglwidgets ^
    --no-positioning ^
    --no-printsupport ^
    --no-qmltooling ^
    --no-quickparticles ^
    --no-quickwidgets ^
    --no-script ^
    --no-scripttools ^
    --no-sensors ^
    --no-serialport ^
    --no-sql ^
    --no-svgwidgets ^
    --no-test ^
    --no-widgets ^
    --no-winextras ^
    --no-webenginecore ^
    --no-webengine ^
    --no-webenginewidgets ^
    --no-3dcore ^
    --no-3drenderer ^
    --no-3dquick ^
    --no-3dquickrenderer ^
    --no-3dinput ^
    --no-3danimation ^
    --no-3dextras ^
    --no-geoservices ^
    --no-webchannel ^
    --no-texttospeech ^
    --no-serialbus ^
    --no-webview ^
    --no-shadertools ^
    --qmldir ..\%PROJECT_DIR% ^
    MinSizeRel\%PROJECT_EXE%

:: available qt components, can be excluded with --no-<name>
::bluetooth concurrent core declarative designer designercomponents gamepad gui
::qthelp multimedia multimediawidgets multimediaquick network nfc opengl
::openglwidgets positioning printsupport qml qmltooling quick quickparticles
::quickwidgets script scripttools sensors serialport sql svg svgwidgets test
::websockets widgets winextras xml webenginecore webengine webenginewidgets 3dcore
::3drenderer 3dquick 3dquickrenderer 3dinput 3danimation 3dextras geoservices
::webchannel texttospeech serialbus webview

cd /d %SCRIPT_DIR%

echo Building installer...
del /f /q installer\VoidTweakInstaller.exe
rmdir /s /q installer\packages\com.nakyle.voidtweak\data
mkdir installer\packages\com.nakyle.voidtweak\data
robocopy build-win64\MinSizeRel installer\packages\com.nakyle.voidtweak\data /S /NFL /NDL /NJH /NJS /NP /NS /NC
binarycreator --offline-only -c installer\config\config.xml -p installer\packages installer\VoidTweakInstaller.exe

echo Done!
