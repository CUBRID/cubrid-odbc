#include "odbc_test.h"

/*
 * usage:
 *       sql_statisticsw dsn table_name
 */

#define MAX_COLS 10
#define MAX_COL_NAME_LEN 256
#define TABLE_NAME "test_t1"

int
sql_statisticsw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV           hEnv;
  SQLHDBC           hDbc;
  SQLHSTMT  hstmt;
  wchar_t *dsn_buf;
  SQLCHAR *table = TABLE_NAME;
  SQLWCHAR *table_buf;
  SQLSMALLINT num_columns;

  SQLWCHAR	szColumnName[MAX_COL_NAME_LEN];
  SQLWCHAR	szIndexName[MAX_COL_NAME_LEN];
  SQLUSMALLINT uIndexQualifier, sType;
  SQLLEN sColumnSize, sCardinality;
  SQLCHAR		*index, *column;

  SQLCHAR *q0 = "DROP TABLE IF EXISTS " TABLE_NAME;
  SQLCHAR *q1 = "CREATE TABLE " TABLE_NAME "(col1 INT, col2 VARCHAR (100), col3 INT, col4 BIGINT, PRIMARY KEY (col1))";
  SQLCHAR *q2 = "CREATE UNIQUE INDEX idx3_test_t1 ON " TABLE_NAME "(col3)";
  SQLCHAR *q3 = "CREATE INDEX idx4_test_t1 ON " TABLE_NAME "(col4)";
  SQLCHAR *q4 = "INSERT INTO " TABLE_NAME " SELECT ROWNUM, 'test ' || ROWNUM, ROWNUM + 9999, ROWNUM + 999999 FROM "
		"db_class a, db_class b";

  int id, i = 1;

  retcode = SQLAllocEnv (&hEnv);
  retcode = SQLSetEnvAttr (hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  retcode = SQLAllocConnect (hEnv, &hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (hDbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLAllocHandle (SQL_HANDLE_STMT, hDbc, &hstmt);

  retcode = run_query_w (hstmt, q0);
  AreNotEqual (retcode, SQL_ERROR);
  retcode = run_query_w (hstmt, q1);
  AreNotEqual (retcode, SQL_ERROR);
  retcode = run_query_w (hstmt, q2);
  AreNotEqual (retcode, SQL_ERROR);
  retcode = run_query_w (hstmt, q3);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = bytes_to_wide_char (table, SQL_NTS, &table_buf, 0, NULL, "UCS2");
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLStatisticsW (hstmt, NULL, 0, NULL, 0, table_buf, SQL_NTS, SQL_INDEX_ALL, SQL_QUICK);
  AreNotEqual (retcode, SQL_ERROR);

  SQLBindCol (hstmt, 6, SQL_C_WCHAR, szIndexName, sizeof (szIndexName), NULL); // INDEX_NAME
  SQLBindCol (hstmt, 9, SQL_C_WCHAR, szColumnName, sizeof (szColumnName), NULL); // COLUMN_NAME
  SQLBindCol (hstmt, 11, SQL_C_SLONG, &sCardinality, 0, NULL);       // CARDINAL

  while (SQLFetch (hstmt) == SQL_SUCCESS)
    {
      wide_char_to_bytes (szIndexName, SQL_NTS, &index, NULL, "UCS2");
      wide_char_to_bytes (szColumnName, SQL_NTS, &column, NULL, "UCS2");
      printf ("\tindex name: %s, col = %s (%s), cardinality = %d\n", index, table, column, sCardinality);
    }

  retcode = SQLDisconnect (hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
  AreNotEqual (retcode, SQL_ERROR);

  return retcode;
}
