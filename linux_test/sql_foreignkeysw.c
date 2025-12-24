#include <stdio.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "test_util.h"

/*
 * usage:
 *  1. create fk on table_name with pk
 *  2. run
 *       sql_foreignkeysw dsn pk_table_name
 */

#define MAX_COLS 10
#define MAX_COL_NAME_LEN 256

int
sql_foreignkeysw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV           hEnv;
  SQLHDBC           hDbc;
  SQLHSTMT  hstmt;
  wchar_t *dsn_buf;
  SQLCHAR *table = "event";
  SQLWCHAR *table_buf;
  SQLSMALLINT num_columns;

  SQLWCHAR	szForeignKeyTable[MAX_COL_NAME_LEN];
  SQLWCHAR	szForeignKeyColumn[MAX_COL_NAME_LEN];
  SQLWCHAR	szForeignKey[MAX_COL_NAME_LEN];
  SQLCHAR		*fk_table_name, *column, *fk;

  int id, i = 1;

  retcode = SQLAllocEnv (&hEnv);
  retcode = SQLSetEnvAttr (hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  retcode = SQLAllocConnect (hEnv, &hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, strlen (dsn), &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (hDbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLAllocHandle (SQL_HANDLE_STMT, hDbc, &hstmt);

  retcode = bytes_to_wide_char (table, SQL_NTS, &table_buf, 0, NULL, "UCS2");
  AreNotEqual (retcode, SQL_ERROR);

  SQLBindCol (hstmt, 7, SQL_C_WCHAR, szForeignKeyTable, sizeof (szForeignKeyTable), NULL);
  SQLBindCol (hstmt, 8, SQL_C_WCHAR, szForeignKeyColumn, sizeof (szForeignKeyColumn), NULL);
  SQLBindCol (hstmt, 12, SQL_C_WCHAR, szForeignKey, sizeof (szForeignKey), NULL);

  retcode = SQLForeignKeysW (hstmt, NULL, 0, NULL, 0, table_buf, SQL_NTS, NULL, 0, NULL, 0, NULL, 0);
  AreNotEqual (retcode, SQL_ERROR);

  while (SQLFetch (hstmt) == SQL_SUCCESS)
    {
      wide_char_to_bytes (szForeignKeyTable, SQL_NTS, &fk_table_name, NULL, "UCS2");
      wide_char_to_bytes (szForeignKeyColumn, SQL_NTS, &column, NULL, "UCS2");
      wide_char_to_bytes (szForeignKey, SQL_NTS, &fk, NULL, "UCS2");
      printf ("\tTable name = %s, fk = %s (%s)\n", fk_table_name, fk, column);
    }

  retcode = SQLDisconnect (hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
  AreNotEqual (retcode, SQL_ERROR);

  return retcode;
}
