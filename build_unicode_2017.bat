@echo off

set WORKSPACE=%~dp0
set INSTALL_DIRS=output_unicode
echo %INSTALL_DIRS%
set PROJECT_DIR=%WORKSPACE%\project

if "%VS2017COMNTOOLS%x" == "x" (
 echo "Please add 'VS2017COMNTOOLS' in the environment variable\n ex) C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools"
 GOTO :EOF
)

if exist %INSTALL_DIRS% (
  rmdir /s /q %INSTALL_DIRS%
)
mkdir %INSTALL_DIRS%

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