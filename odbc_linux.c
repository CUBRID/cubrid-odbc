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

#include	<stdio.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<iconv.h>
#include	<locale.h>
#include	<wchar.h>
#include	<errno.h>
#include	"odbc_portable.h"
#include	"odbcinst.h"
#include	"odbc_resource.h"
#include	"odbc_connection.h"
#include	"odbc_util.h"

#if defined AIX
#define assert
#if !defined (CAST_STRLEN)
#define CAST_STRLEN (int)
#endif /* CAST_STRLEN */
#endif /* AIX */

#ifndef OLECHAR
typedef WCHAR OLECHAR;
#endif

#ifndef BSTR
typedef OLECHAR *BSTR;
#endif

PUBLIC INT_PTR CALLBACK
ConfigDSNDlgProc (HWND hwndParent, UINT message, WPARAM wParam,
		                  LPARAM lParam);

static void set_unicode_char_set ();
#define UNICODE_DEFAULT_CODE "UCS-2"

#include <stdarg.h>

void OutputDebugString (const char *str, ...)
{
#if defined (LINUX_ODBC_DEBUG)
  va_list args;
  char sz_out [4096];
  char logfile [256];

  memset (sz_out, 0, 4096);
  va_start (args, str);
  vsprintf (sz_out, str, args);
  va_end (args);

  FILE *fp, *fopen();

  sprintf (logfile, "%s/tmp/proto.sql", getenv("HOME"));
  fp = fopen (logfile, "a");
  if (fp == NULL)
    {
      return;
    }

  fprintf (fp, sz_out);
  fclose (fp);
#endif /* LINUX_ODBC_DEBUG */
}

WCHAR *
SysAllocStringLen (
		const OLECHAR *strIn,
		UINT ui)
{
	WCHAR *p;

	p = calloc (1, ui * sizeof(WCHAR));
	return p;
}

void 
SysFreeString(WCHAR * bstrString)
{
	if (bstrString)
	  {
		free (bstrString);
		bstrString = NULL;
	  }
}

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

#define TRAN_BUF_SIZE 4096

static char *unicode_charset = NULL;

int MultiByteToWideChar (int codepage, DWORD dwFlags, char *lpMultiByteStr, int cbMultiByte, 
		wchar_t *lpWideCharStr, int cchWideChar)
{
  char *charset;
  wchar_t *iconv_out = lpWideCharStr;
  char *iconv_in = lpMultiByteStr;
  size_t iconv_in_len = cbMultiByte;
  size_t iconv_out_len = cchWideChar;
  size_t iconv_out_org = iconv_out_len;
  wchar_t _buf [TRAN_BUF_SIZE];
  iconv_t cd;
  int ret, required;

  if (unicode_charset == NULL)
    {
      set_unicode_char_set ();
    }

  if (cchWideChar == 0)
    {
      iconv_out_len = TRAN_BUF_SIZE;
      iconv_out_org = TRAN_BUF_SIZE;
      iconv_out = &_buf [0];
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

  if ((cd = iconv_open (unicode_charset, charset)) < 0)
    {
      OutputDebugString ("ERROR: iconv_open");
      return -1;
    }

  ret = iconv (cd, &iconv_in, &iconv_in_len, &iconv_out, &iconv_out_len);

  iconv_close (cd);

  if (ret < 0)
    {
      return -1;
    }

  required = (iconv_out_org - iconv_out_len) / sizeof (unsigned short);

  OutputDebugString ("iconv-pre: org = %d, iconv_out = %d required = %d\n",
                  iconv_out_org, iconv_out_len, iconv_out_org - iconv_out_len);
  OutputDebugString ("iconv: (%s) '%s' to UNICODE, rc=%d): requested len = %d, processed/required len  = %d, str = |%s|\n",
                  cchWideChar == 0 ? "Query" : "Convert from", charset, ret, cbMultiByte, required, lpMultiByteStr);

  return required;
}


int WideCharToMultiByte (int wincode,
				 int dw, 
				 wchar_t *str, 
				 int size, 
				 char *out_buffer, 
				 int cbMultiByte, 
				 char *lpdefaultchar, 
				 char *lpusedfdefaultchar)
{

  char *charset;
  iconv_t cd;
  char *iconv_out = out_buffer;
  unsigned char *iconv_in = (unsigned char *) str;
  size_t iconv_in_len = (size_t) sizeof (unsigned short) * size;
  size_t iconv_out_len = (size_t) cbMultiByte;
  size_t iconv_out_org = (size_t) cbMultiByte;
  int ret, required;
  char _buf [TRAN_BUF_SIZE];

  if (unicode_charset == NULL)
    {
      set_unicode_char_set ();
    }

  if (out_buffer == NULL)
   {
     iconv_out = _buf;
     iconv_out_org = (size_t) TRAN_BUF_SIZE;
     iconv_out_len = (size_t) TRAN_BUF_SIZE;
   }

  if (size < 0)
   {
     size = TRAN_BUF_SIZE/sizeof(wchar_t);
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

  if ((cd = iconv_open (charset, unicode_charset)) < 0)
    {
      return -1;
    }
  memset (iconv_out, 0, iconv_out_len);
  ret = iconv (cd, &iconv_in, &iconv_in_len, &iconv_out, &iconv_out_len);

  iconv_close (cd);

  if (ret < 0)
    {
      return -1;
    }

  required = iconv_out_org - iconv_out_len;

  OutputDebugString ("iconv (to '%s' rc = %d): unicode chars = %d, iconv_out = %d, Multibyte = %d, req/proc = %d, org len = %d, outlen = %d (%d)\n",
            charset, ret, size, iconv_out_len, cbMultiByte, required, iconv_out_org, required, iconv_in_len);


  return required;
}

static void
set_unicode_char_set ()
{

  char *charset;
  char buf [512];
  int ret;

  ret = SQLGetPrivateProfileString ("system", KEYWORD_NLS_NCHAR, "", buf,
                                    sizeof (buf), "ODBC.INI");
  if (ret != 0)
    {
      unicode_charset = UNICODE_DEFAULT_CODE;
    }
  else
    {
      unicode_charset = UT_MAKE_STRING (buf, -1);
    }
}

/*
 * ODBC Driver function not supported
 * We may support those function in the future
 * Mar 22, 2019
 */

ODBC_INTERFACE RETCODE SQL_API
  SQLDataSources (SQLHENV EnvironmentHandle,
                  SQLUSMALLINT Direction,
                  SQLCHAR * ServerName,
                  SQLSMALLINT BufferLength1,
                  SQLSMALLINT * NameLength1Ptr,
                  SQLCHAR * Description,
                  SQLSMALLINT BufferLength2, SQLSMALLINT * NameLength2Ptr)
{
    RETCODE rc = SQL_SUCCESS;

    OutputDebugString ("SQLDataSources called\n");

    return (rc);
}

BOOL SQLSetConfigMode (UWORD wConfigMode)
{
	BOOL rc;

	switch (wConfigMode)
	{
		case ODBC_USER_DSN:
		case ODBC_SYSTEM_DSN:
		case ODBC_BOTH_DSN:
			rc = TRUE;
		default:
			rc = FALSE;
			break;
	}

	return rc;
}

#define NELEM	4096
int INSTAPI SQLGetPrivateProfileString (
	LPCSTR lpszSection,
	LPCSTR lpszEntry,
	LPCSTR lpszDefault,
	LPSTR lpszRetBuffer,
	int cbRetBuffer,
	LPCSTR lpszFilename)
{
	int rc = SQL_ERROR;
	char	inifile[_MAX_PATH];
	char	filename[_MAX_PATH];
	char	element_list[NELEM];
	char *envp, *p;

	OutputDebugString ("SQLGetPrivateProfileString called: ");

	envp = getenv ("ODBCINI");
	if (envp == NULL)
	  {
	    tolower_str (filename, lpszFilename);
	    snprintf (inifile, _MAX_PATH, "%s/.%s", getenv("HOME"), filename);
	  }
	else
	  {
	    snprintf (inifile, _MAX_PATH, "%s", envp);
	  }

	memset (element_list, 0, NELEM);
	PRINT_DEBUG ("file = %s, \n", inifile);
	if (get_section_from_file (inifile, lpszSection, element_list, NELEM) < 0)
	  {
	    return -1;
	  }
	PRINT_DEBUG ("sec = %s, %s |%s|\n", lpszSection, lpszEntry, element_list);

       if ((p = element_value_by_key (element_list, lpszEntry)) == NULL)
         {
           return rc;
         }

	PRINT_DEBUG ("val = %s\n", lpszRetBuffer);

       strcpy (lpszRetBuffer,  p); 

       PRINT_DEBUG ("Profile: %s\n", lpszRetBuffer);
	return 0;
}

/*
 * Version Introduced: ODBC 1.0 Standards Compliance: Deprecated
 */
SQLRETURN SQL_API SQLSetConnectOption (
	SQLHDBC ConnectionHandle,
        SQLUSMALLINT Option,
	SQLULEN Value)
{
	return SQL_ERROR;
}

PUBLIC INT_PTR CALLBACK
ConfigDSNDlgProc (HWND hwndParent, UINT message, WPARAM wParam,
                  LPARAM lParam)
{
	OutputDebugString ("ConfigDSNDlgProc called\n");
	return NULL;
}

/*
 * Microsoft Windows Specific function
 */
INT_PTR DialogBoxParam(
  HINSTANCE hInstance,
  LPCSTR    lpTemplateName,
  HWND      hWndParent,
  void      *lpDialogFunc,
  LPARAM    dwInitParam)
{
	return NULL;	/* Not Supported */
}


/*
 * intl_utf8_to_cp() - converts a UTF-8 encoded char to unicode codepoint
 *  return: unicode code point; 0xffffffff means error
 *  utf8(in) : buffer for UTF-8 char
 *  size(in) : size of buffer
 *  next_char(in/out): pointer to next character
 *
 */
unsigned int
intl_utf8_to_cp (const unsigned char *utf8, const int size, unsigned char **next_char)
{
  assert (utf8 != NULL);
  assert (size > 0);
  assert (next_char != NULL);

  if (utf8[0] < 0x80)
    {
      *next_char = (unsigned char *) utf8 + 1;
      return (unsigned int) (utf8[0]);
    }
  else if (size >= 2 && utf8[0] >= 0xc0 && utf8[0] < 0xe0)
    {
      *next_char = (unsigned char *) utf8 + 2;
      return (unsigned int) (((utf8[0] & 0x1f) << 6) | (utf8[1] & 0x3f));
    }
  else if (size >= 3 && utf8[0] >= 0xe0 && utf8[0] < 0xf0)
    {
      *next_char = (unsigned char *) utf8 + 3;
      return (unsigned int) (((utf8[0] & 0x0f) << 12) | ((utf8[1] & 0x3f) << 6) | (utf8[2] & 0x3f));
    }
  else if (size >= 4 && utf8[0] >= 0xf0 && utf8[0] < 0xf8)
    {
      *next_char = (unsigned char *) utf8 + 4;
      return (unsigned int) (((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3f) << 12) | ((utf8[2] & 0x3f) << 6) |
                             (utf8[3] & 0x3f));
    }
#if INTL_UTF8_MAX_CHAR_SIZE > 4
  else if (size >= 5 && utf8[0] >= 0xf8 && utf8[0] < 0xfc)
    {
      *next_char = (unsigned char *) utf8 + 5;
      return (unsigned int) (((utf8[0] & 0x03) << 24) | ((utf8[1] & 0x3f) << 18) | ((utf8[2] & 0x3f) << 12) |
                             ((utf8[3] & 0x3f) << 6) | (utf8[4] & 0x3f));
    }
  else if (size >= 6 && utf8[0] >= 0xfc && utf8[0] < 0xfe)
    {
      *next_char = (unsigned char *) utf8 + 6;
      return (unsigned int) (((utf8[0] & 0x01) << 30) | ((utf8[1] & 0x3f) << 24) | ((utf8[2] & 0x3f) << 18) |
                             ((utf8[3] & 0x3f) << 12) | ((utf8[4] & 0x3f) << 6) | (utf8[5] & 0x3f));
    }
#endif

  *next_char = (unsigned char *) utf8 + 1;
  return 0xffffffff;
}

/*
 * intl_utf8_to_iso88591() - converts a buffer containing UTF8 text to ISO88591
 *
 *   return: 0 conversion ok, 1 conversion done, but invalid characters where
 *           found
 *   in_buf(in): buffer
 *   in_size(in): size of input string (NUL terminator not included)
 *   out_buf(int/out) : output buffer : uses the pre-allocated buffer passed
 *                      as input or a new allocated buffer;
 *   out_size(out): size of string (NUL terminator not included)
 */
int
intl_utf8_to_iso88591 (const unsigned char *in_buf, const int in_size, unsigned char **out_buf, int *out_size)
{
  const unsigned char *p_in = NULL;
  const unsigned char *p_end;
  unsigned char *p_out = NULL;
  unsigned char *next_utf8;
  int status = 0;
  unsigned int unicode_cp = 0;

  assert (in_size > 0);
  assert (in_buf != NULL);
  assert (out_buf != NULL);
  assert (out_size != NULL);

  for (p_in = in_buf, p_end = in_buf + in_size, p_out = (unsigned char *) *out_buf; p_in < p_end;)
    {
      unicode_cp = intl_utf8_to_cp (p_in, CAST_STRLEN (p_end - p_in), &next_utf8);

      if ((unicode_cp > 0xFF) || ((unicode_cp >= 0x7F) && (unicode_cp <= 0x9F)))
        {
          *p_out++ = '?';
          status = 1;
        }
      else
        {
          *p_out++ = unicode_cp;
        }

      p_in = next_utf8;
    }

  *out_size = CAST_STRLEN (p_out - *(out_buf));

  return status;
}
