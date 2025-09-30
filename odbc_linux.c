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

PUBLIC INT_PTR CALLBACK ConfigDSNDlgProc (HWND hwndParent, UINT message, WPARAM wParam, LPARAM lParam);
static char *find_key (char *string, char *key);
static void dsn2connstr (CUBRIDDSNItem *dsn, char *connstr);

#define LINE_SIZE 512
#define TBUF_SIZE 8192
#define PROF_BUF_SIZE 4096
#define FETCH_SIZE_DEFAULT 1
#define AUTOCOMMIT_DEFAULT "false"
#define OMIT_SCHEMA_DEFAULT "off"

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

  if (stat (lpszFilename, &sb) == 0)
    {
      snprintf (szFileName, sizeof (szFileName), "%s", lpszFilename);
    }
  else if ((envp = getenv ("ODBCINI")) != NULL)
    {
      snprintf (szFileName, sizeof (szFileName), "%s", envp);
    }
  else
    {
      snprintf (szFileName, sizeof (szFileName), "%s/.odbc.ini", getenv ("HOME"));
    }
  if (iniOpen ( &hIni, szFileName, "#;", '[', ']', '=', TRUE) != INI_SUCCESS)
    {
      return rc;
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
  char ini_file[_MAX_PATH];
  const char *ptDSN;
  int port, fetch_size;
  const char *ConnStrIn = NULL;
  CUBRIDDSNItem dsn;
  char connstr_buf[1024] = "";
  char charset[ITEMBUFLEN] = CODE_NAME_UNICODE;
  char autocommit[ITEMBUFLEN] = AUTOCOMMIT_DEFAULT;
  char omit_schema[ITEMBUFLEN] = OMIT_SCHEMA_DEFAULT;

  memset (&dsn, 0, sizeof (dsn));
  snprintf (ini_file, sizeof (ini_file), "%s/.odbc.ini", getenv ("HOME"));

  if (iniOpen (&hIni, ini_file, "#;", '[', ']', '=', TRUE) != INI_SUCCESS)
    {
      return ODBC_ERROR;
    }

  if ((ptDSN = find_key (szConnStrIn, KEYWORD_DSN)) == NULL)
    {
      return ODBC_ERROR;
    }

  snprintf (dsn.dsn, ITEMBUFLEN, "%s", ptDSN);

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


int
MultiByteToWideChar (int codepage, DWORD dwFlags, char *lpMultiByteStr, int cbMultiByte,
		     wchar_t *lpWideCharStr, int cchWideChar)
{
  char *default_unicode_charset = "UTF-16LE";
  char *charset = "UTF-8";
  void *iconv_out = lpWideCharStr;
  char *iconv_in = lpMultiByteStr;
  size_t iconv_in_len = cbMultiByte;
  size_t iconv_out_len = cchWideChar * sizeof (wchar_t);
  size_t iconv_out_org = iconv_out_len;
  wchar_t _buf[TBUF_SIZE];
  char outbuf[TBUF_SIZE], *op = outbuf;
  iconv_t cd;
  int ret, required;

  memset (outbuf, 0, TBUF_SIZE);
  if (cchWideChar == 0)
    {
      iconv_out_len = TBUF_SIZE;
      iconv_out_org = TBUF_SIZE;
      iconv_out = &_buf[0];
    }

  switch (codepage)
    {
    case CP_EUC_KR:
      charset = "EUCKR";
      break;
    case CP_UTF8:
    case CP_ACP:
      charset = "UTF-8";
      break;
    default:
      charset = "UTF-8";
      break;
    }

  if ((cd = iconv_open (default_unicode_charset, charset)) < 0)
    {
      return -1;
    }

  ret = iconv (cd, &iconv_in, &iconv_in_len, &op, &iconv_out_len);
  iconv_close (cd);

  if (ret < 0)
    {
      return -1;
    }

  required = (iconv_out_org - iconv_out_len);

  if (cchWideChar != 0)
    {
      memcpy (iconv_out, op, required);
    }

  return required;
}


int
WideCharToMultiByte (int wincode,
		     int dw,
		     wchar_t *str,
		     int size, char *out_buffer, int cbMultiByte, char *lpdefaultchar, char *lpusedfdefaultchar)
{
  char *charset;
  char *default_unicode_charset = "UTF-16";	// UCS2, UCS2-LE
  iconv_t cd;
  char *iconv_out = out_buffer;
  unsigned char *iconv_in = (unsigned char *) str;

  size_t iconv_in_len = (size_t) size;
  size_t iconv_out_len = (size_t) cbMultiByte;
  size_t iconv_out_org = (size_t) cbMultiByte;
  int ret, required;
  char _buf[TBUF_SIZE];
  char inbuf[TBUF_SIZE], *ip = inbuf;

  memset (inbuf, 0, TBUF_SIZE);

  if (size > 0)
    {
      memcpy (inbuf, &iconv_in[0], size);
    }

  memset (_buf, 0, TBUF_SIZE);

  if (out_buffer == NULL)
    {
      iconv_out = _buf;
      iconv_out_org = (size_t) TBUF_SIZE;
      iconv_out_len = (size_t) TBUF_SIZE;
    }

  if (size < 0)
    {
      size = TBUF_SIZE / sizeof (wchar_t);
    }

  switch (wincode)
    {
    case CP_UTF8:
      charset = "UTF-8";
      break;
    case CP_EUC_KR:
      charset = "EUCKR";
      break;
    default:
      charset = "UTF-8";
      break;
    }

  if ((cd = iconv_open (charset, default_unicode_charset)) < 0)
    {
      return -1;
    }

  memset (iconv_out, 0, iconv_out_len);
  ret = iconv (cd, &ip, &iconv_in_len, &iconv_out, &iconv_out_len);

  iconv_close (cd);

  if (ret < 0)
    {
      return -1;
    }

  required = iconv_out_org - iconv_out_len;

  return required;

}

static char *
find_key (char *string, char *key)
{
  char *value_p, *buf, *ptr;
  int len;

  value_p = element_value_by_key (string, key);

  if (value_p == NULL)
    {
      return NULL;
    }

  len = strlen (value_p);

  if ((buf = UT_ALLOC (len)) == NULL)
    {
      return NULL;
    }

  snprintf (buf, len, "%s", value_p);

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
