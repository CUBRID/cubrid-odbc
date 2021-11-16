/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#if defined (_WINDOWS)
#include		<windows.h>
#endif
#include		<stdio.h>

#include		"odbc_portable.h"
#include		"sqlext.h"
#include		"odbc_connection.h"
#include		"odbc_env.h"
#include		"odbc_statement.h"
#include		"odbc_result.h"
#include		"odbc_diag_record.h"
#include		"odbc_util.h"
#include		"odbc_catalog.h"
#include		"odbc_descriptor.h"
#include		"cas_cci.h"
#include		"odbc_resource.h"

/************************************************************************
* name: SQLDriverConnectW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLDriverConnectW (SQLHDBC hdbc, SQLHWND hwnd,
                   SQLWCHAR *in, SQLSMALLINT in_len,
                   SQLWCHAR *out, SQLSMALLINT out_max, SQLSMALLINT *out_len,
                   SQLUSMALLINT completion)
{
  RETCODE rc = ODBC_SUCCESS;
  SQLCHAR *pt_in, *pt_out;
  int encode_len = 0, temp_out_len = 0;
  ODBC_CONNECTION* conn = (ODBC_CONNECTION*)hdbc;

  OutputDebugString ("SQLDriverConnectW called\n");
  rc = wide_char_to_bytes (in, in_len, &pt_in, &encode_len, conn->charset);
  if (rc != ODBC_SUCCESS)
    {
      odbc_set_diag (conn->diag, "HY001", 0, "encode failed");
      return ODBC_ERROR;
    }
  pt_out = UT_ALLOC (out_max);
  if (pt_out == NULL && out_max > 0)
    {
      odbc_set_diag (conn->diag, "HY001", 0, "malloc failed");
      return ODBC_ERROR;
    }
  memset (pt_out, 0, out_max);
  
  rc = SQLDriverConnect (
         hdbc, hwnd,
         pt_in, in_len,
         pt_out, out_max, out_len,
         completion);
  if (out)
    {
      bytes_to_wide_char (pt_out, strlen (pt_out), &out, out_max, &temp_out_len, conn->charset);
    }
  if (out_len)
    {
      *out_len = temp_out_len;
    }
  UT_FREE (pt_in);
  UT_FREE (pt_out);
  return rc;
}

/************************************************************************
* name: SQLConnectW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLConnectW (SQLHDBC hdbc, SQLWCHAR *dsn, SQLSMALLINT dsn_len,
             SQLWCHAR *user, SQLSMALLINT user_len,
             SQLWCHAR *auth, SQLSMALLINT auth_len)
{
  SQLCHAR *cb_user, *cb_auth, *cb_dsn;
  int cb_user_len, cb_auth_len, cb_dsn_len;
  RETCODE ret;

  ODBC_CONNECTION *conn = (ODBC_CONNECTION *)hdbc;

  OutputDebugString ("SQLConnectW called\n");
  wide_char_to_bytes (user, user_len, &cb_user, &cb_user_len, conn->charset);
  wide_char_to_bytes (auth, auth_len, &cb_auth, &cb_auth_len, conn->charset);
  wide_char_to_bytes (dsn, dsn_len, &cb_dsn, &cb_dsn_len, conn->charset);

  ret = SQLConnect (hdbc,
                    cb_dsn,
                    cb_dsn_len,
                    cb_user,
                    cb_user_len,
                    cb_auth, cb_auth_len) ;

  UT_FREE (cb_user);
  UT_FREE (cb_auth);
  UT_FREE (cb_dsn);
  return ret;
}

/************************************************************************
* name: SQLExecDirectW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLExecDirectW (SQLHSTMT StatementHandle,
                SQLWCHAR * StatementText, SQLINTEGER TextLength)
{
  SQLCHAR *sql_text;
  RETCODE ret;
  int sql_len = 0;

  ODBC_STATEMENT *stmt = (ODBC_STATEMENT *)StatementHandle;

  wide_char_to_bytes (StatementText, TextLength, &sql_text,  &sql_len, stmt->conn->charset);
  OutputDebugString ("SQLExecDirectW called\n");
  ret = SQLExecDirect (StatementHandle, sql_text, sql_len);
  UT_FREE (sql_text);

  return ret;
}

/************************************************************************
* name: SQLGetInfoW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLGetInfoW (SQLHDBC ConnectionHandle,
             SQLUSMALLINT InfoType,
             SQLPOINTER InfoValue,
             SQLSMALLINT BufferLength, SQLSMALLINT * StringLength)
{
  RETCODE rc = SQL_SUCCESS;
  SQLLEN tmp_StringLength;
  SQLLEN info_value_size;
  SQLWCHAR *wvalue;
  char	*charset = ((ODBC_CONNECTION *)ConnectionHandle)->charset;

  OutputDebugString ("SQLGetInfoW called\n");

  DEBUG_TIMESTAMP (START_SQLGetInfoW);

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  if (InfoValue != NULL)
    {
      rc = check_if_even_number (InfoType, BufferLength);
      if (rc == ODBC_ERROR)
        {
          odbc_set_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, "HY090", 0, NULL);
          return ODBC_ERROR;
        }
    }

  rc =
    odbc_get_info ((ODBC_CONNECTION *) ConnectionHandle, InfoType, InfoValue,
                   BufferLength, &tmp_StringLength);

  if (StringLength != NULL)
    {
      info_value_size = tmp_StringLength * sizeof (SQLWCHAR);

      switch (InfoType)
      {
      case SQL_CURSOR_ROLLBACK_BEHAVIOR:
      case SQL_MAX_TABLE_NAME_LEN:
      case SQL_MAX_PROCEDURE_NAME_LEN:
      case SQL_CURSOR_COMMIT_BEHAVIOR:
      case SQL_TXN_ISOLATION_OPTION:
      case SQL_SCHEMA_USAGE:
      case SQL_TXN_CAPABLE:
      case SQL_MAX_SCHEMA_NAME_LEN:
            break;
      default:

        if (InfoValue != NULL)
          {
            *StringLength = (SQLSMALLINT)decide_info_value_length (InfoType, BufferLength, info_value_size);

            if (rc == SQL_SUCCESS)
              {
                if (*StringLength < info_value_size)
                  {
                    rc = SQL_SUCCESS_WITH_INFO;

                    odbc_set_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, "01004", 0, NULL);
                  }
              }

            bytes_to_wide_char (InfoValue, tmp_StringLength, &wvalue, 0, NULL, charset);

            if (wvalue != NULL)
              {
                (void)memcpy ((char *)InfoValue, (const char *)wvalue, *StringLength);
                ((SQLWCHAR *)InfoValue)[*StringLength / 2] = 0;
                UT_FREE_BSTR (wvalue);
              }
          }
        else
          {
            *StringLength = (SQLSMALLINT)info_value_size;
          }
        break;
      }
    }

  DEBUG_TIMESTAMP (END_SQLGetInfo);

  ODBC_RETURN (rc, ConnectionHandle);
}

/************************************************************************
* name: SQLGetDiagRecW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLGetDiagRecW (SQLSMALLINT HandleType,
                SQLHANDLE Handle,
                SQLSMALLINT RecNumber,
                SQLWCHAR * Sqlstate,
                SQLINTEGER * NativeError,
                SQLWCHAR * MessageText,
                SQLSMALLINT BufferLength, SQLSMALLINT * TextLength)
{
  SQLCHAR *sql_state, *message_text_buffer = NULL;
  int sql_state_len = 0;
  int out_length;
  SQLSMALLINT message_text_buffer_len = 0;
  RETCODE ret;
  ODBC_ENV *env;

  ODBC_CONNECTION *conn = (ODBC_CONNECTION *)Handle;

  env = (ODBC_ENV *) Handle;

  if (HandleType == SQL_HANDLE_STMT)
    {
      conn = env->conn;
    }

  OutputDebugString ("SQLGetDiagRecW called\n");
  wide_char_to_bytes (Sqlstate, sqlwcharlen (Sqlstate), &sql_state,  &sql_state_len, conn->charset);
  message_text_buffer = UT_ALLOC (BufferLength);
  if (message_text_buffer == NULL && BufferLength > 0)
    {
      odbc_set_diag (env->diag, "HY001", 0, "malloc failed");
      return ODBC_ERROR;
    }
  memset (message_text_buffer, 0 , BufferLength);

  ret = SQLGetDiagRec (HandleType, Handle, RecNumber, sql_state,
                       NativeError, message_text_buffer, BufferLength, &message_text_buffer_len);
  if (ret == ODBC_ERROR)
   {
     UT_FREE (message_text_buffer);
     return ret;
   }

  if (Sqlstate && sql_state)
  {
		int temp_buffer_length = MultiByteToWideChar (CP_ACP, 0, (LPCSTR) sql_state, strlen(sql_state), NULL, 0);

		MultiByteToWideChar (CP_ACP, 0, (LPCSTR) sql_state, strlen(sql_state), Sqlstate, temp_buffer_length);
  }
  
  bytes_to_wide_char (message_text_buffer, 
               message_text_buffer_len, 
               &MessageText, 
               BufferLength,
               &out_length,
               conn->charset);

  if (TextLength)
   {
     *TextLength = (SQLSMALLINT)(message_text_buffer_len * sizeof(wchar_t));
   }

  UT_FREE (message_text_buffer);
  return ret;
}

/************************************************************************
* name: SQLNativeSqlW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLNativeSqlW (SQLHDBC hdbc, SQLWCHAR *in, SQLINTEGER in_len,
               SQLWCHAR *out, SQLINTEGER out_max, SQLINTEGER *out_len)
{
  RETCODE ret = ODBC_ERROR;
  SQLCHAR *sql_state, *sql_text_buffer = NULL;
  int sql_state_len;  
  ODBC_CONNECTION * conn = (ODBC_CONNECTION *) hdbc;
  
  OutputDebugString ("SQLNativeSqlW called.\n");
  ret = wide_char_to_bytes (in, in_len, &sql_state,  &sql_state_len, conn->charset);
  sql_text_buffer = UT_ALLOC (out_max);
  if (sql_text_buffer == NULL && out_max > 0)
    {
      odbc_set_diag (conn->diag, "HY001", 0, "malloc failed");
      return ODBC_ERROR;
    }
   memset (sql_text_buffer, 0 , out_max);
      
  ret = SQLNativeSql(hdbc,  sql_state,  sql_state_len,  sql_text_buffer,  out_max, out_len);
  sql_state_len = *out_len;

  if (ret == ODBC_ERROR)
   {
     UT_FREE (sql_text_buffer);
     return ret;
   }
  bytes_to_wide_char (sql_text_buffer, sql_state_len, &out, out_max, out_len, conn->charset);
  UT_FREE (sql_text_buffer);
  return ret;
}

/************************************************************************
* name: SQLColumnsW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLColumnsW (SQLHSTMT hstmt,
             SQLWCHAR *catalog, SQLSMALLINT catalog_len,
             SQLWCHAR *schema, SQLSMALLINT schema_len,
             SQLWCHAR *table, SQLSMALLINT table_len,
             SQLWCHAR *column, SQLSMALLINT column_len)
{
  RETCODE ret = ODBC_ERROR;
  char *cb_catalog = NULL, *cb_schema = NULL, *cb_table = NULL, *cb_column = NULL;
  int    cb_catalog_len = 0, cb_schema_len = 0, cb_table_len = 0, cb_column_len = 0;
  ODBC_STATEMENT* stmt_handle = (ODBC_STATEMENT *) hstmt;
  OutputDebugString ("SQLColumnsW called.\n");

  wide_char_to_bytes (catalog, catalog_len, &cb_catalog,  &cb_catalog_len, stmt_handle->conn->charset);
  wide_char_to_bytes (schema, schema_len, &cb_schema,  &cb_schema_len, stmt_handle->conn->charset);
  wide_char_to_bytes (table, table_len, &cb_table,  &cb_table_len, stmt_handle->conn->charset);
  wide_char_to_bytes (column, column_len, &cb_column,  &cb_column_len, stmt_handle->conn->charset);

  ret = SQLColumns (hstmt,
                    cb_catalog, cb_catalog_len,
                    cb_schema, cb_schema_len,
                    cb_table, cb_table_len,
                    cb_column, cb_column_len);

  UT_FREE (cb_catalog);
  UT_FREE (cb_schema);
  UT_FREE (cb_table);
  UT_FREE (cb_column);
  return ret;
}

/************************************************************************
* name: SQLDescribeColW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLDescribeColW (SQLHSTMT hstmt, SQLUSMALLINT column,
                 SQLWCHAR *name, SQLSMALLINT name_max, SQLSMALLINT *name_len,
                 SQLSMALLINT *type, SQLULEN *size, SQLSMALLINT *scale,
                 SQLSMALLINT *nullable)
{
  RETCODE ret = ODBC_ERROR;
  SQLCHAR *name_buffer = NULL;
  SQLSMALLINT name_buffer_len=0;
  ODBC_STATEMENT* stmt_handle = (ODBC_STATEMENT *) hstmt;
  int out_length = -1;

  OutputDebugString ("SQLDescribeColW called.\n");
  
  name_buffer = UT_ALLOC (name_max);
  if (name_buffer == NULL && name_max > 0)
    {
      odbc_set_diag (stmt_handle->diag, "HY001", 0, "malloc failed");
      return ODBC_ERROR;
    }  
    memset (name_buffer, 0 , name_max);
    
  ret = SQLDescribeCol(hstmt, column, name_buffer, name_max, &name_buffer_len, type, size, scale, nullable);
  if (ret == ODBC_ERROR)
   {
     UT_FREE (name_buffer);
     return ret;
   }
   
  bytes_to_wide_char (name_buffer, name_buffer_len, &name, name_max, &out_length, stmt_handle->conn->charset);

  if (name_len)
   {
#if defined (_WINDOWS)
     *name_len = (SQLSMALLINT)out_length;
#else
     *name_len = (SQLSMALLINT)name_buffer_len;
#endif
   }

  UT_FREE (name_buffer);     
  return ret;
}

/************************************************************************
* name: SQLForeignKeysW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLForeignKeysW (SQLHSTMT hstmt,
                 SQLWCHAR *pk_catalog, SQLSMALLINT pk_catalog_len,
                 SQLWCHAR *pk_schema, SQLSMALLINT pk_schema_len,
                 SQLWCHAR *pk_table, SQLSMALLINT pk_table_len,
                 SQLWCHAR *fk_catalog, SQLSMALLINT fk_catalog_len,
                 SQLWCHAR *fk_schema, SQLSMALLINT fk_schema_len,
                 SQLWCHAR *fk_table, SQLSMALLINT fk_table_len)
{
  RETCODE ret = ODBC_ERROR;
  char *cb_pk_catalog = NULL,  *cb_pk_schema = NULL, *cb_pk_table = NULL,
          *cb_fk_catalog = NULL,   *cb_fk_schema = NULL,  *cb_fk_table = NULL;
  int  cb_pk_catalog_len = 0,  cb_pk_schema_len = 0, cb_pk_table_len = 0,
         cb_fk_catalog_len = 0,   cb_fk_schema_len = 0,  cb_fk_table_len = 0;

  ODBC_STATEMENT *stmt = (ODBC_STATEMENT *)hstmt;

  OutputDebugString ("SQLForeignKeysW called.\n"); 
  wide_char_to_bytes (pk_catalog, pk_catalog_len, &cb_pk_catalog, &cb_pk_catalog_len, stmt->conn->charset);
  wide_char_to_bytes (pk_schema, pk_schema_len, &cb_pk_schema, &cb_pk_schema_len, stmt->conn->charset);
  wide_char_to_bytes (pk_table, pk_table_len, &cb_pk_table, &cb_pk_table_len, stmt->conn->charset);
  wide_char_to_bytes (fk_catalog, fk_catalog_len, &cb_fk_catalog, &cb_fk_catalog_len, stmt->conn->charset);
  wide_char_to_bytes (fk_schema, fk_schema_len, &cb_fk_schema, &cb_fk_schema_len, stmt->conn->charset);
  wide_char_to_bytes (fk_table, fk_table_len, &cb_fk_table, &cb_fk_table_len, stmt->conn->charset);
        
  ret = SQLForeignKeys(hstmt, 
                                       cb_pk_catalog, cb_pk_catalog_len, cb_pk_schema, cb_pk_schema_len,
                                       cb_pk_table, cb_pk_table_len, cb_fk_catalog, cb_fk_catalog_len,
                                       cb_fk_schema, cb_fk_schema_len, cb_fk_table, cb_fk_table_len);
  
  UT_FREE(cb_pk_catalog);
  UT_FREE(cb_pk_schema);
  UT_FREE(cb_pk_table);
  UT_FREE(cb_fk_catalog);
  UT_FREE(cb_fk_schema);
  UT_FREE(cb_fk_table);
  return ret;
}

/************************************************************************
* name: SQLGetConnectOptionW
* arguments:
* returns/side-effects:
* description:
* NOTE: ODBC 2.0 API, not need to implement
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLGetConnectOptionW (SQLHDBC hdbc, SQLUSMALLINT option, SQLPOINTER param)
{
  RETCODE ret = ODBC_ERROR;
  OutputDebugString ("SQLGetConnectOptionW called, not implement.\n");
  return ret;
}

/************************************************************************
* name: SQLGetCursorNameW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLGetCursorNameW (SQLHSTMT hstmt, SQLWCHAR *cursor, SQLSMALLINT cursor_max,
                   SQLSMALLINT *cursor_len)
{
  RETCODE ret = ODBC_ERROR;
  char* cursor_name = NULL;
  SQLSMALLINT  cursor_name_len = 0;
  ODBC_STATEMENT* stmt_handle = (ODBC_STATEMENT *) hstmt;
  int out_length;
  
  OutputDebugString ("SQLGetCursorNameW called.\n");
  cursor_name = UT_ALLOC (cursor_max);
  if (cursor_name == NULL && cursor_max > 0)
    {
      odbc_set_diag (stmt_handle->diag, "HY001", 0, "malloc failed");
      return ODBC_ERROR;
    }
  memset (cursor_name, 0 , cursor_max);
  ret = SQLGetCursorName(hstmt, cursor_name, cursor_max, &cursor_name_len);  
  if (ret == ODBC_ERROR)
   {
     UT_FREE (cursor_name);
     return ret;
   }
                     
  bytes_to_wide_char (cursor_name, cursor_name_len, &cursor, cursor_max, &out_length, stmt_handle->conn->charset);

  if (cursor_len)
   {
     *cursor_len = (SQLSMALLINT)out_length;
   }

  UT_FREE(cursor_name);   
  return ret;
}

/************************************************************************
* name: SQLPrepareW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLPrepareW (SQLHSTMT hstmt, SQLWCHAR *str, SQLINTEGER str_len)
{
  RETCODE ret = ODBC_ERROR;
  char* sql_state = NULL;
  int sql_state_len = 0;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLPrepareW called.\n");
  
  stmt_handle = (ODBC_STATEMENT *)hstmt;

  wide_char_to_bytes (str, str_len, &sql_state,  &sql_state_len, stmt_handle->conn->charset);
  
  ret = SQLPrepare(hstmt, sql_state, sql_state_len);
  UT_FREE(sql_state);
  return ret;
}

/************************************************************************
* name: SQLPrimaryKeysW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLPrimaryKeysW (SQLHSTMT hstmt,
                 SQLWCHAR *catalog, SQLSMALLINT catalog_len,
                 SQLWCHAR *schema, SQLSMALLINT schema_len,
                 SQLWCHAR *table, SQLSMALLINT table_len)
{
  RETCODE ret = ODBC_ERROR;
  SQLCHAR *cb_catalog = NULL,  *cb_schema = NULL, *cb_table = NULL;
  int cb_catalog_len = 0, cb_schema_len = 0, cb_table_len = 0;
  
  ODBC_STATEMENT *stmt = (ODBC_STATEMENT *)hstmt;

  OutputDebugString ("SQLPrimaryKeysW called.\n");
  wide_char_to_bytes (catalog, catalog_len, &cb_catalog, &cb_catalog_len, stmt->conn->charset);
  wide_char_to_bytes (schema, schema_len, &cb_schema, &cb_schema_len, stmt->conn->charset);
  wide_char_to_bytes (table, table_len, &cb_table, &cb_table_len, stmt->conn->charset);
  
  ret = SQLPrimaryKeys(hstmt, cb_catalog, cb_catalog_len, cb_schema, cb_schema_len, cb_table, cb_table_len);
  
  UT_FREE(cb_catalog);
  UT_FREE(cb_schema);
  UT_FREE(cb_table);
  return ret;
}

/************************************************************************
* name: SQLSetCursorNameW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLSetCursorNameW (SQLHSTMT hstmt, SQLWCHAR *name, SQLSMALLINT name_len)
{
  RETCODE ret = ODBC_ERROR;
  SQLCHAR *cb_name = NULL;
  int cb_name_len = 0;
  
  OutputDebugString ("SQLSetCursorNameW called.\n");
  wide_char_to_bytes (name, name_len, &cb_name, &cb_name_len, NULL);
  ret = SQLSetCursorName(hstmt, cb_name, cb_name_len);
  UT_FREE(cb_name);
  
  return ret;
}

/************************************************************************
* name: SQLSpecialColumnsW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLSpecialColumnsW (SQLHSTMT hstmt, SQLUSMALLINT type,
                    SQLWCHAR *catalog, SQLSMALLINT catalog_len,
                    SQLWCHAR *schema, SQLSMALLINT schema_len,
                    SQLWCHAR *table, SQLSMALLINT table_len,
                    SQLUSMALLINT scope, SQLUSMALLINT nullable)
{
  RETCODE ret = ODBC_ERROR;
  SQLCHAR *cb_catalog = NULL,  *cb_schema = NULL, *cb_table = NULL;
  int cb_catalog_len = 0, cb_schema_len = 0, cb_table_len = 0;
  
  ODBC_STATEMENT *stmt = (ODBC_STATEMENT *)hstmt;

  OutputDebugString ("SQLSpecialColumnsW called.\n");
  wide_char_to_bytes (catalog, catalog_len, &cb_catalog, &cb_catalog_len, stmt->conn->charset);
  wide_char_to_bytes (schema, schema_len, &cb_schema, &cb_schema_len, stmt->conn->charset);
  wide_char_to_bytes (table, table_len, &cb_table, &cb_table_len, stmt->conn->charset);
  
  ret = SQLSpecialColumns(hstmt, type, 
                                                 cb_catalog, cb_catalog_len, 
                                                 cb_schema, cb_schema_len, 
                                                 cb_table, cb_table_len,
                                                 scope, nullable);
  
  UT_FREE(cb_catalog);
  UT_FREE(cb_schema);
  UT_FREE(cb_table);
  return ret;
}

/************************************************************************
* name: SQLStatisticsW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLStatisticsW (SQLHSTMT hstmt,
                SQLWCHAR *catalog, SQLSMALLINT catalog_len,
                SQLWCHAR *schema, SQLSMALLINT schema_len,
                SQLWCHAR *table, SQLSMALLINT table_len,
                SQLUSMALLINT unique, SQLUSMALLINT accuracy)
{
  RETCODE ret = ODBC_ERROR;
  SQLCHAR *cb_catalog = NULL,  *cb_schema = NULL, *cb_table = NULL;
  int cb_catalog_len = 0, cb_schema_len = 0, cb_table_len = 0;
  
  ODBC_STATEMENT *stmt = (ODBC_STATEMENT *)hstmt;

  OutputDebugString ("SQLStatisticsW called.\n");
  wide_char_to_bytes (catalog, catalog_len, &cb_catalog, &cb_catalog_len, stmt->conn->charset);
  wide_char_to_bytes (schema, schema_len, &cb_schema, &cb_schema_len, stmt->conn->charset);
  wide_char_to_bytes (table, table_len, &cb_table, &cb_table_len, stmt->conn->charset);
  
  ret = SQLStatistics(hstmt, 
                                 cb_catalog, cb_catalog_len, 
                                 cb_schema, cb_schema_len, 
                                 cb_table, cb_table_len,
                                 unique, accuracy);
  
  UT_FREE(cb_catalog);
  UT_FREE(cb_schema);
  UT_FREE(cb_table);
  return ret;
}

/************************************************************************
* name: SQLTablePrivilegesW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLTablePrivilegesW (SQLHSTMT hstmt,
                     SQLWCHAR *catalog, SQLSMALLINT catalog_len,
                     SQLWCHAR *schema, SQLSMALLINT schema_len,
                     SQLWCHAR *table, SQLSMALLINT table_len)
{
  RETCODE ret = ODBC_ERROR;
  SQLCHAR *cb_catalog = NULL,  *cb_schema = NULL, *cb_table = NULL;
  int cb_catalog_len = 0, cb_schema_len = 0, cb_table_len = 0;
  
  ODBC_STATEMENT *stmt = (ODBC_STATEMENT *)hstmt;

  OutputDebugString ("SQLTablePrivilegesW called.\n");
  wide_char_to_bytes (catalog, catalog_len, &cb_catalog, &cb_catalog_len, stmt->conn->charset);
  wide_char_to_bytes (schema, schema_len, &cb_schema, &cb_schema_len, stmt->conn->charset);
  wide_char_to_bytes (table, table_len, &cb_table, &cb_table_len, stmt->conn->charset);
  
  ret = SQLTablePrivileges(hstmt, 
                                           cb_catalog, cb_catalog_len, 
                                           cb_schema, cb_schema_len, 
                                           cb_table, cb_table_len);
  
  UT_FREE(cb_catalog);
  UT_FREE(cb_schema);
  UT_FREE(cb_table);
  return ret;
}

/************************************************************************
* name: SQLTablesW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLTablesW (SQLHSTMT hstmt,
            SQLWCHAR *catalog, SQLSMALLINT catalog_len,
            SQLWCHAR *schema, SQLSMALLINT schema_len,
            SQLWCHAR *table, SQLSMALLINT table_len,
            SQLWCHAR *type, SQLSMALLINT type_len)
{
  RETCODE ret = ODBC_ERROR;
  SQLCHAR *cb_catalog = NULL,  *cb_schema = NULL, *cb_table = NULL, *cb_type=NULL;
  int cb_catalog_len = 0, cb_schema_len = 0, cb_table_len = 0, cb_type_len = 0;
  
  ODBC_STATEMENT *stmt = (ODBC_STATEMENT *)hstmt;

  OutputDebugString ("SQLTablesW called.\n");
  wide_char_to_bytes (catalog, catalog_len, &cb_catalog, &cb_catalog_len, stmt->conn->charset);
  wide_char_to_bytes (schema, schema_len, &cb_schema, &cb_schema_len, stmt->conn->charset);
  wide_char_to_bytes (table, table_len, &cb_table, &cb_table_len, stmt->conn->charset);
  wide_char_to_bytes (type, type_len, &cb_type, &cb_type_len, stmt->conn->charset);
  
  ret = SQLTables(hstmt, 
                             cb_catalog, cb_catalog_len, 
                             cb_schema, cb_schema_len, 
                             cb_table, cb_table_len,
                             cb_type, cb_type_len);
  
  UT_FREE(cb_catalog);
  UT_FREE(cb_schema);
  UT_FREE(cb_table);
  UT_FREE(cb_type);
  return ret;
}

/************************************************************************
* name: SQLGetDescFieldW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLGetDescFieldW (SQLHDESC hdesc, SQLSMALLINT record, SQLSMALLINT field,
                  SQLPOINTER value, SQLINTEGER value_max, SQLINTEGER *value_len)
{
  RETCODE ret = ODBC_ERROR;
  char* cb_value = NULL;
  int cb_value_len = 0;
  ODBC_DESC *DescriptorHandle = (ODBC_DESC*)hdesc;
  OutputDebugString ("SQLGetDescFieldW called.\n");
  
  cb_value = UT_ALLOC (value_max);
  if (cb_value == NULL && value_max > 0)
    {
      odbc_set_diag (DescriptorHandle->diag, "HY001", 0, "malloc failed");
      return ODBC_ERROR;
    }
  memset (cb_value, 0 , value_max);
    
  SQLGetDescField(hdesc, record, field, cb_value, value_max, &cb_value_len);
  if (ret == ODBC_ERROR)
   {
     UT_FREE (cb_value);
     return ret;
   }  
   
  bytes_to_wide_char (cb_value, cb_value_len, (wchar_t**)&value, value_max, (int*)value_len, DescriptorHandle->conn->charset);
  UT_FREE (cb_value);
  return ret;
}

/************************************************************************
* name: SQLGetDescRecW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLGetDescRecW (SQLHDESC hdesc, SQLSMALLINT record, SQLWCHAR *name,
                SQLSMALLINT name_max, SQLSMALLINT *name_len, SQLSMALLINT *type,
                SQLSMALLINT *subtype, SQLLEN *length, SQLSMALLINT *precision,
                SQLSMALLINT *scale, SQLSMALLINT *nullable)
{
  RETCODE ret = ODBC_ERROR;
  SQLCHAR *name_buffer = NULL;
  SQLSMALLINT name_buffer_len=0;
  ODBC_DESC * desc_handle = hdesc;
  int out_length;

  OutputDebugString ("SQLDescribeColW called.\n");
  
  name_buffer = UT_ALLOC (name_max);
  if (name_buffer == NULL && name_max > 0)
    {
      odbc_set_diag (desc_handle->diag, "HY001", 0, "malloc failed");
      return ODBC_ERROR;
    }  
    memset (name_buffer, 0 , name_max);
    
  ret = SQLGetDescRec(hdesc, 
                                       record, name_buffer, name_max, 
                                       &name_buffer_len, type, subtype, 
                                       length, precision, scale, nullable);
  if (ret == ODBC_ERROR)
   {
     UT_FREE (name_buffer);
     return ret;
   }
   
  bytes_to_wide_char (name_buffer, name_buffer_len, &name, name_max, &out_length, desc_handle->conn->charset);

  if (name_len)
   {
     *name_len = (SQLSMALLINT)out_length;
   }

  UT_FREE (name_buffer);     
  return ret;
}

/************************************************************************
* name: SQLSetDescFieldW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLSetDescFieldW (SQLHDESC hdesc, SQLSMALLINT record, SQLSMALLINT field,
                  SQLPOINTER value, SQLINTEGER value_len)
{
  //RETCODE ret = ODBC_ERROR;
  OutputDebugString ("SQLSetDescFieldW called.\n");
  return SQLSetDescField(hdesc, record, field, value, value_len);
}

/************************************************************************
* name: SQLSetDescRecW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLSetDescRecW (SQLHDESC hdesc, SQLSMALLINT record, SQLSMALLINT type,
                SQLSMALLINT subtype, SQLLEN length, SQLSMALLINT precision,
                SQLSMALLINT scale, SQLPOINTER data_ptr,
                SQLLEN *octet_length_ptr, SQLLEN *indicator_ptr)
{
  RETCODE ret = ODBC_ERROR;
  OutputDebugString ("SQLSetDescRecW called\n");
  return SQLSetDescRec(hdesc, 
                                         record, type, subtype, length, 
                                         precision, scale, data_ptr, 
                                         octet_length_ptr, indicator_ptr);
}

/************************************************************************
* name: SQLBrowseConnectW
* arguments:
* returns/side-effects:
* description:
* NOTE: ODBC 2.0 API, not need to implement
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLBrowseConnectW (SQLHDBC hdbc, SQLWCHAR *in, SQLSMALLINT in_len,
                   SQLWCHAR *out, SQLSMALLINT out_max, SQLSMALLINT *out_len)
{
  OutputDebugString ("SQLBrowseConnectW called\n");
  return ODBC_ERROR;
}

/************************************************************************
* name: SQLGetStmtAttrW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLGetStmtAttrW (SQLHSTMT StatementHandle,
                 SQLINTEGER Attribute,
                 SQLPOINTER Value,
                 SQLINTEGER BufferLength, SQLINTEGER * StringLength)
{
  OutputDebugString ("SQLGetStmtAttrW called\n");
  return SQLGetStmtAttr (StatementHandle, Attribute, Value, BufferLength, StringLength);
}

/************************************************************************
* name: SQLSetConnectAttrW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLSetConnectAttrW (SQLHDBC hdbc, SQLINTEGER attribute,
                    SQLPOINTER value, SQLINTEGER value_len)
{
  OutputDebugString ("SQLSetConnectAttrW called.\n");
  return SQLSetConnectAttr (hdbc, attribute, value,  value_len);
}

/************************************************************************
* name: SQLSetStmtAttrW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLSetStmtAttrW (SQLHSTMT hstmt, SQLINTEGER attribute,
                 SQLPOINTER value, SQLINTEGER value_len)
{
  OutputDebugString ("SQLSetStmtAttrW called.\n");
  return SQLSetStmtAttr (hstmt, attribute, value, value_len);
}

/************************************************************************
* name: SQLSetConnectOptionW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLSetConnectOptionW (SQLHDBC hdbc, SQLUSMALLINT option, SQLULEN param)
{
  OutputDebugString ("SQLSetConnectOptionW called.\n");
  return SQLSetConnectOption (hdbc, option, param);
}

/************************************************************************
* name: SQLGetConnectAttrW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLGetConnectAttrW (SQLHDBC hdbc, SQLINTEGER attribute, SQLPOINTER value,
                    SQLINTEGER value_max, SQLINTEGER *value_len)
{
  RETCODE ret = ODBC_ERROR;
  OutputDebugString ("SQLGetConnectAttrW called.\n");

  ret = SQLGetConnectAttr (hdbc, attribute, value, value_max, value_len);
  return ret;
}

/************************************************************************
* name: SQLGetDiagFieldW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLGetDiagFieldW (SQLSMALLINT handle_type, SQLHANDLE handle,
                  SQLSMALLINT record, SQLSMALLINT field,
                  SQLPOINTER info, SQLSMALLINT info_max,
                  SQLSMALLINT *info_len)
{
  RETCODE ret = ODBC_ERROR;
  OutputDebugString ("SQLGetDiagFieldW called.\n");

  ret = SQLGetDiagField (handle_type, handle, record, field, info, info_max, info_len);
  return ret;
}

/************************************************************************
* name: SQLColAttributeW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLColAttributeW (SQLHSTMT StatementHandle,
                  SQLUSMALLINT ColumnNumber,
                  SQLUSMALLINT FieldIdentifier,
                  SQLPOINTER CharacterAttribute,
                  SQLSMALLINT BufferLength, SQLSMALLINT * StringLength,
#if defined (_WIN64) || defined (__linux__) || defined (AIX)
                  SQLLEN * NumericAttribute)
#else
                  SQLPOINTER NumericAttribute)
#endif
{
  RETCODE ret = ODBC_ERROR;
  SQLWCHAR *wvalue;
  ODBC_STATEMENT *stmt = (ODBC_STATEMENT *)StatementHandle;
  OutputDebugString ("SQLColAttributeW called.\n");

  ret = SQLColAttribute (StatementHandle, ColumnNumber,
                         FieldIdentifier, CharacterAttribute, BufferLength,
                         StringLength, NumericAttribute);
  if (CharacterAttribute)
    {
      BufferLength /= sizeof (SQLWCHAR);
      if (StringLength != NULL)
        {
          *StringLength = (SQLSMALLINT) (*StringLength) * sizeof (SQLWCHAR);
        }
      if (CharacterAttribute && StringLength)
        {
          bytes_to_wide_char (CharacterAttribute, *StringLength / 2, &wvalue, 0, NULL, stmt->conn->charset);
          if(wvalue != NULL)
           {
             (void)memcpy ((char *)CharacterAttribute, (const char *)wvalue, *StringLength);
             ((SQLWCHAR *)CharacterAttribute)[*StringLength / 2] = 0;
             UT_FREE_BSTR (wvalue);
           }
        }
    }
  return ret;
}
#endif

/************************************************************************
* name: SQLGetTypeInfoW
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
ODBC_INTERFACE RETCODE SQL_API
SQLGetTypeInfoW (SQLHSTMT StatementHandle, SQLSMALLINT DataType)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLGetTypeInfo called\n");

  DEBUG_TIMESTAMP (START_SQLGetTypeInfo);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_get_type_info (stmt_handle, DataType);

  DEBUG_TIMESTAMP (END_SQLGetTypeInfo);

  ODBC_RETURN (rc, StatementHandle);
}
