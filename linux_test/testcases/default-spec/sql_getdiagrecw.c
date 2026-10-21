#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include "test_util.h"

/*
 * SQLGetDiagRecW must not write past the SQLSTATE buffer.  The buffer an
 * application hands over is SQL_SQLSTATE_SIZE + 1 characters and nothing
 * more, so this testcase puts a guard cell right behind it and checks that
 * the cell survives the call.
 *
 * odbc_get_diag_rec() used to strcpy() the record's sql_state into it with no
 * bound at all, and without checking it for NULL.  The Linux half of
 * SQLGetDiagRecW (src/unicode.c) also handed a SQLSMALLINT * to the parameter
 * through which odbc_get_diag_rec() writes a SQLLEN, overrunning the caller's
 * variable by four bytes, and passed its own SQLWCHAR output buffer where the
 * SQLCHAR message buffer belonged.
 *
 * The message itself is only printed, not checked: both SQLGetDiagRec and
 * SQLGetDiagRecW call odbc_free_diag (diag, INIT) when a record number is out
 * of range, so the driver manager's probe for record 2 throws record 1 away
 * before the application ever asks for it.  Once that is fixed the printf
 * below shows the message and the two checks can be turned into asserts.
 */

#define STATE_LEN	(SQL_SQLSTATE_SIZE + 1)
#define GUARD		0x5a5a

int
sql_getdiagrecw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  wchar_t *dsn_buf = NULL;
  wchar_t *query_buf = NULL;
  /* exactly the buffer the spec asks for, plus a cell nothing may touch */
  SQLWCHAR sqlstate[STATE_LEN + 1];
  SQLWCHAR message[1024];
  SQLINTEGER native_error = 0;
  SQLSMALLINT text_length = -1;
  char *mesg = NULL;
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

  /* a table that cannot exist, so the engine is the one reporting the error */
  bytes_to_wide_char ("SELECT * FROM no_such_table_for_getdiagrecw", SQL_NTS, &query_buf, 0, NULL, "UCS2");
  retcode = SQLExecDirectW (stmt, (SQLWCHAR *) query_buf, SQL_NTS);
  ReportEqual (nok, "SQLExecDirectW on a missing table fails", retcode, SQL_ERROR);

  memset (sqlstate, 0, sizeof (sqlstate));
  memset (message, 0, sizeof (message));
  sqlstate[STATE_LEN] = GUARD;

  retcode = SQLGetDiagRecW (SQL_HANDLE_STMT, stmt, 1, sqlstate, &native_error,
			    message, sizeof (message) / sizeof (message[0]), &text_length);
  AreNotEqual (retcode, SQL_ERROR);

  ReportEqual (nok, "SQLSTATE length", sqlwcharlen ((wchar_t *) sqlstate), SQL_SQLSTATE_SIZE);
  ReportEqual (nok, "cell past the SQLSTATE buffer", sqlstate[STATE_LEN], GUARD);

  wide_char_to_bytes ((wchar_t *) message, sqlwcharlen ((wchar_t *) message), (char **) &mesg, NULL, "UCS2");
  printf ("\tret = %d, native = %d, TextLength = %d, message = |%s|\n",
	  (int) retcode, (int) native_error, (int) text_length, mesg ? mesg : "");
  UT_FREE (mesg);

  SQLFreeHandle (SQL_HANDLE_STMT, stmt);
  retcode = SQLDisconnect (dbc);
  AreNotEqual (retcode, SQL_ERROR);
  SQLFreeHandle (SQL_HANDLE_DBC, dbc);
  SQLFreeHandle (SQL_HANDLE_ENV, env);

  UT_FREE (dsn_buf);
  UT_FREE (query_buf);

  return nok == 0 ? SQL_SUCCESS : 1;
}
