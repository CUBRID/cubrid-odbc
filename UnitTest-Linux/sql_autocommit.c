#include "odbc_test.h"

int
sql_autocommit (int case_num, char *dsn)
{
  RETCODE retcode;

  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT hStmt = SQL_NULL_HSTMT;
  SWORD plm_pcbErrorMsg = 0;
  SQLINTEGER diag_rec;
  SQLINTEGER autocommit;
  SQLWCHAR *dsn_buf;
  SQLCHAR *connstr=
	  "DRIVER=CUBRID ODBC Driver Unicode;DB_NAME=demodb;SERVER=192.168.2.39;PORT=33000;"
	  "UID=dba;PWD=;CHARSET=utf-8;AUTOCOMMIT=ON";
  /* Allocate an environment handle */
  SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
  /* We want ODBC 3 support */
  SQLSetEnvAttr (env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  /* Allocate a connection handle */
  retcode = SQLAllocHandle (SQL_HANDLE_DBC, env, &dbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (connstr, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");

  retcode = SQLDriverConnectW (dbc, NULL, dsn_buf, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

  if (retcode == SQL_ERROR)
    {
      SQLGetDiagField (SQL_HANDLE_DBC, dbc, 0, SQL_DIAG_NUMBER, &diag_rec, 0, &plm_pcbErrorMsg);
    }
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLGetConnectAttr (dbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, NULL);
  AreNotEqual ((int)retcode, SQL_ERROR);
  AreEqual ((int)autocommit, 1);

  SQLDisconnect (dbc);
  SQLFreeHandle (SQL_HANDLE_DBC, dbc);
  SQLFreeHandle (SQL_HANDLE_ENV, env);

  return SQL_SUCCESS;
}
