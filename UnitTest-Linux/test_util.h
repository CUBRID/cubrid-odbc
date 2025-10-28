#ifndef _TEST_UTIL_H
#define _TEST_UTIL_H
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
int bytes_to_wide_char (char *str, int size, wchar_t **buf, int buf_len, int *out_len, char *charset);
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
