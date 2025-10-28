#include "odbc_test.h"

/*
 * usage:
 *  2. run
 *       sql_getdescfieldw dsn
 */

#define ID_LEN 255

int
sql_getdescfieldw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV           hEnv;
  SQLHDBC           hDbc;
  SQLHSTMT  hstmt;
  wchar_t *dsn_buf;
  SQLHDESC hIrd;

  SQLCHAR *qry = "SELECT * from db_user";
  SQLWCHAR *query_buf;

  SQLSMALLINT desc_count = -1;

  retcode = SQLAllocEnv (&hEnv);
  retcode = SQLSetEnvAttr (hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  retcode = SQLAllocConnect (hEnv, &hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (hDbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLAllocHandle (SQL_HANDLE_STMT, hDbc, &hstmt);

  retcode = bytes_to_wide_char (qry, strlen (qry), &query_buf, 0, NULL, "UCS2");
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLExecDirectW (hstmt, query_buf, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLGetStmtAttrW (hstmt, SQL_ATTR_IMP_ROW_DESC, &hIrd, 0, NULL);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLGetDescFieldW (hIrd, 0, SQL_DESC_COUNT, &desc_count, SQL_IS_POINTER, 0);
  AreNotEqual (retcode, SQL_ERROR);

  printf ("\tNum. desc fields (SQL_DESC_COUNT) = %d\n", desc_count);

  retcode = SQLDisconnect (hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
  AreNotEqual (retcode, SQL_ERROR);

  return retcode;
}
