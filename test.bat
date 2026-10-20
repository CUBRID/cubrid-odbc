@echo off
rem Build the Windows unit tests and run them against the registered DSNs.
rem
rem   test.bat       run the tests
rem   test.bat -p    also keep the console output in unit_test*.result
rem
rem Both modes write a TRX next to this script, which
rem script\report_test_result.ps1 turns into a failures-only report.
rem
rem The ODBC drivers are built first and the DSNs are pointed at that build, so
rem a run tests the working copy rather than whatever is installed.  Registering
rem the driver *name* is machine wide, so run this elevated to have the
rem DRIVER=<name> testcases use the build as well.
rem
rem Visual Studio 2017 is the reference environment.  On a machine without it
rem - a CI runner, for instance - the latest Visual Studio is used instead and
rem MSBUILD_OPT carries whatever that build needs, e.g.
rem   set MSBUILD_OPT=/p:PlatformToolset=v143

set BATCH_HOME=%~dp0
set TEST_FAILED=0

rem drop the results of the previous run, so that a build failure cannot be
rem reported with the test results of yesterday
del /q "%BATCH_HOME%unit_test*.trx" "%BATCH_HOME%unit_test*.result" 2>nul

set "VSCOMNTOOLS=%VS2017COMNTOOLS%"
if not exist "%VSCOMNTOOLS%VsDevCmd.bat" call :FINDVS

if not exist "%VSCOMNTOOLS%VsDevCmd.bat" (
    echo "error: cannot find VsDevCmd.bat. Set VS2017COMNTOOLS or install Visual Studio."
    exit /b 1
)

set VSTESTTOOL=%VSCOMNTOOLS%..\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe

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

call "%VSCOMNTOOLS%VsDevCmd.bat"

rem The drivers are built before anything registers or runs: the DSNs have to
rem name the build under test, not whatever is installed on this machine.
set DRIVER_ANSI=%BATCH_HOME%project\build\x64_Release\cubrid_odbc.dll
set DRIVER_UNICODE=%BATCH_HOME%project\build\x64_Release_Unicode\cubrid_odbc_unicode.dll
set ODBC_STATE_ANSI=%BATCH_HOME%odbc_state_ansi.json
set ODBC_STATE_UNICODE=%BATCH_HOME%odbc_state_unicode.json

cd /d %BATCH_HOME%project
msbuild cubrid_odbc_14.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x64 %SDK_OPT% %MSBUILD_OPT%
if errorlevel 1 (
    echo "error: cannot build cubrid_odbc_14.vcxproj"
    set TEST_FAILED=1
    goto REPORT
)

msbuild cubrid_odbc_unicode_14.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x64 %SDK_OPT% %MSBUILD_OPT%
if errorlevel 1 (
    echo "error: cannot build cubrid_odbc_unicode_14.vcxproj"
    set TEST_FAILED=1
    goto REPORT
)

rem Register the drivers just built and the DSNs the tests connect through.
rem Without elevation the DSNs still load the build, but DRIVER=<name>
rem connection strings keep using the installed driver - setup_dsn.ps1 says so.
rem What is there now is saved first and put back at the end, so that a run
rem leaves the machine pointing at whatever it pointed at before.
cd /d %BATCH_HOME%
call "%BATCH_HOME%script\setup_dsn.bat" -Scope User -DriverOnly -DriverName "CUBRID Driver" -DriverDll "%DRIVER_ANSI%" -Backup "%ODBC_STATE_ANSI%"
if errorlevel 1 echo "Warning: cannot register the ANSI driver. The tests using DRIVER=CUBRID Driver may fail."

call "%BATCH_HOME%script\setup_dsn.bat" -Scope User -DriverDll "%DRIVER_UNICODE%" -Backup "%ODBC_STATE_UNICODE%"
if errorlevel 1 echo "Warning: DSN setup skipped. Run script\setup_dsn.bat from an elevated prompt if the tests fail to connect."

cd /d %BATCH_HOME%UnitTest
rem /restore pulls the MSTestV2 packages UnitTest.csproj references
msbuild UnitTest_14.sln /restore /t:Rebuild /p:Configuration=Release /p:Platform=x64 %SDK_OPT% %MSBUILD_OPT%
if errorlevel 1 (
    echo "error: cannot build UnitTest_14.sln"
    set TEST_FAILED=1
    goto CPPTEST
)

cd UnitTest\bin\x64\Release
call :RUNTEST UnitTest.dll unit_test %1

:CPPTEST
cd /d %BATCH_HOME%UnitTest-CPP
msbuild UnitTest-CPP.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 %SDK_OPT% %MSBUILD_OPT%
if errorlevel 1 (
    echo "error: cannot build UnitTest-CPP.sln"
    set TEST_FAILED=1
    goto REPORT
)

cd x64\Release
call :RUNTEST UnitTest-CPP.dll unit_test_cpp %1

:REPORT
cd /d %BATCH_HOME%

rem Hand the ODBC registration back the way it was found, in the reverse order
rem it was changed.  A file that was never written is reported and skipped.
call "%BATCH_HOME%script\setup_dsn.bat" -Restore "%ODBC_STATE_UNICODE%"
call "%BATCH_HOME%script\setup_dsn.bat" -Restore "%ODBC_STATE_ANSI%"

powershell -NoProfile -ExecutionPolicy Bypass -File "%BATCH_HOME%script\report_test_result.ps1"
if errorlevel 1 set TEST_FAILED=1

exit /b %TEST_FAILED%

rem ---------------------------------------------------------------------------

rem %1 test dll, %2 base name of the result files, %3 the -p option of this script
:RUNTEST
if "%~3"=="-p" (
    rem keep the whole console output, then print it: unlike a pipe this does
    rem not swallow the exit code of vstest.console
    call "%VSTESTTOOL%" %1 --platform:x64 /ResultsDirectory:"%BATCH_HOME%." /Logger:"trx;LogFileName=%~2.trx" > "%BATCH_HOME%%~2.result" 2>&1
    if errorlevel 1 set TEST_FAILED=1
    type "%BATCH_HOME%%~2.result"
) else (
    call "%VSTESTTOOL%" %1 --platform:x64 /ResultsDirectory:"%BATCH_HOME%." /Logger:"trx;LogFileName=%~2.trx"
    if errorlevel 1 set TEST_FAILED=1
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
