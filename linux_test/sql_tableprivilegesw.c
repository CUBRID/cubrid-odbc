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

#define MAX_COLS 10
#define MAX_COL_NAME_LEN 256

int
sql_tableprivilegesw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV           hEnv;
  SQLHDBC           hDbc;
  SQLHSTMT  hstmt;
  wchar_t *dsn_buf;
  SQLCHAR *table = "privilegetest";
  SQLWCHAR *table_buf;
  SQLSMALLINT num_columns;

  SQLWCHAR *query_buf;
  SQLCHAR *drop_tbl   = "DROP TABLE privilegetest";
  SQLCHAR *create_tbl = "CREATE TABLE privilegetest (id INT PRIMARY KEY, name VARCHAR(20))";
  SQLCHAR *grant_priv = "GRANT SELECT ON privilegetest TO public";

  SQLWCHAR	 GRANTOR [MAX_COL_NAME_LEN];
  SQLWCHAR	 GRANTEE [MAX_COL_NAME_LEN];
  SQLWCHAR	 PRIVILEGE [MAX_COL_NAME_LEN];
  SQLWCHAR	 IS_GRANTABLE [MAX_COL_NAME_LEN];

  SQLCHAR		*m_grantor, *m_grantee, *m_privilege, *m_is_grantable;
  SQLCHAR		*fk_table_name, *column, *fk;

  int id, i = 1;

  retcode = SQLAllocEnv (&hEnv);
  retcode = SQLSetEnvAttr (hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  retcode = SQLAllocConnect (hEnv, &hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (hDbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLAllocHandle (SQL_HANDLE_STMT, hDbc, &hstmt);

  bytes_to_wide_char((char *) drop_tbl, SQL_NTS, &query_buf, 0, NULL, "UCS2");
  SQLExecDirectW(hstmt, query_buf, SQL_NTS);

  bytes_to_wide_char((char *) create_tbl, SQL_NTS, &query_buf, 0, NULL, "UCS2");
  retcode = SQLExecDirectW(hstmt, query_buf, SQL_NTS);
  AreNotEqual(retcode, SQL_ERROR);

  bytes_to_wide_char((char *) grant_priv, SQL_NTS, &query_buf, 0, NULL, "UCS2");
  retcode = SQLExecDirectW(hstmt, query_buf, SQL_NTS);
  AreNotEqual(retcode, SQL_ERROR);

  retcode = bytes_to_wide_char (table, SQL_NTS, &table_buf, 0, NULL, "UCS2");
  AreNotEqual (retcode, SQL_ERROR);

  SQLBindCol (hstmt, 4, SQL_C_WCHAR, GRANTOR, sizeof (GRANTOR), NULL);
  SQLBindCol (hstmt, 5, SQL_C_WCHAR, GRANTEE, sizeof (GRANTEE), NULL);
  SQLBindCol (hstmt, 6, SQL_C_WCHAR, PRIVILEGE, sizeof (PRIVILEGE), NULL);
  SQLBindCol (hstmt, 7, SQL_C_WCHAR, IS_GRANTABLE, sizeof (IS_GRANTABLE), NULL);

  retcode = SQLTablePrivilegesW (hstmt, NULL, 0, NULL, 0, table_buf, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  while (SQLFetch (hstmt) == SQL_SUCCESS)
    {
      wide_char_to_bytes (GRANTOR, SQL_NTS, &m_grantor, NULL, "UCS2");
      wide_char_to_bytes (GRANTEE, SQL_NTS, &m_grantee, NULL, "UCS2");
      wide_char_to_bytes (PRIVILEGE, SQL_NTS, &m_privilege, NULL, "UCS2");
      wide_char_to_bytes (IS_GRANTABLE, SQL_NTS, &m_is_grantable, NULL, "UCS2");
      printf ("\tTable name = %s, GRANTOR = %s, GRANTEE = %s, PRIVILEGE = %s, IS_GRANTABLE = %s\n", table, m_grantor,
	      m_grantee, m_privilege, m_is_grantable);
    }

  bytes_to_wide_char((char *) drop_tbl, SQL_NTS, &query_buf, 0, NULL, "UCS2");
  SQLExecDirectW(hstmt, query_buf, SQL_NTS);
  AreNotEqual(retcode, SQL_ERROR);

  retcode = SQLDisconnect (hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
  AreNotEqual (retcode, SQL_ERROR);

  return retcode;
}
