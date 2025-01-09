call "%VS2017COMNTOOLS%VsDevCmd.bat"
cd UnitTest
devenv UnitTest_14.sln /project UnitTest\UnitTest.csproj /rebuild "Release|x86" 
cd UnitTest\bin\x86\Release
MSTest/testcontainer:UnitTest.dll