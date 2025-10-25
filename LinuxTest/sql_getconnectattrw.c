#include <stdio.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "test_util.h"

/*
 * usage:
 *  2. run
 *       sql_getconnectattrw dsn
 */

int
sql_getconnectattrw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV           henv;
  SQLHDBC           hdbc;
  SQLHSTMT  hstmt;
  wchar_t *dsn_buf;
  SQLINTEGER autocommit_mode, login_timeout, login_timeoutval = 125;

  retcode = SQLAllocEnv (&henv);
  retcode = SQLSetEnvAttr (henv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  retcode = SQLAllocConnect (henv, &hdbc);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLGetConnectAttrW (hdbc, SQL_LOGIN_TIMEOUT, &login_timeout, sizeof(SQLINTEGER), NULL);
  printf ("\tlogin timeout-1 = %d\n", login_timeout);
  retcode = SQLSetConnectAttrW (hdbc, SQL_ATTR_LOGIN_TIMEOUT, login_timeoutval, 0);

  retcode = SQLGetConnectAttrW (hdbc, SQL_LOGIN_TIMEOUT, &login_timeout, sizeof(SQLINTEGER), NULL);
  printf ("\tlogin timeout-2 = %d\n", login_timeout);

  autocommit_mode = -1;
  retcode = SQLGetConnectAttrW (hdbc, SQL_ATTR_AUTOCOMMIT, &autocommit_mode, sizeof(SQLINTEGER), NULL);
  AreNotEqual (retcode, SQL_ERROR);
  printf ("\tautocommit = %d (%s)\n", autocommit_mode,
	  autocommit_mode == SQL_AUTOCOMMIT_ON ? "autocommit" : "noautocommit");

  autocommit_mode = SQL_AUTOCOMMIT_OFF;
  retcode = SQLSetConnectAttrW (hdbc, SQL_ATTR_AUTOCOMMIT, autocommit_mode, 0);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (hdbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  autocommit_mode = -1;
  retcode = SQLGetConnectAttrW (hdbc, SQL_ATTR_AUTOCOMMIT, &autocommit_mode, sizeof(SQLINTEGER), NULL);
  AreNotEqual (retcode, SQL_ERROR);
  printf ("\tautocommit = %d (%s)\n", autocommit_mode,
	  autocommit_mode == SQL_AUTOCOMMIT_ON ? "autocommit" : "noautocommit");

  retcode = SQLDisconnect (hdbc);
  AreNotEqual (retcode, SQL_ERROR);
  retcode = SQLFreeHandle (SQL_HANDLE_DBC, hdbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, henv);
  AreNotEqual (retcode, SQL_ERROR);

  return retcode;
}

