set BATCH_HOME=%~dp0
set VSTestTool=%VS2017COMNTOOLS%..\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe
call "%VS2017COMNTOOLS%VsDevCmd.bat"
cd UnitTest
devenv UnitTest_14.sln /Upgrade
devenv UnitTest_14.sln /Project UnitTest\UnitTest.csproj /Rebuild "Release|x64" 
cd UnitTest\bin\x64\Release
call "%VSTestTool%" UnitTest.dll --platform:x64

cd %BATCH_HOME%\UnitTest-CPP
devenv UnitTest_14.sln /Upgrade
devenv UnitTest-CPP.sln /Project UnitTest-CPP\UnitTest-CPP.vcxproj /Rebuild "Release|x64" 
cd x64\Release
call "%VSTestTool%" UnitTest-CPP.dll --platform:x64