#include <stdio.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "test_util.h"

/*
 * usage:
 *  1. run
 *       sql_connectw dsn
 */

int
sql_connectw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV           hEnv;
  SQLHDBC           hDbc;
  SQLHSTMT  hStmt;
  wchar_t *dsn_buf;

  retcode = SQLAllocEnv (&hEnv);
  retcode = SQLSetEnvAttr (hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  retcode = SQLAllocConnect (hEnv, &hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, strlen (dsn), &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (hDbc, (SQLWCHAR *) dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLDisconnect (hDbc);
  AreNotEqual (retcode, SQL_ERROR);
  retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
  AreNotEqual (retcode, SQL_ERROR);

  return retcode;
}

