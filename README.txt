```
/*
 * Copyright (c) 2016 CUBRID Corporation.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */
```

# Requirements
- unixODBC (https://www.unixodbc.org)
- CUBRID CCI Driver (https://github.com/CUBRID/cubrid-cci)

# Install binaries
1. downloads/build install unixODBC
2. copy CUBRID LinuxODBC to destination directory (eg. user: cubrid , path: /home/cubrid/cubrid-odbc/lib)
```
$ mkdir -p /home/cubrid/cubrid-odbc
$ cp -rf lib/ /home/cubrid/cubrid-odbc // Copy all files ODBC, Unicode ODBC, and CCI.

3. Add to 'LD_LIBRARY_PATH'
$ export LD_LIBRARY_PATH=/home/cubrid/cubrid-odbc/lib:$LD_LIBRARY_PATH
```

# configure unixODBC
1. Find unixODBC driver configuration file
```
$ odbcinst -j
unixODBC 2.3.9
DRIVERS............: /usr/local/unixODBC/etc/odbcinst.ini
SYSTEM DATA SOURCES: /usr/local/unixODBC/etc/odbc.ini
FILE DATA SOURCES..: /usr/local/unixODBC/etc/ODBCDataSources
USER DATA SOURCES..: /home/cubrid/.odbc.ini
SQLULEN Size.......: 8
SQLLEN Size........: 8
SQLSETPOSIROW Size.: 8
```

2. register CUBRID Linux ODBC driver (find DRIVER path from previous command output)
```
//ANSI
$ vi /usr/local/unixODBC/etc/odbcinst.ini
[CUBRID]
Description     = ODBC for CUBRID
Driver          = /home/cubrid/cubrid-odbc/lib/libcubrid-odbc.so
FileUsage       = 1
```
```
//Unicode
$ vi /usr/local/unixODBC/etc/odbcinst.ini
[CUBRID_UNICODE]
Description     = UNICODE ODBC for CUBRID
Driver          = /home/cubrid/cubrid-odbc/lib/libcubrid-odbcw.so
FileUsage       = 1
```

3. edit .odbc.ini for connecting CUBRID Database Server (2 options are available)
- use predefined CUBRID ODBC Driver name registered in /usr/local/unixODBC/etc/odbcinst.ini
- link2 is 'DSN' name of remote CUBRID Database Server
```
//ANSI
$ vi ~/.odbc.ini
[LINK2]
Driver=CUBRID
Description=CUBRID ODBC
SERVER=192.168.2.39
PORT=33000
UID=cubrid
PWD=1234
FETCH_SIZE=1
CHARSET=utf8
DB_NAME=demodb
```
```
//UNICODE
$ vi ~/.odbc.ini
[LINK2]
Driver=CUBRID_UNICODE
Description=CUBRID UNICODE ODBC
SERVER=192.168.2.39
PORT=33000
UID=cubrid
PWD=1234
FETCH_SIZE=1
CHARSET=utf8
DB_NAME=demodb
```


- use CUBRID Linux ODBC library path directly
```
//ANSI
$ vi ~/.odbc.ini
[LINK2]
Driver=/home/cubrid/cubrid-odbc/lib/libcubrid-odbc.so
Description=CUBRID ODBC
SERVER=192.168.2.39
PORT=33000
UID=cubrid
PWD=1234
FETCH_SIZE=1
CHARSET=utf8
DB_NAME=demodb
```
```
//Unicode
$ vi ~/.odbc.ini
[LINK2]
Driver=/home/cubrid/cubrid-odbc/lib/libcubrid-odbcw.so
Description=CUBRID UNICODE ODBC
SERVER=192.168.2.39
PORT=33000
UID=cubrid
PWD=1234
FETCH_SIZE=1
CHARSET=utf8
DB_NAME=demodb
```

# Test
- Connection test to database demodb with DSN link2
```
$ isql link2
```
