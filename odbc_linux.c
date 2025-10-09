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
#include  "odbc_util.h"
#include  "ini.h"
#include  "odbcinstext.h"

typedef enum
{
  ODBCINI_DSN_NOT_FOUND = 0,
  ODBCINI_DSN_FOUND_USER,
  ODBCINI_DSN_FOUND_SYSTEM
} ODBCINI_DSN_LOOKUP_RESULT;

PUBLIC INT_PTR CALLBACK ConfigDSNDlgProc (HWND hwndParent, UINT message, WPARAM wParam, LPARAM lParam);
static char *find_key (char *string, char *key);
static void dsn2connstr (CUBRIDDSNItem *dsn, char *connstr);
extern char *odbcinst_system_file_path (char *b1);
extern char *odbcinst_system_file_name (char *b2);
extern char *odbcinst_user_file_path (char *buffer);
extern char *odbcinst_user_file_name (char *buffer);
extern BOOL _odbcinst_SystemINI (char *pszFileName, BOOL bVerify);
extern BOOL _odbcinst_UserINI (char *pszFileName, BOOL bVerify);
static ODBCINI_DSN_LOOKUP_RESULT ini_fileopen (const char *dsn, HINI *hInip);

#define FETCH_SIZE_DEFAULT	1
#define AUTOCOMMIT_DEFAULT	"false"
#define OMIT_SCHEMA_DEFAULT	"off"

#define DSN_LOOKUP(conn,ptDSN,hIni,key,dsn_item)						\
	do {											\
	     char *pDSN_item;									\
	     if ((pDSN_item = find_key (conn, key)) == NULL)					\
	       {										\
		 if (iniPropertySeek( hIni, ptDSN, key, "" ) == INI_SUCCESS)			\
		   {										\
		     snprintf (dsn_item, ITEMBUFLEN, "%s", hIni->hCurProperty->szValue);	\
		   }										\
	       }										\
	     else										\
	       {										\
		 snprintf (dsn_item, ITEMBUFLEN, "%s", pDSN_item);				\
		 UT_FREE (pDSN_item);								\
	       }										\
	   } while (0)



/*
 * ODBC Driver function not supported
 * We may support those function in the future
 * Mar 22, 2019
 */

ODBC_INTERFACE RETCODE SQL_API
SQLDataSources (SQLHENV EnvironmentHandle,
		SQLUSMALLINT Direction,
		SQLCHAR *ServerName,
		SQLSMALLINT BufferLength1,
		SQLSMALLINT *NameLength1Ptr,
		SQLCHAR *Description, SQLSMALLINT BufferLength2, SQLSMALLINT *NameLength2Ptr)
{
  OutputDebugString ("SQLDataSources called\n");
  return SQL_SUCCESS;
}

BOOL
SQLSetConfigMode (UWORD mode)
{
  BOOL rc;

  switch (mode)
    {
    case ODBC_USER_DSN:
    case ODBC_SYSTEM_DSN:
    case ODBC_BOTH_DSN:
      rc = TRUE;
      break;
    default:
      rc = FALSE;
      break;
    }

  return rc;
}

int INSTAPI
SQLGetPrivateProfileString (LPCSTR lpszSection,
			    LPCSTR lpszEntry,
			    LPCSTR lpszDefault, LPSTR lpszRetBuffer, int cbRetBuffer, LPCSTR lpszFilename)
{
  int rc = SQL_SUCCESS;
  char *envp;
  HINI hIni;
  char szFileName [512];
  struct stat sb;
  int found = 0;

  OutputDebugString ("SQLGetPrivateProfileString called");

  memset (&hIni, 0, sizeof (hIni));

  if (lpszFilename && stat (lpszFilename, &sb) == 0)
    {
      snprintf (szFileName, sizeof (szFileName), "%s", lpszFilename);
      if (iniOpen ( &hIni, szFileName, "#;", '[', ']', '=', TRUE) != INI_SUCCESS)
	{
	  return rc;
	}
    }
  else
    {
      if ((rc = ini_fileopen (lpszSection, &hIni)) == ODBCINI_DSN_NOT_FOUND)
	{
	  return SQL_ERROR;
	}
    }

  if (iniPropertySeek (hIni, lpszSection, lpszEntry, "") == INI_SUCCESS)
    {
      found = 1;
    }

  if (lpszRetBuffer && cbRetBuffer > 0)
    {
      snprintf (lpszRetBuffer, cbRetBuffer, "%s", found ? hIni->hCurProperty->szValue : lpszDefault);
      rc = strlen (lpszRetBuffer);
    }

  iniClose (hIni);

  return rc;
}

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
  const char *ConnStrIn = NULL;
  CUBRIDDSNItem dsn;
  char connstr_buf[1024] = "";
  char charset[ITEMBUFLEN] = CODE_NAME_UNICODE;
  char autocommit[ITEMBUFLEN] = AUTOCOMMIT_DEFAULT;
  char omit_schema[ITEMBUFLEN] = OMIT_SCHEMA_DEFAULT;

  memset (&dsn, 0, sizeof (dsn));
  if ((ptDSN = find_key (szConnStrIn, KEYWORD_DSN)) == NULL)
    {
      return ODBC_ERROR;
    }
  snprintf (dsn.dsn, ITEMBUFLEN, "%s", ptDSN);

  memset (&hIni, 0, sizeof (hIni));
  if ((rc = ini_fileopen (ptDSN, &hIni)) == ODBCINI_DSN_NOT_FOUND)
    {
      return ODBC_ERROR;
    }

  DSN_LOOKUP (szConnStrIn, ptDSN, hIni, KEYWORD_DBNAME, dsn.db_name);
  DSN_LOOKUP (szConnStrIn, ptDSN, hIni, KEYWORD_USER, dsn.user);
  DSN_LOOKUP (szConnStrIn, ptDSN, hIni, KEYWORD_PASSWORD, dsn.password);
  DSN_LOOKUP (szConnStrIn, ptDSN, hIni, KEYWORD_SERVER, dsn.server);
  DSN_LOOKUP (szConnStrIn, ptDSN, hIni, KEYWORD_PORT, dsn.port);
  port = atoi (dsn.port);
  DSN_LOOKUP (szConnStrIn, ptDSN, hIni, KEYWORD_CHARSET, dsn.charset);
  snprintf (charset, sizeof (charset), "%s", strlen (dsn.charset) ? dsn.charset : CODE_NAME_UNICODE);
  DSN_LOOKUP (szConnStrIn, ptDSN, hIni, KEYWORD_AUTOCOMMIT, dsn.autocommit);
  snprintf (autocommit, sizeof (autocommit), "%s", strlen (dsn.autocommit) ? dsn.autocommit : AUTOCOMMIT_DEFAULT);
  DSN_LOOKUP (szConnStrIn, ptDSN, hIni, KEYWORD_OMIT_SCHEMA, dsn.omit_schema);
  snprintf (omit_schema, sizeof (omit_schema), "%s", strlen (dsn.omit_schema) ? dsn.omit_schema : OMIT_SCHEMA_DEFAULT);
  DSN_LOOKUP (szConnStrIn, ptDSN, hIni, KEYWORD_FETCH_SIZE, dsn.fetch_size);
  fetch_size = strlen (dsn.fetch_size) ? atoi (dsn.fetch_size) : FETCH_SIZE_DEFAULT;

  iniClose (hIni);

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

/*
 * Version Introduced: ODBC 1.0 Standards Compliance: Deprecated
 */
SQLRETURN SQL_API
SQLSetConnectOption (SQLHDBC ConnectionHandle, SQLUSMALLINT Option, SQLULEN Value)
{
  return SQL_ERROR;
}

/*
 * Windows specific functions
 * emulate or dummy function definitions
 */

PUBLIC INT_PTR CALLBACK
ConfigDSNDlgProc (HWND hwndParent, UINT message, WPARAM wParam, LPARAM lParam)
{
  OutputDebugString ("ConfigDSNDlgProc called\n");
  return NULL;
}

INT_PTR
DialogBoxParam (HINSTANCE hInst, LPCSTR tmpNaae, HWND hWndP, INT_PTR CALLBACK lpDialogFunc, LPARAM dwInit)
{
  return NULL;
}

void
OutputDebugString (const char *str, ...)
{
  return;
}

/*
 * itoa() - converts an integer to char string
 */

char *
itoa (int value, char *string, int radix)
{
  char tmp[33];
  char *tp = tmp;
  int i;
  unsigned v;
  int sign;
  char *sp;

  if (radix > 36 || radix <= 1)
    {
      return 0;
    }

  sign = (radix == 10 && value < 0);

  if (sign)
    {
      v = -value;
    }
  else
    {
      v = (unsigned) value;
    }

  while (v || tp == tmp)
    {
      i = v % radix;
      v = v / radix;
      if (i < 10)
	{
	  *tp++ = i + '0';
	}
      else
	{
	  *tp++ = i + 'a' - 10;
	}
    }

  if (string == NULL)
    {
      string = (char *) malloc ((tp - tmp) + sign + 1);
      if (string == NULL)
	{
	  return string;
	}
    }
  sp = string;

  if (sign)
    {
      *sp++ = '-';
    }
  while (tp > tmp)
    {
      *sp++ = *--tp;
    }
  *sp = '\0';
  return string;
}

static char *
find_key (char *string, char *key)
{
  char *value_p, *buf, *ptr;
  size_t len;

  value_p = element_value_by_key (string, key);

  if (value_p == NULL)
    {
      return NULL;
    }

  len = strlen (value_p);

  if ((buf = UT_ALLOC (len + 1)) == NULL)
    {
      return NULL;
    }

  strncpy (buf, value_p, len);

  ptr = strchr (buf, ';');
  if (ptr)
    {
      *ptr = '\0';
    }

  return buf;
}

static void
dsn2connstr (CUBRIDDSNItem *dsn, char *connstr)
{
  if (connstr == NULL)
    {
      return;
    }

  if (strlen (dsn->dsn))
    {
      sprintf (connstr, "%s=%s;", KEYWORD_DSN, dsn->dsn);
    }

  APPEND_TO_CONNSTR (connstr, KEYWORD_DBNAME, dsn->db_name);
  APPEND_TO_CONNSTR (connstr, KEYWORD_USER, dsn->user);
  APPEND_TO_CONNSTR (connstr, KEYWORD_PASSWORD, dsn->password);
  APPEND_TO_CONNSTR (connstr, KEYWORD_SERVER, dsn->server);
  APPEND_TO_CONNSTR (connstr, KEYWORD_PORT, dsn->port);
  APPEND_TO_CONNSTR (connstr, KEYWORD_FETCH_SIZE, dsn->fetch_size);
  APPEND_TO_CONNSTR (connstr, KEYWORD_CHARSET, dsn->charset);
  APPEND_TO_CONNSTR (connstr, KEYWORD_AUTOCOMMIT, dsn->autocommit);
  APPEND_TO_CONNSTR (connstr, KEYWORD_OMIT_SCHEMA, dsn->omit_schema);

  return;
}

PUBLIC char *
ut_make_string_linux (const char *src, int length)
{
  char *new = NULL;
  size_t size;

  if (src == NULL)
    {
      return NULL;
    }

  size = (size_t) (length < 0 ? strlen (src) : length) + 1;

  if ((new = (char *) UT_ALLOC (size)) == NULL)
    {
      return NULL;
    }

  snprintf (new, size, "%s", src);

  return new;
}

/************************************************************************
 * * name: ini_fileopen
 * * arguments:
 * *   const char *dsn
 * *   HINI *hInip
 * * returns/side-effects:
 * * description:
 * Lookup dsn file, if a dsn exists in multiple ini files following
 * order will be applied.
 * 1. $ODBCINI
 * 2. $HOME/.odbc.ini
 * 3. system ini
************************************************************************/

static ODBCINI_DSN_LOOKUP_RESULT
ini_fileopen (const char *dsn, HINI *hInip)
{
  char szIniName[_MAX_PATH];
  int rc = ODBCINI_DSN_NOT_FOUND;

  if (_odbcinst_UserINI (szIniName, FALSE))
    {
      if (iniOpen (hInip, szIniName, "#;", '[', ']', '=', TRUE) == INI_SUCCESS)
	{
	  if (iniPropertySeek ((HINI) *hInip, dsn, "", "") == INI_SUCCESS)
	    {
	      return ODBCINI_DSN_FOUND_USER;
	    }

	  iniClose ((HINI) *hInip);
	}
    }

  if (_odbcinst_SystemINI (szIniName, FALSE))
    {
      if (iniOpen (hInip, szIniName, "#;", '[', ']', '=', TRUE) == INI_SUCCESS)
	{
	  if (iniPropertySeek ((HINI) *hInip, dsn, "", "") == INI_SUCCESS)
	    {
	      return ODBCINI_DSN_FOUND_SYSTEM;
	    }

	  iniClose ((HINI) *hInip);
	}
    }

  return ODBCINI_DSN_NOT_FOUND;
}
