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
#define PROF_BUF_SIZE 4096

#define LENGTH_RATIO_WCHAR_TO_MULTIBYTE 3 // we expect maximum 3 bytes required for a wide charcter
#define REQUIRED_SIZE_ONLY  0
#define ERR_CONV_FAILED     -1
#define ERR_NO_MEMORY       -2
#define CODE_NAME_EUCKR   "EUCKR"
#define CODE_NAME_UTF8    "UTF-8"
#define CODE_NAME_UNICODE "UCS2"
#define NULL_CHAR ((char)'\0')

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
WideCharToMultiByte (int CodePage,
		     int dwFlags,
		     wchar_t *lpWideCharStr,
		     int cchWideChar, char *lpMultiByteStr, int cbMultiByte, char *lpDefaultChar, char *lpUsedDefaultChar)
{
  char *charset = CODE_NAME_UTF8;
  char *unicode_charset = CODE_NAME_UNICODE;
  uint16_t *wstr = lpWideCharStr;
  char *mb_str = lpMultiByteStr;
  char *mb_str_orig;
  size_t wstr_len, mb_len, out_bytes_left;
  iconv_t cd;
  int rc;
  int bytes_required;

  if (cchWideChar == 0)
    {
      return ERR_CONV_FAILED;
    }

  wstr_len = cchWideChar < 0 ? wcslen (lpWideCharStr) * sizeof (wchar_t) : cchWideChar * 2;

  if (cbMultiByte == REQUIRED_SIZE_ONLY)
    {
      mb_len = wstr_len * LENGTH_RATIO_WCHAR_TO_MULTIBYTE;
      if ((mb_str = (char *) calloc (1, wstr_len * LENGTH_RATIO_WCHAR_TO_MULTIBYTE)) == NULL)
	{
	  return ERR_NO_MEMORY;
	}
      mb_str_orig = mb_str;
    }
  else
    {
      mb_len = cbMultiByte;
    }

  out_bytes_left = mb_len;

  if (CodePage == CP_EUC_KR)
    {
      charset = CODE_NAME_EUCKR;
    }

  if ((cd = iconv_open (charset, unicode_charset)) < 0)
    {
      return ERR_CONV_FAILED;
    }

  rc = iconv (cd, (char **)&wstr, &wstr_len, &mb_str, &out_bytes_left);

  bytes_required = rc < 0 ? ERR_CONV_FAILED : (mb_len - out_bytes_left);

  if (cbMultiByte == 0 && mb_str != NULL)
    {
      bytes_required += sizeof (NULL_CHAR);
      free (mb_str_orig);
    }

  rc = iconv_close (cd);
  return (cd < 0 ? ERR_CONV_FAILED : bytes_required);
}

int
MultiByteToWideChar (int CodePage, DWORD dwFlags, char *lpMultiByteStr, int cbMultiByte,
		     wchar_t *lpWideCharStr, int cchWideChar)
{
  char *unicode_charset = CODE_NAME_UNICODE;
  char *charset = CODE_NAME_UTF8;
  wchar_t *wstr = lpWideCharStr;
  wchar_t *wstr_orig = NULL;
  char *mb_str = lpMultiByteStr;
  size_t wstr_len = cchWideChar;
  size_t mb_len, out_bytes_left;
  int bytes_required;
  iconv_t cd;
  int rc;
  int mb_length;

  if (cbMultiByte == 0)
    {
      return ERR_CONV_FAILED;
    }

  mb_len = cbMultiByte < 0 ? strlen (lpMultiByteStr) : cbMultiByte;
  mb_length = mb_len;

  if (cchWideChar == REQUIRED_SIZE_ONLY)
    {
      wstr_len = sizeof (wchar_t) * mb_len;
      if ((wstr = (wchar_t *) calloc (sizeof (wchar_t), mb_len)) == NULL)
	{
	  return ERR_NO_MEMORY;
	}
      wstr_orig = wstr;
    }

  out_bytes_left = wstr_len;

  if (CodePage == CP_EUC_KR)
    {
      charset = CODE_NAME_EUCKR;
    }

  if ((cd = iconv_open (unicode_charset, charset)) < 0)
    {
      return ERR_CONV_FAILED;
    }

  rc = iconv (cd, &mb_str, &mb_len, &wstr, &out_bytes_left);

  bytes_required = rc < 0 ? ERR_CONV_FAILED : (wstr_len - out_bytes_left);

  if (cchWideChar > 0 && rc >= 0)
    {
      * ((wchar_t *) wstr) = L'\0';
    }

  if (cchWideChar == 0 && wstr_orig != NULL)
    {
      bytes_required += sizeof (wchar_t);
      free (wstr_orig);
    }

  rc = iconv_close (cd);

  return (cd < 0 ? ERR_CONV_FAILED : bytes_required);
}
