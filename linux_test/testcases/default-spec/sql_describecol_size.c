#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "test_util.h"

/*
 * APIS-1117 - SQLDescribeCol ColumnSize must agree with SQLColAttribute for
 * the columns whose size comes from SQL_DESC_LENGTH (every type but NUMERIC,
 * which sql_describecol_numeric covers).
 *
 * odbc_get_desc_field() writes SQL_DESC_LENGTH as SQLULEN for
 * SQL_CHAR/SQL_VARCHAR only and as unsigned long for every other type.
 * SQLDescribeCol read it straight into the caller's SQLULEN, which leaves the
 * upper 4 bytes untouched on Win64 (unsigned long is 4 bytes there): with
 * ColumnSize pre-filled with all bits set, STRING came back as
 * 0xFFFFFFFF3FFFFFFF.  GET_STAT/METHOD results (SQL_LONGVARCHAR) take the same
 * path.  LP64 Linux is not affected, so this testcase passes here before and
 * after the fix and is kept as a regression check; the Win64 counterpart is
 * APIS_1117_SQLDescribeCol_ColumnSize in UnitTest-CPP.
 */

#define TABLE "t_describecol_size"

int
sql_describecol_size (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  SQLUSMALLINT i;
  int nok = 0;
  static const struct
  {
    const char *name;
    SQLSMALLINT type;
    SQLULEN size;		/* 0: only compared with SQLColAttribute */
  } expected[] =
  {
    {"c_char", SQL_CHAR, 10},
    {"c_vc", SQL_VARCHAR, 100},
    {"c_int", SQL_INTEGER, 0},
    {"c_str", SQL_LONGVARCHAR, 0},
    {"c_clob", SQL_LONGVARCHAR, 0},
    {"c_blob", SQL_LONGVARBINARY, 0}
  };
  const SQLUSMALLINT ncols = sizeof (expected) / sizeof (expected[0]);

  retcode = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
  retcode = SQLSetEnvAttr (env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, 0);
  retcode = SQLAllocHandle (SQL_HANDLE_DBC, env, &dbc);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLConnect (dbc, (SQLCHAR *) dsn, SQL_NTS, NULL, 0, NULL, 0);
  AreNotEqual (retcode, SQL_ERROR);
  SQLSetConnectAttr (dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_ON, 0);

  retcode = SQLAllocHandle (SQL_HANDLE_STMT, dbc, &stmt);
  AreNotEqual (retcode, SQL_ERROR);

  SQLExecDirect (stmt, (SQLCHAR *) "DROP TABLE IF EXISTS " TABLE, SQL_NTS);
  SQLFreeStmt (stmt, SQL_CLOSE);

  retcode = SQLExecDirect (stmt, (SQLCHAR *) "CREATE TABLE " TABLE " ("
			   "c_char CHAR(10), c_vc VARCHAR(100), c_int INTEGER,"
			   "c_str STRING, c_clob CLOB, c_blob BLOB)", SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);
  SQLFreeStmt (stmt, SQL_CLOSE);

  retcode = SQLExecDirect (stmt, (SQLCHAR *) "SELECT * FROM " TABLE, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  for (i = 1; i <= ncols; i++)
    {
      SQLCHAR col_name[256];
      SQLSMALLINT name_len = 0;
      SQLSMALLINT data_type = 0;
      SQLULEN col_size = (SQLULEN) -1;
      SQLSMALLINT decimal_digits = 0;
      SQLSMALLINT nullable = 0;
      SQLLEN attr;
      char what[64];
      const char *name = expected[i - 1].name;

      retcode = SQLDescribeCol (stmt, i, col_name, sizeof (col_name), &name_len, &data_type,
				&col_size, &decimal_digits, &nullable);
      AreNotEqual (retcode, SQL_ERROR);

      snprintf (what, sizeof (what), "%s DataType", name);
      ReportEqual (nok, what, data_type, expected[i - 1].type);

      if (expected[i - 1].size != 0)
	{
	  snprintf (what, sizeof (what), "%s ColumnSize", name);
	  ReportEqual (nok, what, col_size, expected[i - 1].size);
	}

      /* SQLColAttribute answers through odbc_get_desc_field_num() */
      attr = 0;
      SQLColAttribute (stmt, i, SQL_DESC_LENGTH, NULL, 0, NULL, &attr);
      snprintf (what, sizeof (what), "%s ColumnSize == ColAttribute LENGTH", name);
      ReportEqual (nok, what, col_size, attr);
    }

  SQLFreeStmt (stmt, SQL_CLOSE);
  SQLExecDirect (stmt, (SQLCHAR *) "DROP TABLE " TABLE, SQL_NTS);
  SQLFreeHandle (SQL_HANDLE_STMT, stmt);

  retcode = SQLDisconnect (dbc);
  AreNotEqual (retcode, SQL_ERROR);
  retcode = SQLFreeHandle (SQL_HANDLE_DBC, dbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, env);
  AreNotEqual (retcode, SQL_ERROR);

  return nok ? 1 : SQL_SUCCESS;
}
