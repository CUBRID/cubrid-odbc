@echo off
setlocal enabledelayedexpansion

set WORKSPACE=%~dp0
set SRC_DIR=%WORKSPACE%src
set PROJECT_DIR=%WORKSPACE%project
set INSTALL_DIRS=%WORKSPACE%output

set GIT_PATH=C:\Program Files\Git\bin\git.exe

set VERSION=0
set VERSION_FILE=BUILD_NUMBER
set INCLUDE_VERSION_FILE=odbc_version.i

if "%VS2017COMNTOOLS%x" == "x" (
 echo "Please add 'VS2017COMNTOOLS' in the environment variable\n ex) C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools"
 GOTO :EOF
)

call :FINDEXEC git.exe GIT_PATH "%GIT_PATH%"

echo Checking build number with [%WORKSPACE%\%VERSION_FILE%]...
for /f %%i IN (%WORKSPACE%\%VERSION_FILE%) DO set VERSION=%%i
if ERRORLEVEL 1 echo Cannot check build number. & GOTO :EOF
for /f "tokens=1,2,3,4 delims=." %%a IN (%WORKSPACE%\%VERSION_FILE%) DO (
  set MAJOR_VERSION=%%a
  set MINOR_VERSION=%%b
  set PATCH_VERSION=%%c
)

if EXIST "%WORKSPACE%\.git" (
  for /f "delims=" %%i in ('"%GIT_PATH%" rev-list --count --all') do set EXTRA_VERSION=0000%%i
  set EXTRA_VERSION=!EXTRA_VERSION:~-4!
) else (
  set EXTRA_VERSION=0000
)

echo Build Version is [%VERSION% (%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%EXTRA_VERSION%)]
set VERSION=%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%EXTRA_VERSION%

echo #define MAJOR_VERSION %MAJOR_VERSION% > %SRC_DIR%\%INCLUDE_VERSION_FILE%
echo #define MINOR_VERSION %MINOR_VERSION% >> %SRC_DIR%\%INCLUDE_VERSION_FILE%
echo #define PATCH_VERSION %PATCH_VERSION% >> %SRC_DIR%\%INCLUDE_VERSION_FILE%
echo #define BUILD_SERIAL_NUMBER %EXTRA_VERSION% >> %SRC_DIR%\%INCLUDE_VERSION_FILE%

echo #define VERSION_STRING "%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%EXTRA_VERSION%" >> %SRC_DIR%\%INCLUDE_VERSION_FILE%

call "%VS2017COMNTOOLS%VsDevCmd.bat"

if exist %INSTALL_DIRS% (
  rmdir /s /q %INSTALL_DIRS%
)

mkdir %INSTALL_DIRS%

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

msbuild %PROJECT_DIR%\cubrid_odbc_14.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32 %SDK_OPT%
msbuild %PROJECT_DIR%\cubrid_odbc_14.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 %SDK_OPT%
@REM msbuild %PROJECT_DIR%\cubrid_odbc_14.sln /t:Rebuild /p:Configuration=Debug /p:Platform=Win32 %SDK_OPT%
@REM msbuild %PROJECT_DIR%\cubrid_odbc_14.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64 %SDK_OPT%
 
copy %PROJECT_DIR%\build\Win32_Release\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc32.dll
copy %PROJECT_DIR%\build\x64_Release\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc64.dll
@REM copy %PROJECT_DIR%\build\Win32_Debug\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc32_d.dll
@REM copy %PROJECT_DIR%\build\x64_Debug\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc64_d.dll

copy installer\installer.nsi %INSTALL_DIRS%\installer.nsi
copy installer\license.txt %INSTALL_DIRS%\license.txt
copy installer\README.txt %INSTALL_DIRS%\README.txt

makensis %INSTALL_DIRS%\installer.nsi

if %ERRORLEVEL% NEQ 0 (
  echo "Error: cannot find NSIS. Are system environment variables set?"
  GOTO :EOF
)

powershell -Command "Compress-Archive -Path '%INSTALL_DIRS%\cubrid-odbc.exe' -DestinationPath '%INSTALL_DIRS%\CUBRID_ODBC-%VERSION%-win32-x64.zip'"
GOTO :EOF

:FINDEXEC
if EXIST %3 set %2=%~3
if NOT EXIST %3 for %%X in (%1) do set FOUNDINPATH=%%~$PATH:X
if defined FOUNDINPATH set %2=%FOUNDINPATH:"=%
if NOT defined FOUNDINPATH if NOT EXIST %3 echo Executable [%1] is not found & GOTO :EOF
call echo Executable [%1] is found at [%%%2%%]