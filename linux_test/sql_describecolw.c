#include <stdio.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "test_util.h"

/*
 * usage:
 *       sql_execdirectw dsn
 *       sql_execdirectw dsn "Query with double quote"
 */

#define MAX_COLS 10
#define MAX_COL_NAME_LEN 256

int
sql_describecolw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV           hEnv;
  SQLHDBC           hDbc;
  SQLHSTMT  hstmt;
  wchar_t *dsn_buf;
  SQLCHAR *qry = "SELECT * from event";
  SQLWCHAR *query_buf;
  SQLSMALLINT num_columns;

  SQLWCHAR	ColumnName[MAX_COL_NAME_LEN];
  SQLSMALLINT	ColumnNameLen;
  SQLSMALLINT	ColumnDataType;
  SQLULEN		ColumnDataSize;
  SQLSMALLINT	ColumnDataDigits;
  SQLSMALLINT	ColumnDataNullable;
  SQLCHAR		*ColumnData;
  SQLLEN		ColumnDataLen;
  SQLCHAR		*buf;

  int id, i = 1;

  // qry = arg1;

  retcode = SQLAllocEnv (&hEnv);
  retcode = SQLSetEnvAttr (hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  retcode = SQLAllocConnect (hEnv, &hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, strlen (dsn), &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (hDbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLAllocHandle (SQL_HANDLE_STMT, hDbc, &hstmt);

  retcode = bytes_to_wide_char (qry, strlen(qry), &query_buf, 0, NULL, "UCS2");
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLExecDirectW (hstmt, query_buf, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLNumResultCols (hstmt, &num_columns);
  AreNotEqual (retcode, SQL_ERROR);

  printf ("num columns = %d\n", num_columns);
#if 1
  for (i = 0; i < num_columns; i++)
    {
      retcode = SQLDescribeColW (
			hstmt,                    // Select Statement (Prepared)
			i+1,                      // Columnn Number
			ColumnName,            // Column Name (returned)
			sizeof (ColumnName) / sizeof (SQLWCHAR),         // size of Column Name buffer
			&ColumnNameLen,        // Actual size of column name
			&ColumnDataType,       // SQL Data type of column
			&ColumnDataSize,       // Data size of column in table
			&ColumnDataDigits,     // Number of decimal digits
			&ColumnDataNullable);  // Whether column nullable
      AreNotEqual (retcode, SQL_ERROR);

      wide_char_to_bytes (ColumnName, SQL_NTS, &buf, NULL, "UCS2");
      printf ("Column [%d] = %s (name_len = %d), type = %s, size = %d, num_digit = %d, nullable = %d\n",
	      i + 1, buf, (int) ColumnNameLen, sqltype_name (ColumnDataType), ColumnDataSize, ColumnDataDigits, ColumnDataNullable);
    }

#endif

  /*
   * will print only 1st column of the table, it shoubd be type of INTEGER
   */

  while (SQLFetch (hstmt) == SQL_SUCCESS)
    {
      retcode = SQLGetData (hstmt, 1, SQL_C_LONG, &id, 0, NULL);
      printf ("\t1st col [%d] = %d\n", i++, id);
    }

  retcode = SQLDisconnect (hDbc);
  AreNotEqual (retcode, SQL_ERROR);
  retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
  AreNotEqual (retcode, SQL_ERROR);

  return retcode;
}
