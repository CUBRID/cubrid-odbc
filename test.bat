call "%VS2017COMNTOOLS%VsDevCmd.bat"
cd UnitTest
devenv UnitTest_14.sln /project UnitTest\UnitTest.csproj /rebuild "Release|Any CPU" 
cd UnitTest\bin\Release
MSTest/testcontainer:UnitTest.dll