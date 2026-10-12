#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#define MAXBUFLEN	640
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*
 * Tests for the drivers other than our own that sit on top of CUBRID ODBC.
 * Windows counterpart of linux_test/testcases/other-driver.
 */

#define TABLE	L"t_php_coltype"

namespace UnitTestCPP
{
  TEST_CLASS (OtherTest)
  {
    /* ext/odbc stops binding a column and streams it instead for these types. */
    static bool IsLongType (SQLLEN type)
    {
      return (type == SQL_LONGVARCHAR || type == SQL_WLONGVARCHAR
	      || type == SQL_BINARY || type == SQL_VARBINARY || type == SQL_LONGVARBINARY);
    }

    /* Unlike Assert::AreEqual this keeps going after a mismatch and counts it
       into nok, so one run reports every broken attribute instead of only the
       first - same behaviour as ReportEqual() in linux_test/test_util.h.  The
       caller asserts on nok once every column has been checked. */
    static void ReportEqual (int &nok, const wchar_t *what, SQLLEN v, SQLLEN expect)
    {
      wchar_t msg[MAXBUFLEN];

      if (v == expect)
	{
	  wsprintf (msg, L"%s: OK (%ld)", what, (long)v);
	}
      else
	{
	  wsprintf (msg, L"%s: NOK (%ld, expected %ld)", what, (long)v, (long)expect);
	  nok++;
	}
      Logger::WriteMessage (msg);
    }

    static SQLLEN ColAttr (SQLHSTMT stmt, SQLUSMALLINT col, SQLUSMALLINT field)
    {
      SQLLEN v = 0;

      SQLColAttribute (stmt, col, field, NULL, 0, NULL, &v);
      return v;
    }

public:

    /*
     * PHP-ODBC-001 - SQLColAttribute must return negative SQL type codes intact.
     *
     * Before the fix odbc_col_attribute() read every numeric IRD field through a
     * long *, while SQL_DESC_CONCISE_TYPE/SQL_DESC_TYPE are stored as short.  The
     * upper bytes of the caller's buffer stayed zero, so every negative SQL type
     * code came back as a large positive short instead - e.g. SQL_LONGVARCHAR
     * (-1) as 65535, SQL_BINARY (-2) as 65534, SQL_VARBINARY (-3) as 65533,
     * SQL_LONGVARBINARY (-4) as 65532.  ext/odbc then missed its long-type cases
     * and sized a bound buffer from SQL_DESC_DISPLAY_SIZE, which is
     * MAX_CUBRID_CHAR_LEN for those columns - a 1 GiB emalloc() per column.
     *
     * Columns below cover every CUBRID type that maps onto one of these four
     * negative SQL type codes (src/odbc_type.c odbc_type_by_cci()):
     *   STRING/VARCHAR(>8000)        -> SQL_LONGVARCHAR   (c_string, c_vc20000)
     *   SET/MULTISET/SEQUENCE(LIST)  -> SQL_LONGVARCHAR   (c_set)
     *   CLOB                         -> SQL_LONGVARCHAR   (c_clob)
     *   BIT (fixed length)           -> SQL_BINARY        (c_bit)
     *   BIT VARYING                  -> SQL_VARBINARY     (c_bitvar)
     *   BLOB                         -> SQL_LONGVARBINARY (c_blob)
     */
    TEST_METHOD (PHP_ODBC_001_ColAttributeNegativeType)
    {
      RETCODE retcode;

      SQLHENV env = SQL_NULL_HENV;
      SQLHDBC dbc = SQL_NULL_HDBC;
      SQLHSTMT stmt = SQL_NULL_HSTMT;
      SQLSMALLINT ncols = 0, i;
      int nok = 0;
      int oversized_bound = 0;
      wchar_t msg[MAXBUFLEN];

      retcode = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);
      SQLSetEnvAttr (env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
      retcode = SQLAllocHandle (SQL_HANDLE_DBC, env, &dbc);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      retcode = SQLConnect (dbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);
      SQLSetConnectAttr (dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);

      retcode = SQLAllocHandle (SQL_HANDLE_STMT, dbc, &stmt);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      SQLExecDirect (stmt, L"DROP TABLE IF EXISTS " TABLE, SQL_NTS);
      SQLFreeStmt (stmt, SQL_CLOSE);

      retcode = SQLExecDirect (stmt, L"CREATE TABLE " TABLE L" ("
				L"c_string STRING, c_vc20000 VARCHAR(20000), c_vc100 VARCHAR(100),"
				L"c_char CHAR(10), c_bitvar BIT VARYING(64), c_int INTEGER,"
				L"c_bit BIT(8), c_set SET(INTEGER), c_clob CLOB, c_blob BLOB)", SQL_NTS);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);
      SQLFreeStmt (stmt, SQL_CLOSE);

      retcode = SQLExecDirect (stmt, L"SELECT * FROM " TABLE, SQL_NTS);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      SQLNumResultCols (stmt, &ncols);
      ReportEqual (nok, L"column count", ncols, 10);

      ReportEqual (nok, L"c_string  CONCISE_TYPE", ColAttr (stmt, 1, SQL_DESC_CONCISE_TYPE), SQL_LONGVARCHAR);
      ReportEqual (nok, L"c_string  DESC_TYPE", ColAttr (stmt, 1, SQL_DESC_TYPE), SQL_LONGVARCHAR);
      ReportEqual (nok, L"c_vc20000 CONCISE_TYPE", ColAttr (stmt, 2, SQL_DESC_CONCISE_TYPE), SQL_LONGVARCHAR);
      ReportEqual (nok, L"c_vc100   CONCISE_TYPE", ColAttr (stmt, 3, SQL_DESC_CONCISE_TYPE), SQL_VARCHAR);
      ReportEqual (nok, L"c_char    CONCISE_TYPE", ColAttr (stmt, 4, SQL_DESC_CONCISE_TYPE), SQL_CHAR);
      ReportEqual (nok, L"c_bitvar  CONCISE_TYPE", ColAttr (stmt, 5, SQL_DESC_CONCISE_TYPE), SQL_VARBINARY);
      ReportEqual (nok, L"c_int     CONCISE_TYPE", ColAttr (stmt, 6, SQL_DESC_CONCISE_TYPE), SQL_INTEGER);
      ReportEqual (nok, L"c_bit     CONCISE_TYPE", ColAttr (stmt, 7, SQL_DESC_CONCISE_TYPE), SQL_BINARY);
      ReportEqual (nok, L"c_set     CONCISE_TYPE", ColAttr (stmt, 8, SQL_DESC_CONCISE_TYPE), SQL_LONGVARCHAR);
      ReportEqual (nok, L"c_clob    CONCISE_TYPE", ColAttr (stmt, 9, SQL_DESC_CONCISE_TYPE), SQL_LONGVARCHAR);
      ReportEqual (nok, L"c_blob    CONCISE_TYPE", ColAttr (stmt, 10, SQL_DESC_CONCISE_TYPE), SQL_LONGVARBINARY);

      /* The combination that actually exhausts PHP's memory limit: a column that
         ext/odbc will bind, carrying a display size it has to allocate up front. */
      for (i = 1; i <= ncols; i++)
	{
	  SQLLEN type = ColAttr (stmt, i, SQL_DESC_CONCISE_TYPE);
	  SQLLEN display_size = ColAttr (stmt, i, SQL_DESC_DISPLAY_SIZE);

	  if (!IsLongType (type) && display_size > 8000)
	    {
	      wsprintf (msg, L"  column %d: bound type %ld with display size %ld",
			(int)i, (long)type, (long)display_size);
	      Logger::WriteMessage (msg);
	      oversized_bound++;
	    }
	}
      ReportEqual (nok, L"columns needing an oversized bound buffer", oversized_bound, 0);

      SQLFreeStmt (stmt, SQL_CLOSE);
      SQLExecDirect (stmt, L"DROP TABLE " TABLE, SQL_NTS);
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);

      retcode = SQLDisconnect (dbc);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);
      SQLFreeHandle (SQL_HANDLE_DBC, dbc);
      SQLFreeHandle (SQL_HANDLE_ENV, env);

      Assert::AreEqual (0, nok);
    }
  };
}
