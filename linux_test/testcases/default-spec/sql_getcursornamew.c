#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include "test_util.h"

/*
 * SQLSetCursorNameW / SQLGetCursorNameW have to round-trip the cursor name
 * and report its length in characters.
 *
 * SQLGetCursorNameW (src/unicode.c) used to pass the address of its
 * SQLSMALLINT length variable to odbc_get_cursor_name(), which writes a
 * SQLLEN there - four bytes past the variable on a 64 bit build, over
 * whatever the compiler had put next to it in the stack frame.  The SQLLEN
 * the function already declared for this (tmp_NameLength) was left unused.
 */

#define CURSOR_NAME	"CUR_GETCURSORNAMEW"
#define GUARD		0x5a5a

int
sql_getcursornamew (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  wchar_t *dsn_buf = NULL;
  wchar_t *name_buf = NULL;
  wchar_t *query_buf = NULL;
  SQLWCHAR cursor[64];
  SQLSMALLINT cursor_len = -1;
  char *name = NULL;
  int nok = 0;

  retcode = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
  retcode = SQLSetEnvAttr (env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, 0);
  retcode = SQLAllocHandle (SQL_HANDLE_DBC, env, &dbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (dbc, (SQLWCHAR *) dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLAllocHandle (SQL_HANDLE_STMT, dbc, &stmt);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (CURSOR_NAME, SQL_NTS, &name_buf, 0, NULL, "UCS2");
  retcode = SQLSetCursorNameW (stmt, (SQLWCHAR *) name_buf, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char ("SELECT * FROM db_user", SQL_NTS, &query_buf, 0, NULL, "UCS2");
  retcode = SQLExecDirectW (stmt, (SQLWCHAR *) query_buf, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  memset (cursor, 0, sizeof (cursor));
  retcode = SQLGetCursorNameW (stmt, cursor, sizeof (cursor) / sizeof (cursor[0]), &cursor_len);
  AreNotEqual (retcode, SQL_ERROR);

  wide_char_to_bytes ((wchar_t *) cursor, sqlwcharlen ((wchar_t *) cursor), (char **) &name, NULL, "UCS2");
  printf ("\tcursor name = |%.*s|, length = %d\n",
	  (int) (name ? strlen (name) : 0), name ? name : "", (int) cursor_len);

  ReportEqual (nok, "cursor name length", sqlwcharlen ((wchar_t *) cursor), strlen (CURSOR_NAME));
  /* the W driver reports the length in bytes and the ANSI one in characters,
     so only check that something sane was written through the pointer */
  ReportEqual (nok, "NameLength was written", cursor_len > 0, 1);
  if (name != NULL)
    {
      ReportEqualStr (nok, "cursor name", name, CURSOR_NAME);
    }
  else
    {
      ReportEqual (nok, "cursor name was returned", 0, 1);
    }

  UT_FREE (name);
  SQLFreeHandle (SQL_HANDLE_STMT, stmt);
  retcode = SQLDisconnect (dbc);
  AreNotEqual (retcode, SQL_ERROR);
  SQLFreeHandle (SQL_HANDLE_DBC, dbc);
  SQLFreeHandle (SQL_HANDLE_ENV, env);

  UT_FREE (dsn_buf);
  UT_FREE (name_buf);
  UT_FREE (query_buf);

  return nok == 0 ? SQL_SUCCESS : 1;
}
