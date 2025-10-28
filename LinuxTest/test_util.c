#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <iconv.h>
#include <string.h>
#include <stdlib.h>
#include "test_util.h"

#define WCHAR_LENGTH 2
#define CODE_NAME_UNICODE "UCS2"
#define CODE_NAME_UTF8 "UTF-8"

#define UT_ALLOC(size) (calloc(1,size))


int
bytes_to_wide_char (char *str, int size, wchar_t **buffer, int buffer_length, int *out_length, char *characterset)
{
  size_t wbuf_len = buffer_length;
  size_t mbuf_len;
  char *wbuf, *wbuf_orig;
  char *mbuf = str;
  iconv_t cd;
  size_t out_bytes_left;
  int rc;
  size_t bytes_required = 0;
  int num_chars;

  if (str == NULL || buffer == NULL || (buffer_length > 0 && *buffer == NULL))
    {
      goto ret;
    }

  num_chars = size > 0 ? size : strlen (str);

  wbuf = (char *) *buffer;
  if (buffer_length == 0)
    {
      wbuf_len = num_chars * WCHAR_LENGTH + sizeof (wchar_t);
      wbuf = (char *) UT_ALLOC (wbuf_len);
      if (wbuf == NULL)
	{
	  return -1;
	}
      wbuf_orig = wbuf;
    }

  mbuf_len = size > 0 ? size : (num_chars + 1);
  out_bytes_left = wbuf_len;

  if ((cd = iconv_open (CODE_NAME_UNICODE, CODE_NAME_UTF8)) < 0)
    {
      return -1;
    }

  rc = iconv (cd, &mbuf, &mbuf_len, &wbuf, &out_bytes_left);

  if (rc < 0)
    {
      iconv_close (cd);
      return -1;
    }

  iconv_close (cd);
  bytes_required = rc < 0 ? 0 : (wbuf_len - out_bytes_left);

  if (buffer_length == 0 || buffer_length > (bytes_required + 1))
    {
      * ((wchar_t *) wbuf) = L'\0';
      *buffer = (wchar_t *) wbuf_orig;
    }

ret:
  if (out_length != NULL)
    {
      *out_length = bytes_required;
    }

  return 0;
}

int
wide_char_to_bytes (wchar_t *str, int num_chars, char **target, int *out_length, char *characterset)
{
  iconv_t cd;
  size_t wbuf_len;
  size_t mbuf_len;
  char *mbuf, *mbuf_orig;
  char *wbuf = (char *) str;
  size_t bytes_required;
  size_t out_bytes_left;
  int rc;
  int alloc = 0;
  int num_wchars;

  if (str == NULL)
    {
      if (out_length)
	{
	  *out_length = 0;
	}

      if (target)
	{
	  *target = NULL;
	}

      return SQL_SUCCESS;
    }

  num_wchars = num_chars > 0 ? num_chars : sqlwcharlen (str);
  wbuf_len = (num_wchars) * WCHAR_LENGTH;
  mbuf_len = num_wchars * LENGTH_RATIO_WCHAR_TO_MULTIBYTE + 1;

  if ((mbuf_orig = (char *) UT_ALLOC (mbuf_len)) == NULL)
    {
      return -1;
    }

  mbuf = mbuf_orig;
  out_bytes_left = mbuf_len;

  if ((cd = iconv_open (CODE_NAME_UTF8, CODE_NAME_UNICODE)) < 0)
    {
      return -1;
    }

  if ((rc = iconv (cd, (char **)&wbuf, &wbuf_len, &mbuf, &out_bytes_left)) < 0)
    {
      return -1;
    }

  iconv_close (cd);

  bytes_required = mbuf_len - out_bytes_left;

  if (out_length)
    {
      *out_length = bytes_required;
    }

  if (target)
    {
      if (mbuf_len > bytes_required)
	{
	  *target = UT_REALLOC (mbuf_orig, bytes_required);
	}
      else
	{
	  *target = mbuf_orig;
	}
    }

  return SQL_SUCCESS;
}

int
sqlwcharlen (const wchar_t *wstr)
{
  int len = 0;
  char *p = (char *) wstr;

  while (p)
    {
      if (*p == 0)
	{
	  break;
	}

      p = p + WCHAR_LENGTH;
      ++len;
    }

  return len;
}

RETCODE
run_query_w (SQLHSTMT hstmt, SQLCHAR *query)
{
  RETCODE retcode;
  SQLWCHAR *query_buf;

  bytes_to_wide_char (query, strlen (query), &query_buf, 0, NULL, "UCS2");
  retcode = SQLExecDirectW (hstmt, query_buf, SQL_NTS);

  UT_FREE (query_buf);
  return retcode;
}
