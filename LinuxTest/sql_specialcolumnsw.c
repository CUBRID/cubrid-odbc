#include <stdio.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "test_util.h"

/*
 * usage:
 *  1. do some grant to 'table_name'
 *  2. run
 *       sql_tableprivilegesw dsn table_name
 */

#define ID_LEN 255

int
sql_specialcolumnsw (int case_num, char *dsn)
{
	RETCODE retcode;
	SQLHENV           hEnv;
	SQLHDBC           hDbc;
	SQLHSTMT  hstmt;
	wchar_t *dsn_buf;
	SQLCHAR *table = "db_class";
	SQLWCHAR *table_buf;

	SQLSMALLINT scope;
	SQLCHAR     columname[ID_LEN];
	SQLCHAR     typename[ID_LEN];
	SQLSMALLINT pseudo_column;

	SQLCHAR *m_columname, *m_typename;

	SQLLEN len_scope, len_columname, len_typename, len_pseudo_column;

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

	SQLBindCol(hstmt, 1, SQL_C_SHORT, &scope, sizeof(scope), &len_scope);
	SQLBindCol(hstmt, 2, SQL_C_WCHAR,  columname, sizeof(columname), &len_columname);
	SQLBindCol(hstmt, 4, SQL_C_WCHAR, typename, ID_LEN, &len_typename);
	SQLBindCol(hstmt, 8, SQL_C_SHORT, &pseudo_column, sizeof(pseudo_column), &len_pseudo_column);

	retcode = SQLSpecialColumnsW (hstmt, SQL_BEST_ROWID, NULL, 0, NULL, 0, table_buf, SQL_NTS, SQL_SCOPE_CURROW, SQL_NULLABLE);
	AreNotEqual (retcode, SQL_ERROR);

	while (SQLFetch(hstmt) == SQL_SUCCESS) {
		wide_char_to_bytes (columname, SQL_NTS, &m_columname, NULL, "UCS2");
		wide_char_to_bytes (typename, SQL_NTS, &m_typename, NULL, "UCS2");
		printf("Table name = %s, scope = %d, column = %s, type = %s, pseudo_column = %d\n", table, scope, m_columname, m_typename, pseudo_column);
	}

	retcode = SQLDisconnect (hDbc);
        AreNotEqual (retcode, SQL_ERROR);

	retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
	retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
	AreNotEqual (retcode, SQL_ERROR);

	return retcode;
}
