set BATCH_HOME=%~dp0
set VSTESTTOOL=%VS2017COMNTOOLS%..\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe
call "%VS2017COMNTOOLS%VsDevCmd.bat"
cd UnitTest
devenv UnitTest_14.sln /Project UnitTest\UnitTest.csproj /Rebuild "Release|x64" 
cd UnitTest\bin\x64\Release
 
if "%1"=="-p" (
  powershell -Command "& '%VSTESTTOOL%' UnitTest.dll --platform:x64 | Tee-Object -FilePath '%BATCH_HOME%..\unit_test.result'"
) else (
  call "%VSTESTTOOL%" UnitTest.dll --platform:x64
)
 
cd %BATCH_HOME%UnitTest-CPP
devenv UnitTest-CPP.sln /Upgrade
devenv UnitTest-CPP.sln /Project UnitTest-CPP\UnitTest-CPP.vcxproj /Rebuild "Release|x64" 
cd x64\Release
 
if "%1"=="-p" (
  powershell -Command "& '%VSTESTTOOL%' UnitTest-CPP.dll --platform:x64 | Tee-Object -FilePath '%BATCH_HOME%..\unit_test_cpp.result'"
) else (
  call "%VSTESTTOOL%" UnitTest-CPP.dll --platform:x64
)