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

#include		<stdio.h>
#include		<stdlib.h>
#include		<string.h>

#include		"odbc_portable.h"
#include		"odbc_util.h"
#include		"odbc_statement.h"

#define		UNIT_MEMORY_SIZE		256
#define		STK_SIZE        100

#ifndef CP_EUC_KR
#define CP_EUC_KR 51949 
#endif

static int is_korean (unsigned char ch);

#pragma comment(lib, "legacy_stdio_definitions.lib")



/************************************************************************
* name:  InitStr
* arguments: 
*	D_STRING str;
* returns/side-effects: 
*	
* description: 
* NOTE: 
************************************************************************/
PUBLIC void
InitStr (D_STRING * str)
{
  str->value = NULL;
  str->usedSize = 0;
  str->totalSize = 0;
}


/************************************************************************
* name: ReallocImproved
* arguments: 
*	dest - destination memory pointer
*	destSize - total size of dest
*	usedSize - used memory size of dest
*	allocSize - realloc size 
* returns/side-effects: 
*	destSize increased
* description: 
*	if remain size(destSize - usedSize)  < alloc size, 
*	realloc a bunch of bytes  
* NOTE: 
************************************************************************/
PUBLIC ERR_CODE
ReallocImproved (char **dest, int *destSize, int usedSize, int allocSize)
{
  int remainDestSize;

  remainDestSize = *destSize - usedSize;

  /* MUST reserve one byte for extra */
  while (remainDestSize <= allocSize)
    {
      if (*dest == NULL)
	{
	  *dest = UT_ALLOC (UNIT_MEMORY_SIZE);
	  if (*dest == NULL)
	    {
	      *destSize = 0;
	      return -1;
	    }
	  memset (*dest, 0, UNIT_MEMORY_SIZE);
	  *destSize = UNIT_MEMORY_SIZE;
	  remainDestSize = UNIT_MEMORY_SIZE;

	}
      else
	{
	  *dest = (char *) UT_REALLOC (*dest, *destSize + UNIT_MEMORY_SIZE);
	  if (*dest == NULL)
	    {
	      *destSize = 0;
	      return -1;
	    }
	  memset (*dest + *destSize, 0, UNIT_MEMORY_SIZE);

	  *destSize += UNIT_MEMORY_SIZE;
	  remainDestSize += UNIT_MEMORY_SIZE;
	}
    }
  return 0;
}

/************************************************************************
* name:  MemcatImproved
* arguments: 
*	dest - destination pointer
*	usedSize - total alloc size = used size + remain size
*	src  - source memory pointer,
*	srcSize - source memory size
* returns/side-effects: 
*	OK, ERROR
* description: 
* 	Not string copy. 
*	So this can copy memory stream including null char(\0)
* NOTE: 
*	Using this function, 
*	initialize dest to null, usedSize to zero, remainSize to zero
*	At least, dest to NULL -> InisStr()
************************************************************************/
PUBLIC ERR_CODE
MemcatImproved (D_STRING * dest, char *src, int srcSize)
{

  if (dest->value == NULL)
    {
      dest->totalSize = 0;
      dest->usedSize = 0;
    }
  if (ReallocImproved (&(dest->value), &(dest->totalSize),
		       dest->usedSize, srcSize) < 0)
    {
      return -1;
    }
  memcpy (dest->value + dest->usedSize, src, srcSize);

  dest->usedSize += srcSize;

  return 0;
}

/************************************************************************
* name:  FreeStr
* arguments: 
*	node of D_STRING
* returns/side-effects: 
* description: 
* NOTE: 
************************************************************************/
PUBLIC void
FreeStr (D_STRING * str)
{
  if (str->value != NULL)
    {
      UT_FREE (str->value);
    }
  str->value = NULL;
  str->usedSize = 0;
  str->totalSize = 0;
}

/************************************************************************
* name: sqlwcharlen
* arguments:
*		int wstr
* returns/side-effects:
*		len - the length of wstr
* description:
* NOTE:
************************************************************************/
PUBLIC int 
sqlwcharlen(const WCHAR *wstr)
{
  int len= 0;
  while (wstr && *wstr++)
    ++len;
  return len;
}
/************************************************************************
* name: ut_alloc_bstr
* arguments:
*		int size - size of allocated memory
* returns/side-effects:
*		void* - memory pointer
* description:
* NOTE:
************************************************************************/
PUBLIC WCHAR* 
ut_alloc_bstr (SQLLEN size)
{
 WCHAR *new = NULL;

  new = SysAllocStringLen (NULL, size);

#ifdef _MEM_DEBUG
  if (new == NULL)
    {
      printf ("Alloc wchar: NULL\n");
    }
  else
    {
      printf ("Alloc wchar: %p\n", new);
    }
#endif

  return new;
}

/************************************************************************
* name: ut_free_bstr
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
PUBLIC void
ut_free_bstr  (WCHAR *ptr)
{
#ifdef _MEM_DEBUG
  if (ptr == NULL)
    {
      printf ("Free wchar: NULL\n");
    }
  else
    {
      printf ("Free wchar: %p\n", ptr);
    }
#endif
  if (ptr != NULL)
    SysFreeString (ptr);
}

/************************************************************************
* name: ut_alloc
* arguments:
*		int size - size of allocated memory
* returns/side-effects:
*		void* - memory pointer
* description:
* NOTE:
************************************************************************/
PUBLIC void *
ut_alloc (SQLLEN size)
{
  char *new = NULL;
  if(size == 0)
   {
     return NULL;
   }
  new = malloc (size);

#ifdef _MEM_DEBUG
  if (new == NULL)
    {
      printf ("Alloc : NULL\n");
    }
  else
    {
      printf ("Alloc : %p\n", new);
    }
#endif

  return new;
}

/************************************************************************
* name: ut_free
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
PUBLIC void
ut_free (void *ptr)
{
#ifdef _MEM_DEBUG
  if (ptr == NULL)
    {
      printf ("Free : NULL\n");
    }
  else
    {
      printf ("Free : %p\n", ptr);
    }
#endif
  if (ptr != NULL)
    free (ptr);
}

/************************************************************************
* name: ut_realloc
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
PUBLIC void *
ut_realloc (void *ptr, int size)
{
  void *new = NULL;

  new = realloc (ptr, size);

#ifdef _DEBUG
  if (new == NULL)
    {
      printf ("Realloc : NULL\n");
    }
  else
    {
      printf ("Realloc : %p\n", new);
    }
#endif

  return new;
}


/************************************************************************
* name: ut_make_string
* arguments:
*		const char *src - source string ( not always null-terminated string)
*		length -
*			if length < 0, copy all src(null-term).
*			else copy length characters and null-term.
* returns/side-effects:
*		char * - string pointer
* description:
* NOTE:
************************************************************************/
PUBLIC char *
ut_make_string (const char *src, int length)
{
  char *temp = NULL;

  if (src != NULL)
    {
      if (length < 0)
	{
	  temp = UT_ALLOC (strlen (src) + 1);
	  strcpy (temp, src);
	}
      else
	{
	  temp = UT_ALLOC (length + 1);
	  strncpy (temp, src, length);
	  temp[length] = '\0';
	}
    }

  return temp;
}

/************************************************************************
* name: ut_append_string
* arguments:
*		const char *src - source string ( not always null-terminated string)
*		length -
*			if length < 0, copy all src(null-term).
*			else copy length characters and null-term.
* returns/side-effects:
*		char * - string pointer
* description:
* NOTE:
************************************************************************/
PUBLIC char *
ut_append_string (char *str1, char *str2, int len2)
{
  char *temp = NULL;
  int str_size;

  if (str1 != NULL)
    {
      if (len2 < 0)
	{
	  temp = UT_REALLOC (str1, strlen (str1) + strlen (str2) + 1);
	  strcat (temp, str2);
	}
      else
	{
	  str_size = strlen (str1) + len2;
	  temp = UT_REALLOC (str1, str_size + 1);
	  strncpy (temp + strlen (str1), str2, len2);
	  temp[str_size] = '\0';
	}
    }
  else
    {
      temp = UT_MAKE_STRING (str2, len2);
    }

  return temp;
}

/************************************************************************
* name: ut_make_binary
* arguments:
*		const char *src - source binary
*		length -
*			if length < 0, NULL
*			else copy length binary
* returns/side-effects:
*		char * - string pointer
* description:
* NOTE:
************************************************************************/
PUBLIC char *
ut_make_binary (const char *src, int length)
{
  char *temp = NULL;

  if (src != NULL && length > 0)
    {
      temp = UT_ALLOC (length);
      memcpy (temp, src, length);
    }

  return temp;
}

/************************************************************************
* name: add_element_to_setstring
* arguments:
* NOTE:
************************************************************************/
PUBLIC void
add_element_to_setstring (char *setstring, char *element)
{
  if (setstring == NULL || element == NULL)
    {
      return;
    }

  if (setstring[0] == '\0')
    {				// empty set string
      strcpy (setstring, element);
    }
  else
    {
      strcat (setstring, UT_SET_DELIMITER);
      strcat (setstring, element);
    }
}



/************************************************************************
* name: element_from_setstring
* arguments:
* returns/side-effects:
*       0 false - no more data, 1 ture - value ok
* description:
*       setstring means set delimited by DELIMITER
*       eg) "1;;2;;5"
* NOTE:
************************************************************************/
PUBLIC int
element_from_setstring (char **current, char *buf)
{
  char *pt;

  if (*current == NULL || **current == '\0')
    {
      buf[0] = '\0';
      return 0;
    }


  pt = strstr (*current, UT_SET_DELIMITER);
  if (pt != NULL)
    {
      strncpy (buf, *current, pt - *current + 1);
      buf[pt - *current] = '\0';
      *current = pt + strlen (UT_SET_DELIMITER);
    }
  else
    {
      strcpy (buf, *current);
      *current = *current + strlen (*current);
    }


  return 1;
}

/*-----------------------------------------------------------------------
 *							Linked List
 *----------------------------------------------------------------------*/

PUBLIC ERR_CODE
ListTailAdd (ST_LIST * head, void *key, void *val,
	     ERR_CODE (*assignFunc) (ST_LIST *, void *, void *))
{
  ST_LIST *newNode;
  ST_LIST *temp;
  ERR_CODE retval;

  newNode = (ST_LIST *) UT_ALLOC (sizeof (ST_LIST));
  if (newNode == NULL)
    return -1;

  retval = (*assignFunc) (newNode, (void *) key, (void *) val);
  if (retval < 0)
    {
      UT_FREE (newNode);
      return -1;
    }

  /* find tail node */
  for (temp = head;; temp = temp->next)
    {
      if (temp->next == NULL)
	break;
    }

  temp->next = newNode;
  newNode->next = NULL;

  return 0;
}

/************************************************************************
 * name:        ListDelete - delete list				*
 *                                                                      *
 * arguments:   head    - list header					*
 *              nodeDelete - 'key', 'value' deallocation function	*
 *                                                                      *
 * returns/side-effects:                                                *
 *              (ST_LIST*) NULL						*
 *                                                                      *
 * description: delete all nodes of list.				*
 *									*
 * NOTE:        							*
 *  If 'key' or 'value' has its own memory, the memory must be freed	*
 *  in 'nodeDelete' function. But, the node container is freed in	*
 *  this module.							*
 *  dummy header포함하여 모두 지운다.
 ************************************************************************/

PUBLIC void
ListDelete (ST_LIST * head, void (*nodeDelete) (ST_LIST *))
{
  ST_LIST *temp;
  ST_LIST *del;

  temp = head;

  while (temp != NULL)
    {
      del = temp;
      temp = temp->next;
      if (nodeDelete)
	(*nodeDelete) (del);
      UT_FREE (del);
    }
}

/* Create dummy header */
PUBLIC ERR_CODE
ListCreate (ST_LIST ** head)
{

  *head = (ST_LIST *) UT_ALLOC (sizeof (ST_LIST));
  if (*head == NULL)
    return -1;
  (*head)->key = NULL;
  (*head)->value = NULL;
  (*head)->next = NULL;
  return 0;
}

#ifdef DEBUG
void PUBLIC
ListPrint (ST_LIST * head, void (*nodePrint) (ST_LIST *))
{
  ST_LIST *temp = head->next;

  while (temp != NULL)
    {
      (*nodePrint) (temp);
      temp = temp->next;
    }
}
#endif

/************************************************************************
* name: NodeHead
* arguments:
*	ST_LIST head;
* returns/side-effects:
* 	head node pointer - ST_LIST* 
* description:
* NOTE:
************************************************************************/

PUBLIC ST_LIST *
HeadNode (ST_LIST * dummyHead)
{
  if (dummyHead != NULL)
    {
      return dummyHead->next;
    }
  else
    {
      return NULL;
    }
}

/************************************************************************
* name: NodeNext
* arguments:
*	this node - ST_LIST*
* returns/side-effects:
*	next node - ST_LIST*
* description:
* NOTE:
************************************************************************/

PUBLIC ST_LIST *
NextNode (ST_LIST * node)
{
  if (node != NULL)
    {
      return node->next;
    }
  else
    {
      return NULL;
    }
}


/************************************************************************
* name:  NodeAssign
* arguments: 
*	node - data node
*	key  - key as string
*	value - value string
* returns/side-effects: 
*	OK , ERROR if malloc error
* description: 
* NOTE: 
************************************************************************/

PUBLIC ERR_CODE
NodeAssign (ST_LIST * node, void *key, void *value)
{
  if (key != NULL)
    {
      node->key = (char *) UT_ALLOC (strlen ((char *) key) + 1);
      if (node->key == NULL)
	return -1;

      strcpy ((char *) node->key, (char *) key);
    }
  else
    {
      node->key = NULL;
    }

  node->value = value;

  return 0;
}

PRIVATE int
is_korean (unsigned char ch)
{
  return (ch >= 0xb0 && ch <= 0xc8) || (ch >= 0xa1 && ch <= 0xfe);
}


/*-----------------------------------------------------------------------
 *						Connection string
 *----------------------------------------------------------------------*/

/************************************************************************
 * name: next_element
 * arguments:
 * returns/side-effects:
 *		더이상 value가 없을 때 NULL return
 * description:
 * NOTE:
 *	element list structure
 *		KEY1=VALUE1\0KEY2=VALUE2\0KEY3=VALUE3\0\0
 *		\0 - end of attribute
 *		\0 - end of list
 ************************************************************************/
PUBLIC const char *
next_element (const char *element_list)
{
  const char *pt;

  if (element_list == NULL)
    return NULL;

  pt = element_list;

  pt += strlen (element_list) + 1;	// +1 is for '\0'

  if (*pt == '\0')
    return NULL;		// end of list

  return pt;
}

/************************************************************************
 * name: attr_value
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
PUBLIC const char *
element_value (const char *element)
{
  const char *pt;

  if (element == NULL)
    return NULL;

  pt = strchr (element, '=');
  if (pt == NULL)
    return NULL;

  ++pt;				// for '='

  return pt;
}

/************************************************************************
 * name: element_value_by_key
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
PUBLIC const char *
element_value_by_key (const char *element_list, const char *key)
{
  const char *pt;
  const char *val;

  if (element_list == NULL || key == NULL)
    return NULL;

  pt = element_list;
  do
    {
      if (_strnicmp (pt, key, strlen (key)) == 0)
	{
	  val = element_value (pt);
	  return val;
	}
      pt = next_element (pt);
    }
  while (pt != NULL);

  return NULL;
}

/************************************************************************
 * name: is_oidstr
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 *	앞뒤의 공백문자를 허용한다.
 ************************************************************************/
PUBLIC short
is_oidstr_array (char **array, int size)
{
  int i;

  for (i = 0; i < size; ++i)
    {
      if (is_oidstr (array[i]) == _FALSE_)
	return _FALSE_;
    }

  return _TRUE_;
}

/************************************************************************
 * name: is_oidstr
 * arguments:
 * returns/side-effects:
 * description:
 *	string이 oid인지 판별한다. 
 * NOTE:
 *	앞뒤의 공백문자를 허용한다.
 ************************************************************************/
PUBLIC short
is_oidstr (char *str)
{
  char *pt;
  char state;


  for (pt = str, state = 0; *pt != '\0'; ++pt)
    {
      switch (state)
	{
	case 0:		// start
	  if (*pt == '@')
	    state = 1;
	  else if (*pt == ' ' || *pt == '\t')
	    break;
	  else
	    return _FALSE_;

	  break;

	case 1:		// first digit of page id
	case 3:		// first digit of slot id
	case 5:		// first digit of volume id
	  if (*pt >= '0' && *pt <= '9')
	    ++state;
	  else
	    {
	      return _FALSE_;
	    }
	  break;

	case 2:		// page id
	case 4:		// slot id
	  if (*pt == '|')
	    ++state;
	  else if (*pt < '0' || *pt > '9')
	    {
	      return _FALSE_;
	    }
	  break;

	case 6:
	  if (*pt == ' ' || *pt == '\t')
	    state = 7;
	  else if (*pt < '0' || *pt > '9')
	    {
	      return _FALSE_;
	    }

	  break;
	case 7:
	  if (*pt == ' ' || *pt == '\t')
	    break;
	  else
	    return _FALSE_;

	  break;

	default:
	  return _FALSE_;
	}
    }

  if (state != 6 && state != 7)
    {
      return _FALSE_;
    }

  return _TRUE_;
}


/************************************************************************
 * name:  replace_oid
 * arguments:
 * returns/side-effects:
 *		교체된 oid parameter 개수를 return한다.
 * description:
 * NOTE:
 *		parameter number는 1을 base로 하고 있다고 가정했다.
 ************************************************************************/
PUBLIC int
replace_oid (char *sql_text, char **org_param_pos_pt,
	     char **oid_param_pos_pt, char **oid_param_val_pt)
{
  char *oid_buf = NULL;
  char oid_param_pos[256];
  char org_param_pos[256];
  char oid_param_val[1024];
  char buf[126];
  char *pt;
  char *pt_tmp;
  char current_param_pos = 0;
  char oid_param_num = 0;

  org_param_pos[0] = '\0';
  oid_param_pos[0] = '\0';
  oid_param_val[0] = '\0';

  for (pt = sql_text; *pt != '\0'; ++pt)
    {
      switch (*pt)
	{
	case '?':
	  ++current_param_pos;
	  sprintf (buf, "%d", current_param_pos);
	  add_element_to_setstring (org_param_pos, buf);
	  break;

	case '\'':
	  // find the matched string marker
	  pt_tmp = pt;
	  while (pt_tmp = strchr (pt_tmp + 1, '\''))
	    {
	      if (pt_tmp == NULL || *(pt_tmp - 1) != '\\')
		break;

	    }
	  if (pt_tmp == NULL)
	    break;

	  oid_buf = UT_MAKE_STRING (pt + 1, (int) (pt_tmp - pt - 1));
	  odbc_trim (oid_buf);

	  if (is_oidstr (oid_buf) == _TRUE_)
	    {
	      ++current_param_pos;

	      ++oid_param_num;

	      sprintf (buf, "%d", current_param_pos);
	      add_element_to_setstring (oid_param_pos, buf);

	      sprintf (buf, "%s", oid_buf);
	      add_element_to_setstring (oid_param_val, buf);

	      // replace oid string value to parameter marker
	      *pt = '?';
	      memcpy (pt + 1, pt_tmp + 1, strlen (pt_tmp + 1) + 1);	// include Null terminator
	    }

	  NA_FREE (oid_buf);
	  break;
	}
    }

  if (org_param_pos_pt != NULL)
    *org_param_pos_pt = UT_MAKE_STRING (org_param_pos, -1);
  if (oid_param_pos_pt != NULL)
    *oid_param_pos_pt = UT_MAKE_STRING (oid_param_pos, -1);
  if (oid_param_val_pt != NULL)
    *oid_param_val_pt = UT_MAKE_STRING (oid_param_val, -1);

  return oid_param_num;

}


/************************************************************************
 * name: odbc_trim
 * arguments:
 * returns/side-effects:
 * description:
 *	전후의 공백문자를 제거한 형태로 변형한다. (str의 내용이 변형됨)
 * NOTE:
 ************************************************************************/
PUBLIC char *
odbc_trim (char *str)
{
  char *p;
  char *s;

  if (str == NULL)
    return (str);

  for (s = str;
       *s != '\0' && (*s == ' ' || *s == '\t' || *s == 0x0d || *s == 0x0a);
       s++)
    ;
  if (*s == '\0')
    {
      *str = '\0';
      return (str);
    }

  /* *s must be a non-white char */
  for (p = s; *p != '\0'; p++)
    ;
  for (p--; *p == ' ' || *p == '\t' || *p == 0x0d || *p == 0x0a; p--)
    ;
  *++p = '\0';

  if (s != str)
    memcpy (str, s, strlen (s) + 1);

  return (str);
}

/************************************************************************
 * name: str_value_assign
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
PUBLIC RETCODE
str_value_assign (const char *in_value,
		  char *out_buf, SQLLEN out_buf_len, SQLLEN * val_len_ptr)
{
  RETCODE rc = ODBC_SUCCESS;


  if (in_value != NULL)
    {
      if (out_buf != NULL && out_buf_len > 0)
	{
	  strncpy (out_buf, in_value, out_buf_len - 1);
	  out_buf[out_buf_len - 1] = '\0';
	}

      if ((unsigned int) out_buf_len <= strlen (in_value))
	rc = ODBC_SUCCESS_WITH_INFO;
    }

  if (val_len_ptr != NULL)
    {
      *val_len_ptr = ODBC_STRLEN_IND (in_value);
    }

  return rc;
}


/************************************************************************
 * name: bin_value_assign
 * arguments:
 *		in_value - input binary start pointer
 *		in_val_len - input binary length
 *		out_buf - output binary start pointer
 *		out_buf_len - output binary length
 *		val_len_ptr - input binary length or null indicator
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
PUBLIC RETCODE
bin_value_assign (const void *in_value,
		  SQLLEN in_val_len,
		  char *out_buf, SQLLEN out_buf_len, SQLLEN * val_len_ptr)
{
  RETCODE rc = ODBC_SUCCESS;

  if (in_value != NULL)
    {
      if (out_buf != NULL)
	{
	  memcpy (out_buf, in_value, MIN (out_buf_len, in_val_len));
	}

      if (out_buf_len < in_val_len)
	{
	  rc = ODBC_SUCCESS_WITH_INFO;
	}
    }

  if (val_len_ptr != NULL)
    {
      if (in_value == NULL)
	{
	  *val_len_ptr = SQL_NULL_DATA;
	}
      else
	{
	  *val_len_ptr = in_val_len;
	}
    }

  return rc;
}
/************************************************************************
 * name: bin_value_assign
 * arguments:
 *		stmt - ODBC_STATEMENT
 *		lob - pointer of lob
 *		type - SQL_BLOB or SQL_CLOB
 *		out_buf - output binary start pointer
 *		out_buf_len - output binary length
 *		val_len_ptr - input binary length or null indicator
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
PUBLIC RETCODE
lob_value_assign (
                   ODBC_STATEMENT * stmt,
                   UNI_CCI_A_TYPE lob,
                   SQLSMALLINT type,
                   char *out_buf, 
                   SQLLEN out_buf_len, 
                   SQLLEN * val_len_ptr
                   )
{
  RETCODE rc = ODBC_SUCCESS;
  SQLLEN in_val_len = 0;
  T_CCI_ERROR error;
  long pos=stmt->column_data.offset;
  
  if(type == SQL_BLOB)
  {
    in_val_len = (SQLLEN)cci_blob_size(lob.blob);
    if (out_buf != NULL && out_buf_len != 0)
     {
       cci_blob_read(stmt->conn->connhd,
                     lob.blob,pos,
                     (int)MIN (out_buf_len, in_val_len-pos),
                     out_buf,&error);
       if(error.err_code < 0)
       {
         rc = SQL_ERROR;
       }
     }
     else
     {
       pos=stmt->column_data.offset=0;
     }
  }
  else if(type == SQL_CLOB)
  {
    in_val_len = (SQLLEN)cci_clob_size(lob.clob);
    if (out_buf != NULL && out_buf_len > 0)
     {
       cci_clob_read(stmt->conn->connhd,
                     lob.clob,pos,
                     (int)MIN (out_buf_len, in_val_len-pos),
                     out_buf,&error);
       if(error.err_code < 0)
       {
         rc = SQL_ERROR;
       }
     } 
      else
        {
          pos = stmt->column_data.offset = 0;
        }
    }

  if (out_buf_len < in_val_len - pos)
    {
      rc = ODBC_SUCCESS_WITH_INFO;
    }

  if (val_len_ptr != NULL)
    {
      if (lob.blob == NULL)
        {
          *val_len_ptr = SQL_NULL_DATA;
        }
      else
        {
          *val_len_ptr = in_val_len-pos;
        }
    }

    if (rc == ODBC_SUCCESS_WITH_INFO)
      {
        if (out_buf_len > 0)
         {
           pos += (long) out_buf_len;
         }

      stmt->column_data.current_pt = out_buf + pos;
      stmt->column_data.remain_length = (int) (in_val_len - pos);
      stmt->column_data.lob = lob;
      stmt->column_data.offset = (int)pos;

      odbc_set_diag (stmt->diag, "01004", 0, NULL);
    }
  else
    {
      stmt->column_data.remain_length = 0;
      if (type == SQL_BLOB)
        {
          cci_blob_free (lob.blob);
        }
      else if (type == SQL_CLOB)
        {
          cci_clob_free (lob.clob);
        }
    }

  return rc;
}

/************************************************************************
 * name: tolower_str
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
PUBLIC RETCODE
tolower_str (char *dst, const char *src)
{
  int i = 0;
  if (src == NULL || dst == NULL)
    {
      return ODBC_ERROR;
    }

  while (src[i])
    {
      dst[i] = tolower (src[i]);
      i++;
    }
  dst[i] = '\0';
  return ODBC_SUCCESS;
}

/************************************************************************
 * name: assemble_conn_attr
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
PUBLIC char*
assemble_conn_attr (const char *conn_str_in,
                    const char *key,
                    char *buffer)
{
  const char* value =  element_value_by_key (conn_str_in, key);
  if (value != NULL)
    {
      tolower_str (buffer + strlen (buffer), key);
      strcat (buffer, "=");
      strcat (buffer, value);
      strcat (buffer, "&");
    }
  return value;
}

/************************************************************************
 * name: get_connect_attr
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
PUBLIC void
get_connect_attr (ODBC_CONNECTION_ATTR *attr, const char *conn_str_in, char *buffer)
{
  if (conn_str_in == NULL || buffer == NULL || attr == NULL)
    {
      return;
    }
    
  attr->altHosts = assemble_conn_attr (conn_str_in, KEYWORD_ALTHOSTS, buffer);
  
  attr->disconnect_on_query_timeout =
    assemble_conn_attr (conn_str_in, KEYWORD_DISCONNECTONQUERYTIMEOUT, buffer);
  if (attr->disconnect_on_query_timeout == NULL)
    {
      attr->disconnect_on_query_timeout =
        assemble_conn_attr (conn_str_in, KEYWORD_DISCONNECT_ON_QUERY_TIMEOUT, buffer);
    }
    
  attr->loadBalance = assemble_conn_attr (conn_str_in, KEYWORD_LOADBALANCE, buffer);
  attr->logBaseDir = assemble_conn_attr (conn_str_in, KEYWORD_LOGBASEDIR, buffer);
  attr->logFile = assemble_conn_attr (conn_str_in, KEYWORD_LOGFILE, buffer);
  attr->logSlowQueries = assemble_conn_attr (conn_str_in, KEYWORD_LOGSLOWQUERIES, buffer);
  attr->logTraceApi = assemble_conn_attr (conn_str_in, KEYWORD_LOGTRACEAPI, buffer);
  attr->logTraceNetwork = assemble_conn_attr (conn_str_in, KEYWORD_LOGTRACENETWORK, buffer);

  attr->query_timeout = assemble_conn_attr (conn_str_in, KEYWORD_QUERYTIMEOUT, buffer);
  if (attr->query_timeout == NULL)
    {
      attr->query_timeout = assemble_conn_attr (conn_str_in, KEYWORD_QUERY_TIMEOUT, buffer);
    }

  attr->login_timeout = assemble_conn_attr (conn_str_in, KEYWORD_LOGINTIMEOUT, buffer);
  if (attr->login_timeout == NULL)
    {
      attr->login_timeout = assemble_conn_attr (conn_str_in, KEYWORD_LOGIN_TIMEOUT, buffer);
    }

  attr->rcTime = assemble_conn_attr (conn_str_in, KEYWORD_RCTIME, buffer);
  attr->slowQueryThresholdMillis =
    assemble_conn_attr (conn_str_in, KEYWORD_SLOWQUERYTHRESHOLDMILLIS, buffer);

  if (buffer[strlen (buffer) - 1] == '&')
    {
      buffer[strlen (buffer) - 1] = '\0';
    }
}

/************************************************************************
* name: encode_string_to_charset
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/
int
encode_string_to_charset(wchar_t *str, int size, char **target, int* out_length, char *charset)
{
    int nLength;
    char *tmp_string;
    int wincode = CP_ACP;

    if (charset == NULL || _stricmp(charset, "utf-8") == 0)
      {
        wincode = CP_UTF8;
      }
    else if (_stricmp(charset, "euc-kr") == 0)
      {
        wincode = CP_EUC_KR;
      }

    nLength = WideCharToMultiByte(wincode, 0, str, -1, NULL, 0, NULL, NULL);
    tmp_string = (char *)ut_alloc(sizeof(char) * (nLength + 1));
    if (tmp_string == NULL)
      {
        return CCI_ER_NO_MORE_MEMORY;
      }

    nLength = WideCharToMultiByte(wincode, 0, str, -1, tmp_string, nLength, NULL, NULL);
    if (target)
      {
        *target = tmp_string;
      }

    if (out_length)
      {
        *out_length = nLength;
      }

    return ODBC_SUCCESS;
}

/************************************************************************
 * name: encode_string_to_utf8
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
int
encode_string_to_utf8 (wchar_t *str, int size, char **target,  int* out_length)
{
  int nLength;
  char *tmp_string;
  int wincode = CP_UTF8;

  nLength = WideCharToMultiByte (wincode, 0, str, -1, NULL, 0, NULL,
				 NULL);
  tmp_string = (char *) ut_alloc (sizeof (char) * (nLength + 1));
  if (tmp_string == NULL)
    {
      return CCI_ER_NO_MORE_MEMORY;
    }

  nLength = WideCharToMultiByte (wincode, 0, str, -1, tmp_string, nLength, NULL,
		       NULL);
  if (target)
    {
      *target = tmp_string;
    }
    
    if(out_length)
     {
       * out_length = nLength;
     }
    
  return ODBC_SUCCESS;
}

/************************************************************************
 * name: encode_string
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
int
wide_char_to_bytes (wchar_t *str, int size, char **target, int* out_length, char* characterset)
{
  if(size <= 0)
    { 
      size = sqlwcharlen(str);
    }

  return encode_string_to_charset(str, size, target, out_length, characterset);
}

/************************************************************************
 * name: decode_string
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
int
bytes_to_wide_char (char *str, int size, wchar_t **buffer, int buffer_length, int* out_length, char* characterset)
{
  int nLength;
  int wincode = CP_ACP;
  wchar_t* temp_buffer = *buffer;
  int temp_buffer_length = buffer_length;
  
  if (characterset == NULL || _stricmp(characterset, "utf-8") == 0)
    {
      wincode = CP_UTF8;
    }
  else if (_stricmp(characterset, "euc-kr") == 0)
    {
      wincode = CP_EUC_KR;
    }

  if(str == NULL || buffer == NULL)
   {
     return ODBC_SUCCESS;
   }
  
  temp_buffer_length = MultiByteToWideChar (wincode, 0, (LPCSTR) str, size, NULL, 0);
  if(buffer_length==0) // need to malloc buffer
    {
      temp_buffer = UT_ALLOC_BSTR (temp_buffer_length);
      if (temp_buffer == NULL)
        {
          return CCI_ER_NO_MORE_MEMORY;
        }
      buffer_length = temp_buffer_length;
   } 
   
  nLength = MultiByteToWideChar (wincode, 0, (LPCSTR) str, size, temp_buffer, buffer_length);
  temp_buffer[nLength] = '\0';
  *buffer = temp_buffer;
  //SysFreeString (bstrCode);
  if(out_length != NULL)
    {
      *out_length = nLength * sizeof(wchar_t);
    }
  if ( buffer_length < temp_buffer_length)
    {
      return ODBC_SUCCESS_WITH_INFO;
    }  
  return ODBC_SUCCESS;
}

/************************************************************************
 * name: decode_string
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 ************************************************************************/
int
get_wide_char_result (char *str, int size, wchar_t **buffer, int buffer_length, int* out_length, char* characterset)
{
  int nLength;
  int wincode = CP_ACP;
  int temp_buffer_length = buffer_length;
  int rc = ODBC_SUCCESS;
  
  if (characterset == NULL || _stricmp (characterset, "utf-8") == 0)
    {
      wincode = CP_UTF8;
    }
  else if (_stricmp(characterset, "euc-kr") == 0)
    {
      wincode = CP_EUC_KR;
    }

  if(str == NULL || buffer == NULL)
   {
     return ODBC_SUCCESS;
   }
  
  temp_buffer_length = MultiByteToWideChar (wincode, 0, (LPCSTR) str, size, NULL, 0);
  if(buffer_length == 0)
   {
     *out_length = temp_buffer_length * sizeof(wchar_t);
     return ODBC_SUCCESS_WITH_INFO;
   }
   
  nLength = MultiByteToWideChar (wincode, 0, (LPCSTR) str, size, *buffer, buffer_length);
  //SysFreeString (bstrCode);
  if(out_length != NULL)
   {
     *out_length = nLength * sizeof(wchar_t);
   }
  if ( buffer_length < temp_buffer_length)
   {
      return ODBC_SUCCESS_WITH_INFO;
   }  
  return ODBC_SUCCESS;
}

#ifdef CUBRID_ODBC_UNICODE
PRIVATE _BOOL_ is_odd_number (int num)
{
  return (num & 1) ? _TRUE_ : _FALSE_;
}

PUBLIC int check_if_even_number (SQLUSMALLINT info_type, SQLSMALLINT buffer_length)
{
  if (buffer_length < 0)
    {
      /* ignore case */
      return ODBC_SUCCESS;
    }

  switch (info_type)
    {
      /* info_type that returns a string */
      case SQL_ACCESSIBLE_PROCEDURES:
      case SQL_ACCESSIBLE_TABLES:
      case SQL_CATALOG_NAME:
      case SQL_CATALOG_NAME_SEPARATOR:
      case SQL_CATALOG_TERM:
      case SQL_COLLATION_SEQ:
      case SQL_COLUMN_ALIAS:
      case SQL_DATA_SOURCE_NAME:
      case SQL_DATA_SOURCE_READ_ONLY:
      case SQL_DATABASE_NAME:
      case SQL_DBMS_NAME:
      case SQL_DBMS_VER:
      case SQL_DESCRIBE_PARAMETER:
      case SQL_DRIVER_NAME:
      case SQL_DRIVER_ODBC_VER:
      case SQL_DRIVER_VER:
      case SQL_EXPRESSIONS_IN_ORDERBY:
      case SQL_IDENTIFIER_QUOTE_CHAR:
      case SQL_INTEGRITY:
      case SQL_KEYWORDS:
      case SQL_LIKE_ESCAPE_CLAUSE:
      case SQL_MAX_ROW_SIZE_INCLUDES_LONG:
      case SQL_MULT_RESULT_SETS:
      case SQL_MULTIPLE_ACTIVE_TXN:
      case SQL_NEED_LONG_DATA_LEN:
      case SQL_ORDER_BY_COLUMNS_IN_SELECT:
      case SQL_PROCEDURE_TERM:
      case SQL_PROCEDURES:
      case SQL_ROW_UPDATES:
      case SQL_SCHEMA_TERM:
      case SQL_SEARCH_PATTERN_ESCAPE:
      case SQL_SERVER_NAME:
      case SQL_SPECIAL_CHARACTERS:
      case SQL_TABLE_TERM:
      case SQL_USER_NAME:
      case SQL_XOPEN_CLI_YEAR:
        if (is_odd_number (buffer_length))
          {
            return ODBC_ERROR;
          }

        break;

      default:
        break;
    }

  return ODBC_SUCCESS;
}

PUBLIC int decide_info_value_length (SQLUSMALLINT info_type, int buffer_length, int info_value_length)
{
  switch (info_type)
    {
      /* info_type that returns a string */
      case SQL_ACCESSIBLE_PROCEDURES:
      case SQL_ACCESSIBLE_TABLES:
      case SQL_CATALOG_NAME:
      case SQL_CATALOG_NAME_SEPARATOR:
      case SQL_CATALOG_TERM:
      case SQL_COLLATION_SEQ:
      case SQL_COLUMN_ALIAS:
      case SQL_DATA_SOURCE_NAME:
      case SQL_DATA_SOURCE_READ_ONLY:
      case SQL_DATABASE_NAME:
      case SQL_DBMS_NAME:
      case SQL_DBMS_VER:
      case SQL_DESCRIBE_PARAMETER:
      case SQL_DRIVER_NAME:
      case SQL_DRIVER_ODBC_VER:
      case SQL_DRIVER_VER:
      case SQL_EXPRESSIONS_IN_ORDERBY:
      case SQL_IDENTIFIER_QUOTE_CHAR:
      case SQL_INTEGRITY:
      case SQL_KEYWORDS:
      case SQL_LIKE_ESCAPE_CLAUSE:
      case SQL_MAX_ROW_SIZE_INCLUDES_LONG:
      case SQL_MULT_RESULT_SETS:
      case SQL_MULTIPLE_ACTIVE_TXN:
      case SQL_NEED_LONG_DATA_LEN:
      case SQL_ORDER_BY_COLUMNS_IN_SELECT:
      case SQL_PROCEDURE_TERM:
      case SQL_PROCEDURES:
      case SQL_ROW_UPDATES:
      case SQL_SCHEMA_TERM:
      case SQL_SEARCH_PATTERN_ESCAPE:
      case SQL_SERVER_NAME:
      case SQL_SPECIAL_CHARACTERS:
      case SQL_TABLE_TERM:
      case SQL_USER_NAME:
      case SQL_XOPEN_CLI_YEAR:
        if (info_value_length > buffer_length - sizeof (SQLWCHAR))
          {
            info_value_length = buffer_length - sizeof (SQLWCHAR);
          }

        break;

      default:
        break;
    }

  return info_value_length;
}
#endif
