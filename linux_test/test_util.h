#ifndef _TEST_UTIL_H
#define _TEST_UTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
static int caseno=1;

#define AreNotEqual(v,expect)		\
    do {				\
	if (v == expect || v == SQL_INVALID_HANDLE)		\
	  {				\
	    printf ("testcase%d-%d: NOK (retcode = %d)\n", case_num, caseno++, v);	\
	    return (1);			\
	  }				\
	else				\
	  {				\
	    printf ("testcase%d-%d: OK\n", case_num, caseno++);	\
	  }				\
        }				\
    while (0)

/* Unlike AreNotEqual, these keep going after a mismatch and count it into nok,
   so one run reports every broken attribute instead of only the first. */
#define ReportEqual(nok,what,v,expect)					\
    do {								\
	long _v = (long) (v);						\
	long _e = (long) (expect);					\
	if (_v == _e)							\
	  {								\
	    printf ("testcase%d-%d: %s OK\n", case_num, caseno++, what);	\
	  }								\
	else								\
	  {								\
	    printf ("testcase%d-%d: %s NOK (%ld, expected %ld)\n",	\
		    case_num, caseno++, what, _v, _e);			\
	    (nok)++;							\
	  }								\
        }								\
    while (0)

#define ReportEqualStr(nok,what,v,expect)				\
    do {								\
	const char *_v = (const char *) (v);				\
	const char *_e = (const char *) (expect);			\
	if (_v != NULL && strcmp (_v, _e) == 0)				\
	  {								\
	    printf ("testcase%d-%d: %s OK\n", case_num, caseno++, what);	\
	  }								\
	else								\
	  {								\
	    printf ("testcase%d-%d: %s NOK (\"%s\", expected \"%s\")\n",	\
		    case_num, caseno++, what, _v ? _v : "(null)", _e);	\
	    (nok)++;							\
	  }								\
        }								\
    while (0)

int bytes_to_wide_char (char *str, int size, wchar_t **buf, int buf_len, int *out_len, char *charset);
int wide_char_to_bytes (wchar_t *str, int num_chars, char **target, int *out_len, char *charset);
/* counts UCS-2 units, which is what the wchar_t * buffers above really hold */
int sqlwcharlen (const wchar_t *wstr);
#define LENGTH_RATIO_WCHAR_TO_MULTIBYTE 3

#if !defined (UT_REALLOC)
#define UT_REALLOC(ptr,size) (realloc(ptr, size))
#endif
extern char *sqltype_name (short sqltype);
extern char *sqlinfo_name (int infotype);

#if !defined (UT_FREE)
#define UT_FREE(ptr)					\
			do {                        	\
				if (ptr != NULL) {	\
					free (ptr);	\
					ptr = NULL;	\
				}			\
			} while (0)
#endif
#endif
