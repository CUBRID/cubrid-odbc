set WORKSPACE=%~dp0
set INSTALL_DIRS=output
echo %INSTALL_DIRS%

call "E:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"
mkdir %INSTALL_DIRS%

devenv cubrid_odbc.sln /project cubrid_odbc.vcproj /rebuild "release|Win32" 
devenv cubrid_odbc.sln /project cubrid_odbc.vcproj /rebuild "debug|Win32" 
devenv cubrid_odbc.sln /project cubrid_odbc.vcproj /rebuild "release|x64" 
devenv cubrid_odbc.sln /project cubrid_odbc.vcproj /rebuild "debug|x64" 

 
copy build\Win32_Debug\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc32_d.dll
copy build\Win32_Release\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc32.dll
copy build\x64_Debug\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc64_d.dll
copy build\x64_Release\cubrid_odbc.dll  %INSTALL_DIRS%\cubrid_odbc64.dll

copy installer\installer.nsi %INSTALL_DIRS%\installer.nsi
copy installer\license.txt %INSTALL_DIRS%\license.txt
copy installer\README.txt %INSTALL_DIRS%\README.txt

c:
cd C:\NSIS

makensis %WORKSPACE%\%INSTALL_DIRS%\installer.nsi

