@echo off

set PROJECT_NAME=voidtweak
set EXECUTABLE_NAME=%PROJECT_NAME%
set INSTALLER_NAME=VoidTweakInstaller.exe
set PACKAGE_NAME=com.nakyle.voidtweak
set ZIPFILE_NAME=

echo Looking up latest Qt version...
set QT_PREFIX=C:\Qt
set QT_SUFFIX=msvc2019_64\bin
for /F "eol=| delims=" %%I in ('dir %QT_PREFIX%\6.* /AD /B /O-N 2^>nul') do (
    if exist "%QT_PREFIX%\%%I\%QT_SUFFIX%\qmake.exe" (
        set QT_BIN=%QT_PREFIX%\%%I\%QT_SUFFIX%
        goto :FOUNDQT
    )
)
goto :NOQTFOUND
:FOUNDQT

echo Looking up vcvarsall...
set VC_PREFIX=C:\Program Files (x86)\Microsoft Visual Studio\2019
set VC_SUFFIX=VC\Auxiliary\Build\vcvarsall.bat
for /F "eol=| delims=" %%I in ('dir "%VC_PREFIX%\*" /AD /B /O-N 2^>nul') do (
    if exist "%VC_PREFIX%\%%I\%VC_SUFFIX%" (
        set VC_VARS="%VC_PREFIX%\%%I\%VC_SUFFIX%"
        goto :FOUNDVC
    )
)
goto :NOVCFOUND
:FOUNDVC

if "%INSTALLER_NAME%" == "" goto :FOUNDBC
echo Looking up Qt Installer Framework...
set BC_PREFIX=%QT_PREFIX%\Tools\QtInstallerFramework
set BC_SUFFIX=bin
for /F "eol=| delims=" %%I in ('dir "%BC_PREFIX%\*" /AD /B /O-N 2^>nul') do (
    if exist "%BC_PREFIX%\%%I\%BC_SUFFIX%\binarycreator.exe" (
        set BINARYCREATOR=%BC_PREFIX%\%%I\%BC_SUFFIX%\binarycreator.exe
        goto :FOUNDBC
    )
)
goto :NOBCFOUND
:FOUNDBC

set SCRIPT_DIR=%~dp0
set QT_TOOLCHAIN=%QT_BIN%\..\lib\cmake\Qt6\qt.toolchain.cmake
set CMAKE_BIN=%QT_PREFIX%\Tools\CMake_64\bin
set CMAKE=%CMAKE_BIN%\cmake.exe
set PATH=%QT_BIN%;%CMAKE_BIN%;%PATH%

echo Setting up MSVC environment...
call %VC_VARS% x64

echo Cleaning up...
rmdir /s /q build-win64 2> nul
mkdir build-win64
cd build-win64

echo Compiling...
%CMAKE% -DCMAKE_TOOLCHAIN_FILE="%QT_TOOLCHAIN%" ^
        -DCMAKE_BUILD_TYPE=MinSizeRel ^
        ..\%PROJECT_NAME%
%CMAKE% --build . --config MinSizeRel

echo Pulling in dependencies...
del /f /q MinSizeRel\*.pdb  2> nul
del /f /q MinSizeRel\*.lib  2> nul
del /f /q MinSizeRel\*.exp  2> nul
del /f /q MinSizeRel\*.manifest  2> nul
%QT_BIN%\windeployqt.exe ^
    --release ^
    --qmldir ..\%PROJECT_NAME% ^
    MinSizeRel\%EXECUTABLE_NAME%.exe

cd /d %SCRIPT_DIR%

if not "%ZIPFILE_NAME%" == "" (
    .\tools\7za.exe a -tzip %PROJECT_NAME%.zip .\build-win64\MinSizeRel\*
)

if not "%INSTALLER_NAME%" == "" (
    echo Cleaning up installer folder...
    del /f /q installer\%INSTALLER_NAME%  2> nul
    rmdir /s /q installer\packages\%PACKAGE_NAME%\data  2> nul
    mkdir installer\packages\%PACKAGE_NAME%\data
    echo Moving in package data...
    robocopy build-win64\MinSizeRel installer\packages\%PACKAGE_NAME%\data ^
             /S /NFL /NDL /NJH /NJS /NP /NS /NC
    echo Building installer...
    %BINARYCREATOR% --offline-only ^
                    -c installer\config\config.xml ^
                    -p installer\packages ^
                    installer\%INSTALLER_NAME%
)

echo Done!
exit /b

:NOQTFOUND
echo No valid Qt install found!
exit /b -1

:NOVCFOUND
echo No valid Visual C++ install found!
exit /b -1

:NOBCFOUND
echo No valid Qt installer framework found!
exit /b -1
