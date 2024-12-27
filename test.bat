call "%VS2017COMNTOOLS%VsDevCmd.bat"
cd UnitTest
devenv UnitTest.sln /project UnitTest\UnitTest.csproj /rebuild "Release|x64" 
cd UnitTest\bin\x64\Release
MSTest/testcontainer:UnitTest.dll