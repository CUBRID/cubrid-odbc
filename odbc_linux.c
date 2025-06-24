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

static int get_section_from_file (const char *ini, const char *section, char *value_p, int size);

PUBLIC INT_PTR CALLBACK ConfigDSNDlgProc (HWND hwndParent, UINT message, WPARAM wParam, LPARAM lParam);

#define LINE_SIZE 512
#define TBUF_SIZE 8192
#define PROF_BUF_SIZE 4096

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
  int rc = SQL_ERROR;
  char inifile[_MAX_PATH];
  char filename[_MAX_PATH];
  char element_list[PROF_BUF_SIZE];
  char *envp, *p;

  OutputDebugString ("SQLGetPrivateProfileString called");

  envp = getenv ("ODBCINI");
  if (envp != NULL)
    {
      snprintf (inifile, _MAX_PATH, "%s", envp);
    }
  else
    {
      tolower_str (filename, lpszFilename);
      snprintf (inifile, _MAX_PATH, "%s/.%s", getenv ("HOME"), filename);
    }

  memset (element_list, 0, PROF_BUF_SIZE);
  if (get_section_from_file (inifile, lpszSection, element_list, PROF_BUF_SIZE) < 0)
    {
      return -1;
    }

  if ((p = element_value_by_key (element_list, lpszEntry)) == NULL)
    {
      return rc;
    }

  strcpy (lpszRetBuffer, p);

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


WCHAR *
SysAllocStringLen (const WCHAR *strIn, UINT ui)
{
  WCHAR *p;

  p = calloc (1, ui * sizeof (WCHAR));
  return p;
}

void
SysFreeString (WCHAR *bstr)
{
  if (bstr)
    {
      free (bstr);
      bstr = NULL;
    }
}

void
OutputDebugString (const char *str, ...)
{
  return;
}

/*
 * Linux Specific
 */

static int
get_section_from_file (const char *ini, const char *section, char *value_p, int size)
{
  FILE *fp, *fopen ();
  char buf[LINE_SIZE];
  char *p, *pt;
  int rc = -1;
  int found = 0;

  if (ini == NULL || section == NULL || value_p == NULL || size < 1)
    {
      return -1;
    }

  if ((fp = fopen (ini, "r")) == NULL)
    {
      return 1;
    }

  while (!feof (fp))
    {
      if (fgets (buf, LINE_SIZE, fp))
	{
	  if (buf[0] == '[')
	    {
	      p = strchr (buf, ']');
	      if (p != NULL)
		{
		  *p = '\0';
		  if (strcasecmp (&buf[1], section) == 0)
		    {
		      found = 1;
		      break;
		    }
		}
	    }
	  else
	    {
	      continue;
	    }
	}
    }

  if (!found)
    {
      return -1;
    }

  while (!feof (fp))
    {
      if (fgets (buf, LINE_SIZE, fp))
	{
	  p = strchr (buf, '\n');
	  if (p)
	    {
	      *p = '\0';
	    }

	  if (buf[0] == '\0' || buf[0] == '#' || buf[0] == '[')
	    {
	      rc = 0;
	      break;
	    }

	  strcat (value_p, buf);
	  strcat (value_p, ";");
	  rc = 0;
	}
    }

  fclose (fp);
  for (pt = value_p; *pt != '\0'; ++pt)
    {
      if (*pt == ';')		// connection string delimiter
	{
	  *pt = '\0';
	}
    }

  return rc;
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
