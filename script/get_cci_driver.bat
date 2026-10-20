@echo off
setlocal

REM
REM  Copyright 2008 Search Solution Corporation
REM  Copyright 2016 CUBRID Corporation
REM 
REM   Licensed under the Apache License, Version 2.0 (the "License");
REM   you may not use this file except in compliance with the License.
REM   You may obtain a copy of the License at
REM 
REM       http://www.apache.org/licenses/LICENSE-2.0
REM 
REM   Unless required by applicable law or agreed to in writing, software
REM   distributed under the License is distributed on an "AS IS" BASIS,
REM   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM   See the License for the specific language governing permissions and
REM   limitations under the License.
REM 

REM %~dp0 ends with a backslash, and a quoted path that ends with one escapes
REM the closing quote; resolve it to a plain directory name once, here
for %%i in ("%~dp0.") do set SCRIPT_DIR=%%~fi
for %%i in ("%SCRIPT_DIR%\..") do set SOURCE_DIR=%%~fi
set CCI_SUB_MODULE_DIR=%SOURCE_DIR%\cci-src
set CCI_WIN_DIR=%CCI_SUB_MODULE_DIR%\win
set CCI_SRC_DIRS=%CCI_SUB_MODULE_DIR%\src\cci
set INSTALL_DIRS=%SOURCE_DIR%\cci
set CCI_32_LIB=%CCI_WIN_DIR%\output\CUBRID_Release_Win32_V141_other\static_lib\cas_cci.lib
set CCI_32_DEBUG_LIB=%CCI_WIN_DIR%\output\CUBRID_Debug_Win32_V141_other\static_lib\cas_cci.lib
set CCI_64_LIB=%CCI_WIN_DIR%\output\CUBRID_Release_x64_V141_other\static_lib\cas_cci.lib
set CCI_64_DEBUG_LIB=%CCI_WIN_DIR%\output\CUBRID_Debug_x64_V141_other\static_lib\cas_cci.lib
set CCI_32_LIB_INSTALL=%INSTALL_DIRS%\lib32\cas_cci_14.lib
set CCI_32_DEBUG_LIB_INSTALL=%INSTALL_DIRS%\lib32\cas_cci_d_14.lib
set CCI_64_LIB_INSTALL=%INSTALL_DIRS%\lib64\cas_cci_14.lib
set CCI_64_DEBUG_LIB_INSTALL=%INSTALL_DIRS%\lib64\cas_cci_d_14.lib
set CCI_OPENSSL_LIB=%CCI_WIN_DIR%\external\openssl

echo "SCRIPT_DIR: %SCRIPT_DIR%"
echo "SOURCE_DIR: %SOURCE_DIR%"
echo "CCI_SUB_MODULE_DIR: %CCI_SUB_MODULE_DIR%"
echo "CCI_WIN_DIR: %CCI_WIN_DIR%"
echo "CCI_SRC_DIRS: %CCI_SRC_DIRS%"
echo "INSTALL_DIRS: %INSTALL_DIRS%"

if NOT EXIST "%CCI_SUB_MODULE_DIR%\BUILD_NUMBER" (
    echo "Source is not found, Submodule force update cci-src"
    git submodule update --init --recursive --force
)

if NOT EXIST "%CCI_WIN_DIR%" echo "CCI_WIN_DIR is not found" & GOTO FAILED

REM cd needs /d. Without it a path on another drive is accepted and silently
REM ignored, and every build below then runs in whatever directory we started in
cd /d "%CCI_WIN_DIR%"
if not "%ERRORLEVEL%" == "0" echo "Cannot enter %CCI_WIN_DIR%" & GOTO FAILED

call build.bat clean
if not "%ERRORLEVEL%" == "0" echo "Please check for clean build" & GOTO FAILED
call build.bat /other /32
if not "%ERRORLEVEL%" == "0" echo "Please check for 32bit V141 Relase Library." & GOTO FAILED
call build.bat /other /32 /debug
if not "%ERRORLEVEL%" == "0" echo "Please check for 32bit V141 Debug Library." & GOTO FAILED
call build.bat /other /64
if not "%ERRORLEVEL%" == "0" echo "Please check for 64bit V141 Relase Library." & GOTO FAILED
call build.bat /other /64 /debug
if not "%ERRORLEVEL%" == "0" echo "Please check for 64bit V141 Debug Library." & GOTO FAILED

REM copying into a directory that is not there would leave a file of that name
if NOT EXIST "%INSTALL_DIRS%\lib32" md "%INSTALL_DIRS%\lib32"
if NOT EXIST "%INSTALL_DIRS%\lib64" md "%INSTALL_DIRS%\lib64"
if NOT EXIST "%INSTALL_DIRS%\include" md "%INSTALL_DIRS%\include"

call :INSTALL "Release 32bit V141 Library" "%CCI_32_LIB%" "%CCI_32_LIB_INSTALL%"
if not "%ERRORLEVEL%" == "0" GOTO FAILED
call :INSTALL "Debug 32bit V141 Library" "%CCI_32_DEBUG_LIB%" "%CCI_32_DEBUG_LIB_INSTALL%"
if not "%ERRORLEVEL%" == "0" GOTO FAILED
call :INSTALL "Release 64bit V141 Library" "%CCI_64_LIB%" "%CCI_64_LIB_INSTALL%"
if not "%ERRORLEVEL%" == "0" GOTO FAILED
call :INSTALL "Debug 64bit V141 Library" "%CCI_64_DEBUG_LIB%" "%CCI_64_DEBUG_LIB_INSTALL%"
if not "%ERRORLEVEL%" == "0" GOTO FAILED

call :INSTALL "header" "%CCI_SRC_DIRS%\broker_cas_error.h" "%INSTALL_DIRS%\include"
if not "%ERRORLEVEL%" == "0" GOTO FAILED
call :INSTALL "header" "%CCI_SRC_DIRS%\cas_cci.h" "%INSTALL_DIRS%\include"
if not "%ERRORLEVEL%" == "0" GOTO FAILED
call :INSTALL "header" "%CCI_SRC_DIRS%\compat_dbtran_def.h" "%INSTALL_DIRS%\include"
if not "%ERRORLEVEL%" == "0" GOTO FAILED

call :INSTALL "OpenSSL 32bit library" "%CCI_OPENSSL_LIB%\lib_v140\libcrypto.lib" "%INSTALL_DIRS%\lib32"
if not "%ERRORLEVEL%" == "0" GOTO FAILED
call :INSTALL "OpenSSL 32bit library" "%CCI_OPENSSL_LIB%\lib_v140\libssl.lib" "%INSTALL_DIRS%\lib32"
if not "%ERRORLEVEL%" == "0" GOTO FAILED
call :INSTALL "OpenSSL 64bit library" "%CCI_OPENSSL_LIB%\lib64_v140\libcrypto.lib" "%INSTALL_DIRS%\lib64"
if not "%ERRORLEVEL%" == "0" GOTO FAILED
call :INSTALL "OpenSSL 64bit library" "%CCI_OPENSSL_LIB%\lib64_v140\libssl.lib" "%INSTALL_DIRS%\lib64"
if not "%ERRORLEVEL%" == "0" GOTO FAILED

REM The OpenSSL objects inside cas_cci and libcrypto/libssl name ossl_static.pdb,
REM which build_openssl.ps1 leaves next to those libraries. Without it the linker
REM reports LNK4099 once per object - about 1800 warnings per driver build - and
REM drops the OpenSSL debug info. It belongs beside the .lib files it describes.
call :INSTALL "OpenSSL 32bit debug symbols" "%CCI_OPENSSL_LIB%\lib_v140\ossl_static.pdb" "%INSTALL_DIRS%\lib32"
if not "%ERRORLEVEL%" == "0" GOTO FAILED
call :INSTALL "OpenSSL 64bit debug symbols" "%CCI_OPENSSL_LIB%\lib64_v140\ossl_static.pdb" "%INSTALL_DIRS%\lib64"
if not "%ERRORLEVEL%" == "0" GOTO FAILED

echo "CCI driver is installed into %INSTALL_DIRS%"
cd /d "%SCRIPT_DIR%"
exit /b 0

REM a caller has to be able to tell a failed build from a finished one, so the
REM script ends on a non zero code here instead of on whatever the last
REM echo or cd happened to leave behind
:FAILED
cd /d "%SCRIPT_DIR%"
exit /b 1

REM  %1 what it is, %2 source, %3 destination file or directory
:INSTALL
echo "Copying %~1: %~2 to %~3"
if NOT EXIST %2 echo "  source is not there: %~2" & exit /b 1
copy %2 %3
if not "%ERRORLEVEL%" == "0" echo "  copy failed: %~2 -> %~3" & exit /b 1
exit /b 0
