#include <stdio.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "test_util.h"

/*
 * usage:
 *       sql_statisticsw dsn table_name
 */

#define MAX_COLS 10
#define MAX_COL_NAME_LEN 256

int
sql_statisticsw (int case_num, char *dsn)
{
	RETCODE retcode;
	SQLHENV           hEnv;
	SQLHDBC           hDbc;
	SQLHSTMT  hstmt;
	wchar_t *dsn_buf;
	SQLCHAR *table = "s1";
	SQLWCHAR *table_buf;
	SQLSMALLINT num_columns;

	SQLWCHAR	szColumnName[MAX_COL_NAME_LEN];
	SQLWCHAR	szIndexName[MAX_COL_NAME_LEN];
	SQLUSMALLINT uIndexQualifier, sType;
	SQLLEN sColumnSize, sCardinality;
	SQLCHAR		*index, *column;

	int id, i = 1;

	retcode = SQLAllocEnv (&hEnv);
	retcode = SQLSetEnvAttr (hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
	retcode = SQLAllocConnect (hEnv, &hDbc);
	AreNotEqual (retcode, SQL_ERROR);

	bytes_to_wide_char (dsn, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");
	retcode = SQLConnectW (hDbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
	AreNotEqual (retcode, SQL_ERROR);

	retcode = SQLAllocHandle (SQL_HANDLE_STMT, hDbc, &hstmt);

	retcode = bytes_to_wide_char (table, SQL_NTS, &table_buf, 0, NULL, "UCS2");
	AreNotEqual (retcode, SQL_ERROR);

	retcode = SQLStatisticsW (hstmt, NULL, 0, NULL, 0, table_buf, SQL_NTS, SQL_INDEX_ALL, SQL_QUICK);
	AreNotEqual (retcode, SQL_ERROR);

	SQLBindCol(hstmt, 6, SQL_C_WCHAR, szIndexName, sizeof (szIndexName), NULL); // INDEX_NAME
	SQLBindCol(hstmt, 9, SQL_C_WCHAR, szColumnName, sizeof (szColumnName), NULL); // COLUMN_NAME
	SQLBindCol(hstmt, 11, SQL_C_SLONG, &sCardinality, 0, NULL);        // CARDINAL

	while (SQLFetch(hstmt) == SQL_SUCCESS) {
		wide_char_to_bytes (szIndexName, SQL_NTS, &index, NULL, "UCS2");
		wide_char_to_bytes (szColumnName, SQL_NTS, &column, NULL, "UCS2");
		printf("\tindex name: %s, col = %s (%s), cardinality = %d\n", index, table, column, sCardinality);
	}

	retcode = SQLDisconnect (hDbc);
        AreNotEqual (retcode, SQL_ERROR);

	retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
	retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
	AreNotEqual (retcode, SQL_ERROR);

	return retcode;
}
