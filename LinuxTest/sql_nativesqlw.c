#include "odbc_test.h"

/*
 * usage:
 *       sql_nativesqlw dsn
 *       sql_nativesqlw dsn "Query with double quote"
 */

int
sql_nativesqlw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV           hEnv;
  SQLHDBC           hDbc;
  SQLHSTMT  hstmt;
  wchar_t *dsn_buf;
  SQLCHAR *qry = "SELECT * from t1";
  SQLWCHAR *query_buf, OutStatementText[1024];
  SQLINTEGER TextLength2Ptr;
  SQLCHAR *m_query;
  int id, i = 1;

  retcode = SQLAllocEnv (&hEnv);
  retcode = SQLSetEnvAttr (hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  retcode = SQLAllocConnect (hEnv, &hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (hDbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = bytes_to_wide_char (qry, strlen (qry), &query_buf, 0, NULL, "UCS2");
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLNativeSqlW (hDbc, query_buf, SQL_NTS, OutStatementText, sizeof (OutStatementText), &TextLength2Ptr);
  AreNotEqual (retcode, SQL_ERROR);

  wide_char_to_bytes (OutStatementText, SQL_NTS, &m_query, NULL, "UCS2");
  printf ("\tconverted query (%d) = |%s|\n", TextLength2Ptr, m_query);

  retcode = SQLDisconnect (hDbc);
  AreNotEqual (retcode, SQL_ERROR);
  retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
  AreNotEqual (retcode, SQL_ERROR);

  return retcode;
}
