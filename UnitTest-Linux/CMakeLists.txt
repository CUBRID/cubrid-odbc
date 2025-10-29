#
#
#  Copyright 2016 CUBRID Corporation
# 
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# 
#
message(STATUS "=========== Linux ODBC ===========")

CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

PROJECT(cubrid-odbc-test)

find_package(Git)

set(CUBRID_ODBC_TEST_SOURCES
  test_util.c
  odbc_debug.c
  sql_connectw.c
  sql_describecolw.c
  sql_foreignkeysw.c
  sql_getdescrecw.c
  sql_primarykeysw.c
  sql_statisticsw.c
  sql_getconnectattrw.c
  sql_getdiagfieldw.c
  sql_setconnectattrw.c
  sql_tableprivilegesw.c
  sql_execdirectw.c
  sql_getdescfieldw.c
  sql_nativesqlw.c
  sql_specialcolumnsw.c
  sql_tablesw.c
  sql_autocommit.c
  )

set (CMAKE_CXX_FLAGS "-Wall")
set (CMAKE_EXE_LINK_FLAGS "-m64 -Wall")
set (CMAKE_include_directories_BEFORE ON)
set (TARGET odbc_test)

ADD_COMPILE_OPTIONS(-m64 -Wall)

LINK_DIRECTORIES(/usr/local/lib)

ADD_LIBRARY(${CMAKE_PROJECT_NAME} SHARED ${CUBRID_ODBC_TEST_SOURCES})

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DGCC -DLINUX)

add_link_options(-ldl)
target_link_libraries(${CMAKE_PROJECT_NAME} odbc pthread stdc++)

ADD_EXECUTABLE (${TARGET} odbc_test.c)
add_dependencies(${TARGET} ${CMAKE_PROJECT_NAME})

