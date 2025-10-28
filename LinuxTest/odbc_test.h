#ifndef ODBC_TEST_H
#define	ODBC_TEST_H
#include <stdio.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "test_util.h"

#define LINUXODBC_TESTLIB "libcubrid-odbc-test.so"
#define	DEFAULT_DSN "link2u"
#define IS_TESTCASE_LOADED(idx)       (odbc_testcases[idx].func!=NULL)
#define CASE_PREFIX "sql_"
#define	MAX_TEST_CASES 512
#define PATHMAX 1024
#define DSNFILE "dsn.txt"

RETCODE run_query_w (SQLHSTMT hstmt, SQLCHAR *query);

enum { EXISTS = 1, NOT_EXISTS = 0};
typedef struct
{
  char name[PATHMAX];
  char arg1[PATHMAX];
  char arg2[PATHMAX];
  SQLRETURN   (*func)();
} testcase_t;
#endif
