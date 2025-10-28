# Creating a testcase
1. create a file named sql_xxxxx.c
```   
   #include <stdio.h>
   #include <wchar.h>
   #include <sql.h>
   #include <sqlext.h>
   #include <string.h>
   #include "test_util.h"

   int
   sql_xxxxxx (int case_num, char *dsn)
     {
       RETCODE retcode;
       ...

       AreNotEqual (retcode, SQL_ERROR);
     }
```

2. add the filename in CUBRID_ODBC_TEST_SOURCES variable in CMakeLists.txt
   * this will compile sql_xxxxx.c to sql_xxxxx.o and add it to **libcubrid-odbc-test.so**
   * and, sql_xxxxx will be a testcase name.
   * odbc_test (main) will call sql_xxxxxx ().
3. build
   $ cmake CMakeLists.txt
   $ make

4. run (environments)
   * copy 'CUBRID Linux ODBC' & 'CUBRID Linux ODBC Unicode', 'CUBRID CCI' drivers in your library directroy
     $ mkdir -p $HOME/lib
     $ cp libcascci.so.11.2 $HOME/lib
     $ cp libcubrid-odbcw.so.11.4.0 $HOME/lib
     $ cp libcubrid-odbc.so.11.4.0 $HOME/lib
     $ cd $HOME/lib
     $ ln -s libcascci.so.11.2 libcascci.so
     $ ln -s libcubrid-odbc.so.11.4.0 libcubrid-odbc.so
     $ ln -s libcubrid-odbcw.so.11.4.0 libcubrid-odbcw.so
   * install unixODBC (https://www.unixodbc.org/)
   * setup unixODBC
     1. include odbc library path and your library to LD_LIBRARY_PATH
        (find unixODBC library directory shown in 'odbcinst -j' command after install unixODBC
        $ export LD_LIBRARY_PATH=/usr/local/lib:$HOME/lib:$LD_LIBRARY_PATH
     2. regist 'CUBRID Linux ODBC' and 'CUBRID Linux ODBC Unicode' driver file path to unixODBC
        to 'DRIVERS' file shown in 'odbcinst -j'
	```
	[CUBRID ODBC Driver]
	Description     = CUBRID Linux ODBC Driver
	Driver          = /home/cubrid/lib/libcubrid-odbc.so
	FileUsage       = 1

	[CUBRID ODBC Driver Unicode]
	Description     = CUBRID Linux ODBC Unicode
	Driver          = /home/cubrid/lib/libcubrid-odbcw.so
	IANAAppCodePage=1
	```
     3. add your DSN to odbc.ini
	* refer 'SYSTEM DATA SOURCES' and 'USER DATA SOURCES' files shown in 'odbcinst -j' command
        * if same DSN name shown in both system and user data sources, DSN of user data sources will be applied.
	  (/usr/local/etc/odbc.ini or $HOME/.odbc.ini)
	```
	[link2]
	Driver=CUBRID ODBC Driver
	Description=CUBRID ODBC
	DB_NAME=demodb
	UID=public
	PWD=
	SERVER=192.168.2.39
	PORT=33000
	FETCH_SIZE=1
	CHARSET=ko_KR.utf8

	[link2u]
	Driver=CUBRID ODBC Driver Unicode
	Description=CUBRID ODBC
	DB_NAME=hdb1
	UID=cubrid
	PWD=1234
	SERVER=192.168.2.39
	PORT=33000
	FETCH_SIZE=1
	CHARSET=ko_KR.utf8
	```

     $ edit 'dsn.txt' for DSN name to conneect CUBRID DB server, for example 'link2u' (default is 'test-db-server')

5. run all testcases
   * this will scan current directory and look file names like sql_xxx.x
   * and remove suffix '.x' for example, .c, .o and will use it as a testcase name
   * for a testcase 'sql_xxx', odbc_test lookup a fuction 'sql_xxx ()' is exists
   * in the testcase library previous compiled, 'libcubrid-odbc-test.so'
   * and will call with case_num and 'dsn'
   $ ./odbc_test

6. run indiviaual testcase (by sample)
   $ ./ocbc_test sql_xxxxx

