```
Copyright 2016 CUBRID Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

# Requirements
- unixODBC (https://www.unixodbc.org)
- CUBRID CCI Driver (https://github.com/CUBRID/cubrid-cci)

# Build CUBRID Linux ODBC Driver
- The build process will also build the CUBRID cci driver used by CUBRID Linux ODBC.
```
$ cd cubrid-odbc
$ git submodule init
$ git submodule update
$ cd cci-src
$ sh build.sh
$ cd ..
$ cmake CMakeLists.txt
$ make
```

# Install binaries
1. downloads/build install unixODBC
2. build CUBRID Linux ODBC
3. copy compiled CUBRID LinuxODBC to destination directory (eg. $HOME/lib)
```
$ cp libcubrid-odbc.so $HOME/lib
$ cp libcubrid-odbc.so.11.4.0 $HOME/lib

$ cp cci-src/build_x86_64_release/cci/libcascci.so $HOME/lib
$ cp cci-src/build_x86_64_release/cci/libcascci.so.11.2 $HOME/lib

$ export LD_LIBRARY_PATH=$HOME/lib:$LD_LIBRARY_PATH
```

# configure unixODBC
1. Find unixODBC driver configuration file
```
$ odbcinst -j
unixODBC 2.3.9
DRIVERS............: /usr/local/unixODBC/etc/odbcinst.ini
SYSTEM DATA SOURCES: /usr/local/unixODBC/etc/odbc.ini
FILE DATA SOURCES..: /usr/local/unixODBC/etc/ODBCDataSources
USER DATA SOURCES..: /home/kshan/.odbc.ini
SQLULEN Size.......: 8
SQLLEN Size........: 8
SQLSETPOSIROW Size.: 8
```

2. register CUBRID Linux ODBC driver (find DRIVER path from previous command output)
```
$ vi /usr/local/unixODBC/etc/odbcinst.ini
[CUBRID]
Description     = ODBC for CUBRID
Driver          = /home/cubrid/lib/libcubrid-odbc.so
FileUsage       = 1
```

3. edit .odbc.ini for connecting CUBRID Database Server (2 options are available)
- use predefined CUBRID ODBC Driver name registered in /usr/local/unixODBC/etc/odbcinst.ini
- link2 is 'DSN' name of remote CUBRID Database Server
```
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

- use CUBRID Linux ODBC library path directly
```
$ vi ~/.odbc.ini
[LINK2]
Driver=/home/cubrid/lib/libcubrid-odbc.so
Description=CUBRID ODBC
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
