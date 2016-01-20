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

#ifndef	__CUBRID_ODBC_UTIL_HEADER	/* to avoid multiple inclusion */
#define	__CUBRID_ODBC_UTIL_HEADER

#include		"odbc_portable.h"

#define		UT_ALLOC(size)				ut_alloc(size)
#define		UT_REALLOC(ptr, size)		ut_realloc(ptr, size)
#define		UT_FREE(ptr)				ut_free(ptr)
#define		UT_MAKE_STRING(ptr, length)	ut_make_string(ptr, length)
#define		UT_MAKE_BINARY(ptr, length) ut_make_binary(ptr, length)
#define		UT_APPEND_STRING(str1, str2, len2)	ut_append_string(str1, str2, len2)

#define		UT_SET_DELIMITER			";;"

/* NC_FREE - NULL check free 
 * NA_FREE - NULL assign free
 * MOVE_STRING - free & copy string
 * NC_FREE_WH - NULL check free with handler
 * NA_FREE_WH - NULL assign free with handler
 * NC_FREE_WHO - NULL check free with handler, option
 * NA_FREE_WHO - NULL assign free with handler, option
 */
#define		NC_FREE(ptr)	if ( ptr != NULL ) UT_FREE(ptr)
#define		NA_FREE(ptr)									\
	do {													\
		if ( ptr != NULL ) {								\
			UT_FREE(ptr);									\
			ptr= NULL;										\
		}													\
	} while (0)

#define		NC_FREE_WH(handle, ptr)  if ( ptr != NULL ) handle(ptr)
#define		NA_FREE_WH(handle, ptr)							\
	do {													\
		if ( ptr != NULL ) {								\
			handle(ptr);									\
			ptr = NULL;										\
		}													\
	} while (0)												\

#define		NC_FREE_WHO(handle, ptr, opt)  if ( ptr != NULL ) handle(ptr, opt)
#define		NA_FREE_WHO(handle, ptr, opt)					\
	do {													\
		if ( ptr != NULL ) {								\
			handle(ptr, opt);								\
			ptr = NULL;										\
		}													\
	} while (0)												\

#define		UT_COPY_STRING(target, value)						\
	NC_FREE(target);										\
	target = UT_MAKE_STRING(value, -1)

#define SET_OPTION(value, option)       ((value) |= (option))
#define UNSET_OPTION(value, option)     ((value) ^= (option))
#define IS_OPTION_SETTED(value, option)    (( (value) & (option) ) == (option))

//////////////////   debug log
#ifdef __DEBUG_LOG
#ifdef UNIX
#include <sys/time.h>
#include <stdlib.h>

#define DEBUG_FILE_KEY          "DEBUG_TIMESTAMP_FILE"
#define DEBUG_TIMESTAMP(value)                          \
                        do {                                            \
                            struct timeval now;                         \
                            FILE        *fp = NULL;                     \
                            char        *pt;                            \
                            pt = getenv(DEBUG_FILE_KEY);                \
                            if ( pt != NULL ) {                         \
                                fp = fopen(pt, "a+");                   \
                                gettimeofday(&now, NULL);               \
                                fprintf(fp,#value "	%ld.%07ld	%s	%d\n", now.tv_sec, now.tv_usec, __FILE__, __LINE__);                         \
                                fclose(fp);                             \
                            }                                           \
                        } while(0)
#else
#include <sys/types.h>
#include <sys/timeb.h>

#define DEBUG_FILE_KEY          "d:\\lsj1888\\time_log.txt"
#define DEBUG_TIMESTAMP(value)
/*
                #define DEBUG_TIMESTAMP(value)                          \
                        do {                                            \
                            struct _timeb now;                          \
                            FILE        *fp = NULL;                     \
                            fp = fopen(DEBUG_FILE_KEY, "a+");           \
                            _ftime(&now);                       \
                            fprintf(fp,#value "	%ld.%07ld	%s	%d\n", now.time, now.millitm, __FILE__, __LINE__);                           \
                            fclose(fp);                         \
                        } while(0)
						*/
#endif
#else
#define DEBUG_TIMESTAMP(value)
#endif

#ifdef __DEBUG_LOG
#ifdef UNIX
#include <stdlib.h>

#define DEBUG_LOG_FILE_KEY          "DEBUG_LOG_FILE"
#define DEBUG_LOG(value)                          \
                        do {                                            \
                            FILE        *fp = NULL;                     \
                            char        *pt;                            \
                            pt = getenv(DEBUG_LOG_FILE_KEY);                \
                            if ( pt != NULL ) {                         \
                                fp = fopen(pt, "a+");                   \
                                fprintf(fp,"%s at %s %d\n", value, __FILE__, __LINE__);                         \
                                fclose(fp);                             \
                            }                                           \
                        } while(0)
#else

#define DEBUG_LOG_FILE_KEY          "c:\\odbc_log.txt"
#define DEBUG_LOG(value)                          \
                        do {                                            \
                            FILE        *fp = NULL;                     \
                            fp = fopen(DEBUG_LOG_FILE_KEY, "a+");           \
                            fprintf(fp,"%s at %s %d\n", value, __FILE__, __LINE__);                           \
                            fclose(fp);                         \
                        } while(0)
#endif
#else
#define DEBUG_LOG(value)
#endif

/* Dynamic string */
typedef struct __st_dynamic_string
{
  char *value;
  int totalSize;
  int usedSize;
} D_STRING;

typedef struct __st_dynamic_binary
{
  char *value;
  int size;
} D_BINARY;

typedef struct tagST_LIST
{
  void *key;
  void *value;
  struct tagST_LIST *next;
} ST_LIST;

PUBLIC void InitStr (D_STRING * str);
PUBLIC void FreeStr (D_STRING * str);

PUBLIC ERR_CODE MemcatImproved (D_STRING * dest, char *src, int srcSize);

PUBLIC void *ut_alloc (SQLLEN size);
PUBLIC void ut_free (void *ptr);
PUBLIC void *ut_realloc (void *ptr, int size);
PUBLIC char *ut_make_string (const char *src, int length);
PUBLIC char *ut_append_string (char *str1, char *str2, int len2);
PUBLIC char *ut_make_binary (const char *src, int length);

PUBLIC int element_from_setstring (char **current, char *buf);

PUBLIC void add_element_to_setstring (char *setstring, char *element);

PUBLIC char *odbc_trim (char *str);
PUBLIC RETCODE str_value_assign (const char *in_value,
				 char *out_buf,
				 SQLLEN out_buf_len, SQLLEN * val_len_ptr);
PUBLIC RETCODE bin_value_assign (const void *in_value,
				 SQLLEN in_val_len,
				 char *out_buf,
				 SQLLEN out_buf_len, SQLLEN * val_len_ptr);

PUBLIC short is_oidstr (char *str);
PUBLIC short is_oidstr_array (char **array, int size);
PUBLIC int replace_oid (char *sql_text, char **org_param_pos_pt,
			char **oid_param_pos_pt, char **oid_param_val_pt);

extern ERR_CODE ListTailAdd (ST_LIST * head, void *key, void *val,
			     ERR_CODE (*assignFunc) (ST_LIST *, void *,
						     void *));

extern void ListDelete (ST_LIST * head, void (*nodeDelete) (ST_LIST *));
extern ERR_CODE ListCreate (ST_LIST ** head);

extern ST_LIST *HeadNode (ST_LIST * dummyHead);
extern ST_LIST *NextNode (ST_LIST * node);

#ifdef _DEBUG
extern void ListPrint (ST_LIST * head, void (*nodePrint) (ST_LIST *));
#endif

/* node assign function */
extern ERR_CODE NodeAssign (ST_LIST * node, void *key, void *value);

/*-------------  connection string util	------------------------*/
PUBLIC const char *next_element (const char *element_list);
PUBLIC const char *element_value (const char *element);
PUBLIC const char *element_value_by_key (const char *element_list,
					 const char *key);

#endif /* ! __CUBRID_ODBC_UTIL_HEADER */
