## CUBRID Database Management System
CUBRID is a DBMS being supported by an active community of open source developers 
and provides better performance and features necessary for Web services. 

This Software is released under Apache License 2.0 and BSD according to CUBRID components.
For brevity, CUBRID Server Engine is under Apache License 2.0 and CUBRID APIs and Connectors are under BSD License.
For details, please refer to the CUBRID License Page(http://www.cubrid.org/cubrid).

Below You will see the brief list of sections to guide You to easily get started. 

## MAJOR REFERENCES
- CUBRID Official Site: http://www.cubrid.org ,  http://www.cubrid.com
- CUBRID Development Site(Global): http://jira.cubrid.org
- CUBRID Manuals: http://www.cubrid.org/manuals 
- CUBRID ODBC Manuals (V11.4) : https://www.cubrid.org/manual/en/11.4/api/odbc.html (Eng)
  https://www.cubrid.org/manual/ko/11.4/api/odbc.html (Kor)

## DOWNLOADS and FILE REPOSITORIES
- http://www.cubrid.org/downloads
- http://ftp.cubrid.org

## HOW TO BUILD CUBRID ODBC Driver
### For Windows
#### Requirements
- Visual Studio 2017 version 15.0 or newer (Build)
- NSIS 3.08 or newer (Package)
```
C:\> cd cubrid-odbc
C:\> build_2017.bat         // Build and Package ANSI
C:\> build_unicode_2017.bat // Package Unicode

- Result File Directory
 -- ANSI : output
 -- Unicode : output_unicode
```
### For Linux
#### Requirements
- CMAKE 3.21 or newer
- GCC 8.3 or newer
- ncurses-devel (for CUBRID CCI Driver)
```
$ cd cubrid-odbc
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make package // Package
```
For installation on Linux, please refer to REAME.txt file.

GETTING HELP
============
If You encounter any difficulties with getting started, or just have some
questions, or find bugs, or have some suggestions, we kindly ask You to 
post your thoughts on CUBRID Forum at http://forum.cubrid.org.

Sincerely,
Your CUBRID Development Team.
