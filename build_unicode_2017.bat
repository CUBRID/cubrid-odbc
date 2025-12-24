@echo off

set WORKSPACE=%~dp0
set INSTALL_DIRS=output_unicode
echo %INSTALL_DIRS%

if "%VS2017COMNTOOLS%x" == "x" (
 echo "Please add 'VS2017COMNTOOLS' in the environment variable\n ex) C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools"
 GOTO :EOF
)

call "%VS2017COMNTOOLS%VsDevCmd.bat"
mkdir %INSTALL_DIRS%

copy build\Win32_Debug_Unicode\cubrid_odbc_unicode.dll  %INSTALL_DIRS%\cubrid_odbc32_d.dll
copy build\Win32_Release_Unicode\cubrid_odbc_unicode.dll  %INSTALL_DIRS%\cubrid_odbc32.dll
copy build\x64_Debug_Unicode\cubrid_odbc_unicode.dll  %INSTALL_DIRS%\cubrid_odbc64_d.dll
copy build\x64_Release_Unicode\cubrid_odbc_unicode.dll  %INSTALL_DIRS%\cubrid_odbc64.dll

copy installer\installer-unicode.nsi %INSTALL_DIRS%\installer-unicode.nsi
copy installer\license.txt %INSTALL_DIRS%\license.txt
copy installer\README.txt %INSTALL_DIRS%\README.txt

makensis %WORKSPACE%\%INSTALL_DIRS%\installer-unicode.nsi

if %ERRORLEVEL% NEQ 0 (
  echo "error: cannot find NSIS. Are system environment variables set?"
  GOTO :EOF
)