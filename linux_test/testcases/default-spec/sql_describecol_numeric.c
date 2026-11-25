#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "test_util.h"

/*
 * APIS-1117 - SQLDescribeCol must return the full precision of a NUMERIC
 * column as ColumnSize.
 *
 * SQL_DESC_PRECISION is stored as short.  The old code wrote it into the
 * first 2 bytes of the caller's SQLULEN and then cleared the rest through an
 * unsigned long *.  That is enough on LP64 Linux, so this testcase passes
 * here before and after the fix; it only fails on Win64 (unsigned long is
 * 4 bytes there), see APIS_1117_SQLDescribeCol_Numeric in UnitTest-CPP.
 * It is kept as a regression check.  ColumnSize is pre-filled with all bits
 * set so that any byte the driver leaves untouched shows up.
 *
 * DecimalDigits must be the column's scale.  The type table had 0 instead
 * of -1 (= use the scale) for DECIMAL/NUMERIC, so SQLDescribeCol always
 * returned 0 while SQLColAttribute(SQL_DESC_SCALE) returned the real scale.
 * Both calls are compared against each other as well.
 */

#define TABLE "t_describecol_numeric"

int
sql_describecol_numeric (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  SQLUSMALLINT i;
  int nok = 0;
  static const struct
  {
    SQLULEN precision;
    SQLSMALLINT scale;
  } expected[] = { {15, 2}, {38, 10} };

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

  retcode = SQLExecDirect (stmt, (SQLCHAR *) "CREATE TABLE " TABLE
			   " (c_num NUMERIC(15,2), c_num38 NUMERIC(38,10))", SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);
  SQLFreeStmt (stmt, SQL_CLOSE);

  retcode = SQLExecDirect (stmt, (SQLCHAR *) "SELECT * FROM " TABLE, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  for (i = 1; i <= 2; i++)
    {
      SQLCHAR col_name[256];
      SQLSMALLINT name_len = 0;
      SQLSMALLINT data_type = 0;
      SQLULEN col_size = (SQLULEN) -1;
      SQLSMALLINT decimal_digits = 0;
      SQLSMALLINT nullable = 0;
      SQLLEN attr;
      char what[64];

      retcode = SQLDescribeCol (stmt, i, col_name, sizeof (col_name), &name_len, &data_type,
				&col_size, &decimal_digits, &nullable);
      AreNotEqual (retcode, SQL_ERROR);

      snprintf (what, sizeof (what), "column %d DataType", (int) i);
      ReportEqual (nok, what, data_type, SQL_NUMERIC);

      snprintf (what, sizeof (what), "column %d ColumnSize", (int) i);
      ReportEqual (nok, what, col_size, expected[i - 1].precision);

      snprintf (what, sizeof (what), "column %d DecimalDigits", (int) i);
      ReportEqual (nok, what, decimal_digits, expected[i - 1].scale);

      attr = 0;
      SQLColAttribute (stmt, i, SQL_DESC_PRECISION, NULL, 0, NULL, &attr);
      snprintf (what, sizeof (what), "column %d ColAttribute PRECISION", (int) i);
      ReportEqual (nok, what, attr, col_size);

      attr = 0;
      SQLColAttribute (stmt, i, SQL_DESC_SCALE, NULL, 0, NULL, &attr);
      snprintf (what, sizeof (what), "column %d ColAttribute SCALE", (int) i);
      ReportEqual (nok, what, attr, decimal_digits);
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
