@echo off
setlocal enabledelayedexpansion

set WORKSPACE=%~dp0
set INSTALL_DIRS=%WORKSPACE%output_unicode
echo %INSTALL_DIRS%
set PROJECT_DIR=%WORKSPACE%project

set VERSION=0
set VERSION_FILE=BUILD_NUMBER

if "%VS2017COMNTOOLS%x" == "x" (
 echo "Please add 'VS2017COMNTOOLS' in the environment variable\n ex) C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools"
 GOTO :EOF
)

if exist %INSTALL_DIRS% (
  rmdir /s /q %INSTALL_DIRS%
)

mkdir %INSTALL_DIRS%

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

call "%VS2017COMNTOOLS%VsDevCmd.bat"

copy %PROJECT_DIR%\build\Win32_Release_Unicode\cubrid_odbc_unicode.dll  %INSTALL_DIRS%\cubrid_odbc32.dll
copy %PROJECT_DIR%\build\x64_Release_Unicode\cubrid_odbc_unicode.dll  %INSTALL_DIRS%\cubrid_odbc64.dll
@REM copy %PROJECT_DIR%\build\Win32_Debug_Unicode\cubrid_odbc_unicode.dll  %INSTALL_DIRS%\cubrid_odbc32_d.dll
@REM copy %PROJECT_DIR%\build\x64_Debug_Unicode\cubrid_odbc_unicode.dll  %INSTALL_DIRS%\cubrid_odbc64_d.dll

copy %WORKSPACE%\installer\installer-unicode.nsi %INSTALL_DIRS%\installer-unicode.nsi
copy %WORKSPACE%\installer\license.txt %INSTALL_DIRS%\license.txt
copy %WORKSPACE%\installer\README.txt %INSTALL_DIRS%\README.txt

makensis %INSTALL_DIRS%\installer-unicode.nsi

if %ERRORLEVEL% NEQ 0 (
  echo "error: cannot find NSIS. Are system environment variables set?"
  GOTO :EOF
)

powershell -Command "Compress-Archive -Path '%INSTALL_DIRS%\cubrid-odbc-unicode.exe' -DestinationPath '%INSTALL_DIRS%\CUBRID_ODBC-%VERSION%-unicode-win32-x64.zip'"

GOTO :EOF

:FINDEXEC
if EXIST %3 set %2=%~3
if NOT EXIST %3 for %%X in (%1) do set FOUNDINPATH=%%~$PATH:X
if defined FOUNDINPATH set %2=%FOUNDINPATH:"=%
if NOT defined FOUNDINPATH if NOT EXIST %3 echo Executable [%1] is not found & GOTO :EOF
call echo Executable [%1] is found at [%%%2%%]
