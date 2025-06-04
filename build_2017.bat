@echo off
setlocal enabledelayedexpansion

set WORKSPACE=%~dp0
set INSTALL_DIRS=output

set GIT_PATH=C:\Program Files\Git\bin\git.exe

set VERSION=0
set VERSION_FILE=BUILD_NUMBER
set INCLUDE_VERSION_FILE=odbc_version.i
set BUILD_NUMBER=0

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

echo #define MAJOR_VERSION %MAJOR_VERSION% > %WORKSPACE%\%INCLUDE_VERSION_FILE%
echo #define MINOR_VERSION %MINOR_VERSION% >> %WORKSPACE%\%INCLUDE_VERSION_FILE%
echo #define PATCH_VERSION %PATCH_VERSION% >> %WORKSPACE%\%INCLUDE_VERSION_FILE%
echo #define BUILD_SERIAL_NUMBER %EXTRA_VERSION% >> %WORKSPACE%\%INCLUDE_VERSION_FILE%

echo #define VERSION_STRING "%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%EXTRA_VERSION%" >> %WORKSPACE%\%INCLUDE_VERSION_FILE%

call "%VS2017COMNTOOLS%VsDevCmd.bat"
mkdir %INSTALL_DIRS%

devenv cubrid_odbc_14.sln /rebuild "Release|Win32"
devenv cubrid_odbc_14.sln /rebuild "Release|x64"
devenv cubrid_odbc_14.sln /rebuild "Debug|Win32"
devenv cubrid_odbc_14.sln /rebuild "Debug|x64"
 
copy build\Win32_Debug\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc32_d.dll
copy build\Win32_Release\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc32.dll
copy build\x64_Debug\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc64_d.dll
copy build\x64_Release\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc64.dll

copy installer\installer.nsi %INSTALL_DIRS%\installer.nsi
copy installer\license.txt %INSTALL_DIRS%\license.txt
copy installer\README.txt %INSTALL_DIRS%\README.txt

makensis %WORKSPACE%\%INSTALL_DIRS%\installer.nsi

GOTO :EOF

if %ERRORLEVEL% NEQ 0 (
  echo "Error: cannot find NSIS. Are system environment variables set?"
  GOTO :EOF
)

:FINDEXEC
if EXIST %3 set %2=%~3
if NOT EXIST %3 for %%X in (%1) do set FOUNDINPATH=%%~$PATH:X
if defined FOUNDINPATH set %2=%FOUNDINPATH:"=%
if NOT defined FOUNDINPATH if NOT EXIST %3 echo Executable [%1] is not found & GOTO :EOF
call echo Executable [%1] is found at [%%%2%%]
GOTO :EOF