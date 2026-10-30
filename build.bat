@echo off
setlocal enabledelayedexpansion

rem Build the Windows ODBC drivers and pack them into their installers.
rem
rem   build.bat            ANSI and Unicode (default)
rem   build.bat -a         ANSI only      -> output\
rem   build.bat -u         Unicode only   -> output_unicode\
rem   build.bat -n         build only, skip the NSIS installer and the zip
rem   build.bat -h         this help
rem
rem Replaces build_2017.bat and build_unicode_2017.bat.  Both drivers live in
rem one solution, so the solution is built once and each flavour is packed from
rem what that build produced - the Unicode script used to pack without building
rem anything, which only worked when the ANSI script had just run.
rem
rem Visual Studio 2017 is the reference environment.  On a machine without it
rem the latest Visual Studio is used instead and MSBUILD_OPT carries whatever
rem that build needs, e.g.
rem   set MSBUILD_OPT=/p:PlatformToolset=v143

set WORKSPACE=%~dp0
set SRC_DIR=%WORKSPACE%src
set PROJECT_DIR=%WORKSPACE%project
set INSTALLER_DIR=%WORKSPACE%installer
set OUT_ANSI=%WORKSPACE%output
set OUT_UNICODE=%WORKSPACE%output_unicode

set GIT_PATH=C:\Program Files\Git\bin\git.exe
set VERSION=0
set VERSION_FILE=BUILD_NUMBER
set INCLUDE_VERSION_FILE=odbc_version.i

set BUILD_ANSI=1
set BUILD_UNICODE=1
set MAKE_INSTALLER=1

:PARSE
if "%~1"=="" goto PARSED
if /i "%~1"=="-a"        ( set BUILD_UNICODE=0 & shift & goto PARSE )
if /i "%~1"=="-ansi"     ( set BUILD_UNICODE=0 & shift & goto PARSE )
if /i "%~1"=="-u"        ( set BUILD_ANSI=0    & shift & goto PARSE )
if /i "%~1"=="-unicode"  ( set BUILD_ANSI=0    & shift & goto PARSE )
if /i "%~1"=="-n"        ( set MAKE_INSTALLER=0 & shift & goto PARSE )
if /i "%~1"=="-h"        ( goto USAGE )
if "%~1"=="/?"           ( goto USAGE )
echo "error: unknown option %~1"
goto USAGE
:PARSED

rem ---------------------------------------------------------- toolchain

set "VSCOMNTOOLS=%VS2017COMNTOOLS%"
if not exist "%VSCOMNTOOLS%VsDevCmd.bat" call :FINDVS

if not exist "%VSCOMNTOOLS%VsDevCmd.bat" (
    echo "error: cannot find VsDevCmd.bat. Set VS2017COMNTOOLS or install Visual Studio."
    exit /b 1
)

call :FINDEXEC git.exe GIT_PATH "%GIT_PATH%"

rem ---------------------------------------------------------- version

echo Checking build number with [%WORKSPACE%%VERSION_FILE%]...
for /f %%i IN (%WORKSPACE%%VERSION_FILE%) DO set VERSION=%%i
if ERRORLEVEL 1 (
    echo "error: cannot read %VERSION_FILE%"
    exit /b 1
)
for /f "tokens=1,2,3,4 delims=." %%a IN (%WORKSPACE%%VERSION_FILE%) DO (
    set MAJOR_VERSION=%%a
    set MINOR_VERSION=%%b
    set PATCH_VERSION=%%c
)

if EXIST "%WORKSPACE%.git" (
    for /f "delims=" %%i in ('"%GIT_PATH%" rev-list --count --all') do set EXTRA_VERSION=0000%%i
    set EXTRA_VERSION=!EXTRA_VERSION:~-4!
) else (
    set EXTRA_VERSION=0000
)

set VERSION=%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.!EXTRA_VERSION!
echo Build Version is [!VERSION!]

rem the header both drivers compile the version string from.  The Unicode
rem build needs it just as much, so it is written whichever flavour is asked for
echo #define MAJOR_VERSION %MAJOR_VERSION% > %SRC_DIR%\%INCLUDE_VERSION_FILE%
echo #define MINOR_VERSION %MINOR_VERSION% >> %SRC_DIR%\%INCLUDE_VERSION_FILE%
echo #define PATCH_VERSION %PATCH_VERSION% >> %SRC_DIR%\%INCLUDE_VERSION_FILE%
echo #define BUILD_SERIAL_NUMBER !EXTRA_VERSION! >> %SRC_DIR%\%INCLUDE_VERSION_FILE%
echo #define VERSION_STRING "!VERSION!" >> %SRC_DIR%\%INCLUDE_VERSION_FILE%

rem ---------------------------------------------------------- build

call "%VSCOMNTOOLS%VsDevCmd.bat"

rem Find latest Windows 10 SDK
set "SDK_ROOT=C:\Program Files (x86)\Windows Kits\10\Include"
set "LATEST_SDK_VERSION="
if exist "%SDK_ROOT%" (
    for /f "delims=" %%D in ('dir /b /ad /on "%SDK_ROOT%\10.*"') do set "LATEST_SDK_VERSION=%%D"
)

if "%LATEST_SDK_VERSION%"=="" (
    echo "Warning: Cannot find Windows 10 SDK. Using default."
    set SDK_OPT=
) else (
    echo "Found latest Windows 10 SDK: %LATEST_SDK_VERSION%"
    set SDK_OPT=/p:WindowsTargetPlatformVersion=%LATEST_SDK_VERSION%
)

rem cubrid_odbc_14.sln holds both the ANSI and the Unicode project, so one
rem pass over it produces every dll packed below
msbuild "%PROJECT_DIR%\cubrid_odbc_14.sln" /t:Rebuild /p:Configuration=Release /p:Platform=Win32 %SDK_OPT% %MSBUILD_OPT%
if errorlevel 1 (
    echo "error: cannot build cubrid_odbc_14.sln for Win32"
    exit /b 1
)

msbuild "%PROJECT_DIR%\cubrid_odbc_14.sln" /t:Rebuild /p:Configuration=Release /p:Platform=x64 %SDK_OPT% %MSBUILD_OPT%
if errorlevel 1 (
    echo "error: cannot build cubrid_odbc_14.sln for x64"
    exit /b 1
)

rem ---------------------------------------------------------- package

if "%BUILD_ANSI%"=="1" (
    call :PACK ansi "%OUT_ANSI%" Release cubrid_odbc installer.nsi cubrid-odbc.exe ""
    if errorlevel 1 exit /b 1
)

if "%BUILD_UNICODE%"=="1" (
    call :PACK unicode "%OUT_UNICODE%" Release_Unicode cubrid_odbc_unicode installer-unicode.nsi cubrid-odbc-unicode.exe "unicode-"
    if errorlevel 1 exit /b 1
)

echo.
echo Build complete. Version !VERSION!
if "%BUILD_ANSI%"=="1"    echo   ANSI    : %OUT_ANSI%
if "%BUILD_UNICODE%"=="1" echo   Unicode : %OUT_UNICODE%
exit /b 0

rem ---------------------------------------------------------------------------

rem %1 label, %2 output dir, %3 build sub-dir suffix, %4 dll base name,
rem %5 nsi file, %6 installer exe, %7 zip name infix
:PACK
set "P_LABEL=%~1"
set "P_OUT=%~2"
set "P_CFG=%~3"
set "P_DLL=%~4"
set "P_NSI=%~5"
set "P_EXE=%~6"
set "P_TAG=%~7"

echo.
echo ========== packing %P_LABEL% ==========

if exist "%P_OUT%" rmdir /s /q "%P_OUT%"
mkdir "%P_OUT%"

copy "%PROJECT_DIR%\build\Win32_%P_CFG%\%P_DLL%.dll" "%P_OUT%\cubrid_odbc32.dll" >nul
if errorlevel 1 (
    echo "error: cannot find Win32_%P_CFG%\%P_DLL%.dll"
    exit /b 1
)
copy "%PROJECT_DIR%\build\x64_%P_CFG%\%P_DLL%.dll" "%P_OUT%\cubrid_odbc64.dll" >nul
if errorlevel 1 (
    echo "error: cannot find x64_%P_CFG%\%P_DLL%.dll"
    exit /b 1
)

copy "%INSTALLER_DIR%\%P_NSI%" "%P_OUT%\%P_NSI%" >nul
copy "%INSTALLER_DIR%\license.txt" "%P_OUT%\license.txt" >nul
copy "%INSTALLER_DIR%\README.txt" "%P_OUT%\README.txt" >nul

if not "%MAKE_INSTALLER%"=="1" (
    echo installer skipped
    goto :eof
)

makensis "%P_OUT%\%P_NSI%"
if %ERRORLEVEL% NEQ 0 (
    echo "error: cannot find NSIS. Are system environment variables set?"
    exit /b 1
)

powershell -NoProfile -Command "Compress-Archive -Force -Path '%P_OUT%\%P_EXE%' -DestinationPath '%P_OUT%\CUBRID_ODBC-!VERSION!-%P_TAG%win32-x64.zip'"
if errorlevel 1 (
    echo "error: cannot create the zip"
    exit /b 1
)
goto :eof

rem Locate the newest Visual Studio that can build C++, for machines that do
rem not have Visual Studio 2017 installed.
:FINDVS
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :eof
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSCOMNTOOLS=%%i\Common7\Tools\"
echo "Using Visual Studio at %VSCOMNTOOLS%"
goto :eof

:FINDEXEC
if EXIST %3 set %2=%~3
if NOT EXIST %3 for %%X in (%1) do set FOUNDINPATH=%%~$PATH:X
if defined FOUNDINPATH set %2=%FOUNDINPATH:"=%
if NOT defined FOUNDINPATH if NOT EXIST %3 echo Executable [%1] is not found & GOTO :EOF
call echo Executable [%1] is found at [%%%2%%]
goto :eof

:USAGE
echo Usage: build.bat [-a ^| -u] [-n] [-h]
echo.
echo   (none)      build and pack both ANSI and Unicode
echo   -a, -ansi   ANSI only      -^> output\
echo   -u, -unicode Unicode only  -^> output_unicode\
echo   -n          build only, skip the NSIS installer and the zip
echo   -h, /?      show this help
echo.
echo Needs Visual Studio (2017 or later) and, unless -n is given, NSIS on PATH.
exit /b 1
