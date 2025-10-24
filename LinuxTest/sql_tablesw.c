// 연결 처리, SQLTables 함수 호출, 결과 집합 검색을 포함하는 C 예제

#include <stdio.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "test_util.h"

/*
 * usage:
 *  2. run
 *       sql_tablesw dsn
 */

int
sql_tablesw (int case_num, char *dsn)
{
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN retcode;
    wchar_t *dsn_buf;
    SQLWCHAR table_name[254];
    SQLCHAR *buf;
    int i = 1;

    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    retcode = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    retcode = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    AreNotEqual (retcode, SQL_ERROR);
    
    bytes_to_wide_char (dsn, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");
    retcode = SQLConnectW (dbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
    AreNotEqual (retcode, SQL_ERROR);

    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    retcode = SQLTablesW(stmt, NULL, 0, NULL, 0, NULL, 0, (SQLCHAR*)"", SQL_NTS);
    AreNotEqual (retcode, SQL_ERROR);

    SQLBindCol(stmt, 3, SQL_C_WCHAR, table_name, sizeof(table_name), NULL);

    while ((retcode = SQLFetch(stmt)) == SQL_SUCCESS) {
        wide_char_to_bytes (table_name, SQL_NTS, &buf, NULL, "UCS2");
        printf("\ttable [%d]: |%s|\n", i++, buf);
	UT_FREE (buf);
    }

    AreNotEqual (retcode, SQL_ERROR);

    retcode = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    retcode = SQLDisconnect(dbc);
    AreNotEqual (retcode, SQL_ERROR);

    retcode = SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    retcode = SQLFreeHandle(SQL_HANDLE_ENV, env);
    AreNotEqual (retcode, SQL_ERROR);

    return 0;
}
