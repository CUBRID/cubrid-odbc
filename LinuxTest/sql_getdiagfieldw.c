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

int
sql_getdiagfieldw (int case_num, char *dsn)
{
	RETCODE retcode;
	SQLHENV           hEnv;
	SQLHDBC           hDbc;
	SQLHSTMT  hstmt;
	wchar_t *dsn_buf;
	SQLCHAR *qry = "SELECT * from db_user";
	SQLWCHAR *query_buf;
	int id, i = 1;
	SQLLEN	status;
	SQLSMALLINT statuslen;

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

	/*
 	 * will print only 1st column of the table, it shoubd be type of INTEGER
 	 */

	while (SQLFetch (hstmt) == SQL_SUCCESS)
	  {
	    retcode = SQLGetData (hstmt, 1, SQL_C_LONG, &id, 0, NULL);
	    printf ("1st col [%d] = %d\n", i++, id);
	    retcode = SQLGetDiagFieldW (SQL_HANDLE_STMT, hstmt, 1, 4, &status, SQL_INTEGER, &statuslen);
	    printf ("SQLGetDiagFieldW: ret = %d, status = %d, status_len = %d\n", retcode, status, statuslen);
	  }

	retcode = SQLDisconnect (hDbc);
	AreNotEqual (retcode, SQL_ERROR);
	retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
	retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
	AreNotEqual (retcode, SQL_ERROR);

	return retcode;
}
