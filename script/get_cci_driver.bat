@echo off

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

set SCRIPT_DIR=%~dp0
set CCI_WIN_DIR=%SCRIPT_DIR%\cubrid-cci\win\cas_cci
set CCI_SRC_DIRS=%SCRIPT_DIR%\cubrid-cci\src\cci
set INSTALL_DIRS=%SCRIPT_DIR%\..\cci

if EXIST "%SCRIPT_DIR%\cubrid-cci" rmdir /s /q cubrid-cci

git clone git@github.com:CUBRID/cubrid-cci.git

if "%VS140COMNTOOLS%x" == "x" echo "Please add 'VS140COMNTOOLS' in the environment variable\n ex) C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools"
if "%VS110COMNTOOLS%x" == "x" echo "Please add 'VS110COMNTOOLS' in the environment variable\n ex) C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools"

cd %CCI_WIN_DIR%

call "%VS140COMNTOOLS%vsvars32.bat"
devenv cas_cci_v140_lib.vcxproj /build "release|x86"
if not "%ERRORLEVEL%" == "0" echo "Please check for 32bit V140 Relase Library." & GOTO END_SCRIPT
devenv cas_cci_v140_lib.vcxproj /build "debug|x86"
if not "%ERRORLEVEL%" == "0" echo "Please check for 32bit V140 Debug Library." & GOTO END_SCRIPT
devenv cas_cci_v140_lib.vcxproj /build "release|x64"
if not "%ERRORLEVEL%" == "0" echo "Please check for 64bit V140 Relase Library." & GOTO END_SCRIPT
devenv cas_cci_v140_lib.vcxproj /build "debug|x64"
if not "%ERRORLEVEL%" == "0" echo "Please check for 64bit V140 Debug Library." & GOTO END_SCRIPT

copy %CCI_WIN_DIR%\Win32\Release\cas_cci.lib  %INSTALL_DIRS%\lib32\cas_cci_14.lib
copy %CCI_WIN_DIR%\Win32\Debug\cas_cci.lib  %INSTALL_DIRS%\lib32\cas_cci_d_14.lib
copy %CCI_WIN_DIR%\x64\Release\cas_cci.lib  %INSTALL_DIRS%\lib64\cas_cci_14.lib
copy %CCI_WIN_DIR%\x64\Debug\cas_cci.lib  %INSTALL_DIRS%\lib64\cas_cci_d_14.lib

copy %CCI_SRC_DIRS%\broker_cas_error.h  %INSTALL_DIRS%\include\
copy %CCI_SRC_DIRS%\cas_cci.h  %INSTALL_DIRS%\include\
copy %CCI_SRC_DIRS%\compat_dbtran_def.h  %INSTALL_DIRS%\include\

:END_SCRIPT
cd %SCRIPT_DIR%
