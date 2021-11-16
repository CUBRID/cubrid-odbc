CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(cubrid-odbc)

set (CMAKE_CXX_FLAGS "-g -Wall")
set (CMAKE_EXE_LINK_FLAGS "-g -m64 -Wall")
set (CMAKE_include_directories_BEFORE ON)

ADD_COMPILE_OPTIONS(-g -m64 -Wall)


INCLUDE_DIRECTORIES(cci/include)

set(CUBRID_ODBC_SRCS
	odbc_linux.c
	unicode.c
	odbc_catalog.c
	odbc_connection.c
	odbc_descriptor.c
	odbc_diag_record.c
	odbc_env.c
	odbc_interface.c
	odbc_number.c
	odbc_result.c
	odbc_statement.c
	odbc_type.c
	odbc_util.c
)

FILE(GLOB SRCS)

add_definitions(-DGCC -DLINUX -D_GNU_SOURCE -DCUBRID_ODBC_UNICODE -DSQL_WCHART_CONVERT)

LINK_DIRECTORIES(cci/linux_lib)
ADD_LIBRARY(${CMAKE_PROJECT_NAME} SHARED ${CUBRID_ODBC_SRCS})
target_link_libraries(${CMAKE_PROJECT_NAME} cascci pthread stdc++)
SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME} PROPERTIES SOVERSION "11.1")
