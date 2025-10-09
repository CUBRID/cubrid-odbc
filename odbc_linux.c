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
#include  "odbc_linux.h"

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

char *
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

void
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

ODBCINI_DSN_LOOKUP_RESULT
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
