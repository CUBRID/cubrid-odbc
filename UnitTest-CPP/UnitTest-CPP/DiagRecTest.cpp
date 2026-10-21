#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#define MAXBUFLEN	640
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*
 * Tests for the diagnostic records the driver hands back.
 * Linux counterpart: linux_test/testcases/default-spec/sql_getdiagrecw.c.
 */

#define STATE_LEN	(SQL_SQLSTATE_SIZE + 1)
#define GUARD		0x5a5a

namespace UnitTestCPP
{
  TEST_CLASS (DiagRecTest)
  {
    /* Unlike Assert::AreEqual this keeps going after a mismatch and counts it
       into nok, so one run reports every broken item instead of only the
       first - same behaviour as ReportEqual() in linux_test/test_util.h. */
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

    static size_t StateLen (const SQLWCHAR *s)
    {
      size_t n = 0;

      while (s[n] != 0)
	{
	  n++;
	}
      return n;
    }

public:

    /*
     * SQLGetDiagRec must not write past the SQLSTATE buffer.
     *
     * The buffer an application hands over is SQL_SQLSTATE_SIZE + 1 characters
     * and nothing more, so this testcase puts a guard cell right behind it and
     * checks that the cell survives the call.  odbc_get_diag_rec() used to
     * strcpy() the record's sql_state into it with no bound at all, and
     * without checking it for NULL - the same record->sql_state that
     * odbc_set_diag() builds with UT_MAKE_STRING() out of whatever string its
     * caller passed, and that odbc_get_diag_field() a few lines above already
     * guards against NULL.
     */
    TEST_METHOD (DiagRec_SqlStateBufferBounds)
    {
      RETCODE retcode;

      SQLHENV env = SQL_NULL_HENV;
      SQLHDBC dbc = SQL_NULL_HDBC;
      SQLHSTMT stmt = SQL_NULL_HSTMT;
      /* exactly the buffer the spec asks for, plus a cell nothing may touch */
      SQLWCHAR sqlstate[STATE_LEN + 1];
      SQLWCHAR message[1024];
      SQLINTEGER native_error = 0;
      SQLSMALLINT text_length = -1;
      wchar_t msg[MAXBUFLEN];
      int nok = 0;

      retcode = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);
      SQLSetEnvAttr (env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
      retcode = SQLAllocHandle (SQL_HANDLE_DBC, env, &dbc);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      retcode = SQLConnect (dbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      retcode = SQLAllocHandle (SQL_HANDLE_STMT, dbc, &stmt);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      /* a table that cannot exist, so the engine is the one reporting the error */
      retcode = SQLExecDirect (stmt, L"SELECT * FROM no_such_table_for_getdiagrec", SQL_NTS);
      ReportEqual (nok, L"SQLExecDirect on a missing table fails", retcode, SQL_ERROR);

      memset (sqlstate, 0, sizeof (sqlstate));
      memset (message, 0, sizeof (message));
      sqlstate[STATE_LEN] = GUARD;

      retcode = SQLGetDiagRec (SQL_HANDLE_STMT, stmt, 1, sqlstate, &native_error,
			       message, sizeof (message) / sizeof (message[0]), &text_length);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      ReportEqual (nok, L"SQLSTATE length", (SQLLEN)StateLen (sqlstate), SQL_SQLSTATE_SIZE);
      ReportEqual (nok, L"cell past the SQLSTATE buffer", sqlstate[STATE_LEN], GUARD);
      ReportEqual (nok, L"message is not empty", StateLen (message) > 0, 1);

      wsprintf (msg, L"SQLSTATE = %s, native = %ld, TextLength = %d, message = %s",
		(const wchar_t *)sqlstate, (long)native_error, (int)text_length,
		(const wchar_t *)message);
      Logger::WriteMessage (msg);

      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
      retcode = SQLDisconnect (dbc);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);
      SQLFreeHandle (SQL_HANDLE_DBC, dbc);
      SQLFreeHandle (SQL_HANDLE_ENV, env);

      Assert::AreEqual (0, nok);
    }

    /*
     * SQLGetCursorNameW has to round-trip the cursor name and write its length
     * through NameLengthPtr.  Its Linux half used to pass the address of a
     * SQLSMALLINT to odbc_get_cursor_name(), which writes a SQLLEN there.
     * Windows never had that bug, so this is the counterpart that pins the
     * behaviour both platforms are supposed to share.
     */
    TEST_METHOD (GetCursorName_RoundTrip)
    {
      RETCODE retcode;

      SQLHENV env = SQL_NULL_HENV;
      SQLHDBC dbc = SQL_NULL_HDBC;
      SQLHSTMT stmt = SQL_NULL_HSTMT;
      SQLWCHAR cursor[64];
      SQLSMALLINT cursor_len = -1;
      int nok = 0;

      retcode = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);
      SQLSetEnvAttr (env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
      retcode = SQLAllocHandle (SQL_HANDLE_DBC, env, &dbc);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      retcode = SQLConnect (dbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      retcode = SQLAllocHandle (SQL_HANDLE_STMT, dbc, &stmt);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      retcode = SQLSetCursorName (stmt, L"CUR_GETCURSORNAMEW", SQL_NTS);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      retcode = SQLExecDirect (stmt, L"SELECT * FROM db_user", SQL_NTS);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      memset (cursor, 0, sizeof (cursor));
      retcode = SQLGetCursorName (stmt, cursor, sizeof (cursor) / sizeof (cursor[0]), &cursor_len);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);

      Logger::WriteMessage ((const wchar_t *)cursor);
      ReportEqual (nok, L"cursor name length", (SQLLEN)StateLen (cursor), 18);
      ReportEqual (nok, L"NameLength was written", cursor_len > 0, 1);
      ReportEqual (nok, L"cursor name", wcscmp ((const wchar_t *)cursor, L"CUR_GETCURSORNAMEW"), 0);

      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
      retcode = SQLDisconnect (dbc);
      Assert::AreNotEqual ((int)retcode, SQL_ERROR);
      SQLFreeHandle (SQL_HANDLE_DBC, dbc);
      SQLFreeHandle (SQL_HANDLE_ENV, env);

      Assert::AreEqual (0, nok);
    }
  };
}
