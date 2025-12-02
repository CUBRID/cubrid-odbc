/*
 *
 * Copyright 2016 CUBRID Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include  <stdio.h>
#include  <fcntl.h>
#include  <stdlib.h>
#include  <iconv.h>
#include  <locale.h>
#include  <wchar.h>
#include  <errno.h>
#include  <stdarg.h>
#include  <string.h>
#include  "odbc_portable.h"
#include  "odbcinst.h"
#include  "odbc_resource.h"
#include  "odbc_connection.h"
#include  "odbc_linux.h"

/*
 * supplementary Functions for CUBRID ODBC Unicode for Linux
 * these functions will be used Linux & Unicode ONLY
 */

#define FETCH_SIZE_DEFAULT	1
#define AUTOCOMMIT_DEFAULT	"false"
#define OMIT_SCHEMA_DEFAULT	"off"

ODBC_INTERFACE RETCODE SQL_API
SQLDriverConnectLinux (HDBC hdbc,
		       HWND hWnd,
		       UCHAR *szConnStrIn,
		       SWORD cbConnStrIn,
		       UCHAR *szConnStrOut, SWORD cbConnStrOut, SQLSMALLINT *pcbConnStrOut, UWORD uwMode)
{
  HINI    hIni;
  RETCODE rc = ODBC_SUCCESS;
  const char *ptDSN;
  int port, fetch_size;
  CUBRIDDSNItem dsn;
  char connstr_buf[1024] = "";
  char charset[ITEMBUFLEN] = CODE_NAME_UNICODE;
  char autocommit[ITEMBUFLEN] = AUTOCOMMIT_DEFAULT;
  char omit_schema[ITEMBUFLEN] = OMIT_SCHEMA_DEFAULT;
  char *pt;
  char ConnStrIn[BUF_SIZE * 4];
  size_t connstr_len;
  ODBCINI_DSN_LOOKUP_RESULT retcode;

  connstr_len = cbConnStrIn > 0 ? cbConnStrIn : strlen (szConnStrIn);
  if (szConnStrIn != NULL)
    {
      strncpy (ConnStrIn, szConnStrIn, connstr_len);
    }

  for (pt = ConnStrIn; *pt != '\0'; ++pt)
    {
      if (*pt == ';')		// connection string delimiter
	{
	  *pt = '\0';
	}
    }

  memset (&dsn, 0, sizeof (dsn));
  ptDSN = find_key (ConnStrIn, KEYWORD_DSN);

  if (ptDSN)
    {
      snprintf (dsn.dsn, ITEMBUFLEN, "%s", ptDSN);
    }

  memset (&hIni, 0, sizeof (hIni));
  if (ptDSN)
    {
      retcode = ini_fileopen (ptDSN, &hIni);
    }

  DSN_LOOKUP (ConnStrIn, ptDSN, hIni, KEYWORD_DBNAME, dsn.db_name);
  DSN_LOOKUP (ConnStrIn, ptDSN, hIni, KEYWORD_USER, dsn.user);
  DSN_LOOKUP (ConnStrIn, ptDSN, hIni, KEYWORD_PASSWORD, dsn.password);
  DSN_LOOKUP (ConnStrIn, ptDSN, hIni, KEYWORD_SERVER, dsn.server);
  DSN_LOOKUP (ConnStrIn, ptDSN, hIni, KEYWORD_PORT, dsn.port);
  port = atoi (dsn.port);
  DSN_LOOKUP (ConnStrIn, ptDSN, hIni, KEYWORD_CHARSET, dsn.charset);
  snprintf (charset, sizeof (charset), "%s", strlen (dsn.charset) ? dsn.charset : CODE_NAME_UNICODE);
  DSN_LOOKUP (ConnStrIn, ptDSN, hIni, KEYWORD_AUTOCOMMIT, dsn.autocommit);
  snprintf (autocommit, sizeof (autocommit), "%s", strlen (dsn.autocommit) ? dsn.autocommit : AUTOCOMMIT_DEFAULT);
  DSN_LOOKUP (ConnStrIn, ptDSN, hIni, KEYWORD_OMIT_SCHEMA, dsn.omit_schema);
  snprintf (omit_schema, sizeof (omit_schema), "%s", strlen (dsn.omit_schema) ? dsn.omit_schema : OMIT_SCHEMA_DEFAULT);
  DSN_LOOKUP (ConnStrIn, ptDSN, hIni, KEYWORD_FETCH_SIZE, dsn.fetch_size);
  fetch_size = strlen (dsn.fetch_size) ? atoi (dsn.fetch_size) : FETCH_SIZE_DEFAULT;

  if (retcode != ODBCINI_DSN_NOT_FOUND)
    {
      iniClose (hIni);
    }

  dsn2connstr (&dsn, connstr_buf);

  rc = odbc_connect_new (hdbc, dsn.dsn, dsn.db_name, dsn.user, dsn.password, dsn.server, port,
			 fetch_size, charset, autocommit, omit_schema, ConnStrIn);

  UT_FREE (ptDSN);

  if ((szConnStrOut) && cbConnStrOut > 0)
    {
      snprintf (szConnStrOut, MIN (strlen (connstr_buf), (unsigned) cbConnStrOut), "%s", connstr_buf);
    }

  if (pcbConnStrOut)
    {
      *pcbConnStrOut = MIN (strlen (connstr_buf), (unsigned) cbConnStrOut);
    }

  return rc;
}

ODBC_INTERFACE RETCODE SQL_API
SQLConnectLinux (SQLHDBC ConnectionHandle,
		 SQLCHAR *DataSource,
		 SQLSMALLINT NameLength1,
		 SQLCHAR *UserName, SQLSMALLINT NameLength2, SQLCHAR *Authentication, SQLSMALLINT NameLength3)
{
  RETCODE rc = SQL_SUCCESS;
  SQLCHAR *stDataSource = NULL;
  SQLCHAR *stUserName = NULL;
  SQLCHAR *stAuthentication = NULL;
  SQLCHAR stDBName[ITEMBUFLEN];
  SQLCHAR stServerName[ITEMBUFLEN];
  SQLINTEGER Port, FetchSize;
  SQLCHAR stCharSet[ITEMBUFLEN];
  SQLCHAR stAutocommit[ITEMBUFLEN];
  SQLCHAR stOmitSchema[ITEMBUFLEN];
  SQLCHAR *user = NULL, *pass = NULL;

  if (UserName == NULL || NameLength2 <= 0)
    {
      NameLength2 = SQL_MAX_USER_NAME_LEN + 1;
      NameLength3 = SQL_MAX_OPTION_STRING_LENGTH;
      stUserName = user = UT_ALLOC (NameLength2);
      stAuthentication = pass = UT_ALLOC (NameLength3);
    }
  else
    {
      stUserName = UT_MAKE_STRING (UserName, NameLength2);
      stAuthentication = UT_MAKE_STRING (Authentication, NameLength3);
    }

  get_dsn_info (DataSource, stDBName, sizeof (stDBName), user, NameLength2, pass, NameLength3,
		stServerName, sizeof (stServerName), &Port, &FetchSize,
		stCharSet, sizeof (stCharSet), stAutocommit, sizeof (stAutocommit),
		stOmitSchema, sizeof (stOmitSchema));

  rc = odbc_connect_new ((ODBC_CONNECTION *) ConnectionHandle, stDataSource,
			 stDBName, stUserName, stAuthentication, stServerName,
			 Port, FetchSize, stCharSet, stAutocommit, stOmitSchema, NULL);

  UT_FREE (stUserName);
  UT_FREE (stAuthentication);

  return rc;
}

ODBC_INTERFACE RETCODE SQL_API
SQLExecDirectLinux (SQLHSTMT StatementHandle, SQLCHAR *StatementText, SQLINTEGER TextLength)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  if (strcasecmp (StatementText, "@QP@") == 0)
    {
      stmt_handle->query_plan = CCI_EXEC_ONLY_QUERY_PLAN;
      return ODBC_SUCCESS;
    }

  if (strcasecmp (StatementText, "@QE@") == 0)
    {
      stmt_handle->query_plan = CCI_EXEC_ONLY_QUERY_PLAN | CCI_EXEC_QUERY_ALL;
      return ODBC_SUCCESS;
    }

  stmt_handle->is_prepared = _FALSE_;

  rc = odbc_prepare (stmt_handle, StatementText);
  ERROR_GOTO (rc, error);

  rc = odbc_execute (stmt_handle);
  ERROR_GOTO (rc, error);

error:
  ODBC_RETURN (rc, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLDescribeColLinux (SQLHSTMT StatementHandle,
		     SQLUSMALLINT ColumnNumber,
		     SQLCHAR *ColumnName,
		     SQLSMALLINT BufferLength,
		     SQLSMALLINT *NameLength,
		     SQLSMALLINT *DataType, SQLULEN *ColumnSize, SQLSMALLINT *DecimalDigits, SQLSMALLINT *Nullable)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle = (ODBC_STATEMENT *) StatementHandle;;

  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_describe_col (stmt_handle, ColumnNumber,
			  ColumnName, BufferLength, NameLength, DataType, ColumnSize, DecimalDigits, Nullable);

  ODBC_RETURN (rc, StatementHandle);
}
