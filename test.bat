set VSTestTool=%VS2017COMNTOOLS%..\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe
call "%VS2017COMNTOOLS%VsDevCmd.bat"
cd UnitTest
devenv UnitTest_14.sln /Upgrade
devenv UnitTest_14.sln /Project UnitTest\UnitTest.csproj /Rebuild "Release|x64" 
cd UnitTest\bin\x64\Release
call "%VSTestTool%" UnitTest.dll --platform:x64
