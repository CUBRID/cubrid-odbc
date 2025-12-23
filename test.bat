set BATCH_HOME=%~dp0
set VSTESTTOOL=%VS2017COMNTOOLS%..\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe

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

call "%VS2017COMNTOOLS%VsDevCmd.bat"
cd UnitTest
msbuild UnitTest_14.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 %SDK_OPT%
cd UnitTest\bin\x64\Release
 
if "%1"=="-p" (
  powershell -Command "& '%VSTESTTOOL%' UnitTest.dll --platform:x64 | Tee-Object -FilePath '%BATCH_HOME%..\unit_test.result'"
) else (
  call "%VSTESTTOOL%" UnitTest.dll --platform:x64
)
 
cd %BATCH_HOME%UnitTest-CPP
msbuild UnitTest-CPP.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 %SDK_OPT%
cd x64\Release
 
if "%1"=="-p" (
  powershell -Command "& '%VSTESTTOOL%' UnitTest-CPP.dll --platform:x64 | Tee-Object -FilePath '%BATCH_HOME%..\unit_test_cpp.result'"
) else (
  call "%VSTESTTOOL%" UnitTest-CPP.dll --platform:x64
)