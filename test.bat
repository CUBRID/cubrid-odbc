call "E:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"
cd UnitTest
devenv UnitTest.sln /project UnitTest\UnitTest.csproj /rebuild "Release|x86" 
cd UnitTest\bin\x86\Release
MSTest/testcontainer:UnitTest.dll