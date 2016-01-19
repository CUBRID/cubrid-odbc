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

#ifndef	__ODBC_STMT_HEADER	/* to avoid multiple inclusion */
#define	__ODBC_STMT_HEADER

#include		"odbc_portable.h"
#include		"sqlext.h"
#include		"odbc_descriptor.h"
#include		"odbc_util.h"
#include		"odbc_type.h"

#define   IND_IS_DATA_AT_EXEC(value)	( (value) == SQL_DATA_AT_EXEC || \
  (value) <= SQL_LEN_DATA_AT_EXEC_OFFSET )

#define		METHOD_STMT_TYPE(stmt_type)		\
  ( stmt_type == CUBRID_STMT_CALL || stmt_type == CUBRID_STMT_EVALUATE )
// Does stmt_type create result set?
#define		RESULTSET_STMT_TYPE(stmt_type)	\
  ( stmt_type == CUBRID_STMT_SELECT || METHOD_STMT_TYPE(stmt_type) || GET_STAT_STMT_TYPE(stmt_type))
#define		GET_STAT_STMT_TYPE(stmt_type)	\
  ( stmt_type == CUBRID_STMT_GET_STATS )

typedef enum
{
  NULL_RESULT = 0,
  QUERY,
  TABLES,
  STATISTICS,
  COLUMNS,
  SPECIALCOLUMNS,
  TYPE_INFO,
  PRIMARY_KEYS,
  FOREIGN_KEYS,
  TABLE_PRIVILEGES,
  PROCEDURES,
  PROCEDURE_COLUMNS
} RESULT_TYPE;

typedef long SQL_STMT_OPTION;	/* SQL_CLOSE, etc */


typedef struct tagPARAM_DATA
{
  D_STRING val;
  int index;
} PARAM_DATA;

typedef struct tagCATALOG_RESULT
{
  ST_LIST *value;
  void *current;		// catalog result set cursor
} CATALOG_RESULT;

/* COLUMN_DATA
*		large data�� ���ؼ� �����Ͽ� ������ �� �ֵ��� �ϴ� SQLGetData��
*		�����ϱ� ���ؼ� ��ȵǾ���.
*		column_no : column_no�� ������ sequential call�� �ν��Ѵ�.
*		current_pt : ���� data�� start pointer
*		remain_length :	���� data�� length, ������ remain_length�� 0�̶�� �ؼ�
*			SQL_NO_DATA�� return�� �� ����.  empty string�� ���ؼ� buffer length�� 0�̸�
*			���� ��Ȳ�� �߻��Ѵ�.  �̸� ���� ���ؼ� �����Ѱ� fetch_status�̴�.
*		fetch_status :  
*				������ function call���� SQL_NO_DATA�� �߻��� �� ������, SQL_SUCCESS_WITH_INFO
*				������ SQL_NO_DATA�� �ǳ� �� �� ����. �̰��� �̿��ؼ� ���� return status��
*				�����ؼ� buffer length�� 0�� ���ؼ� SQL_NO_DATA�� return�� ���� �����Ѵ�.
*					
*					->		SQL_SUCCESS_WITH_INFO
*
*					->		SQL_SUCCESS
*
*					X		SQL_NO_DATA
*/
typedef struct tagCOLUMN_DATA
{
  short column_no;
  char *current_pt;
  int remain_length;
  short prev_return_status;
} COLUMN_DATA;

typedef struct tagREVISED_SQL
{
  char *sql_text;		// revised sql text
  char *org_param_pos;		// original param pos in revised sql text
  char *oid_param_pos;		// oid param pos in revised sql text
  char *oid_param_val;		// oid param val
  short oid_param_num;
} REVISED_SQL;

typedef struct st_odbc_statement
{
  unsigned short handle_type;
  struct st_diag *diag;


  struct st_odbc_statement *next;
  struct st_odbc_connection *conn;
  int stmthd;			/* internal handle for CAS */
  CATALOG_RESULT catalog_result;
  struct tagCOLUMN_DATA column_data;

  RESULT_TYPE result_type;	/* if NULL_RESULT, no result set. */
  char *sql_text;		/* origianl SQL statement */
  struct tagREVISED_SQL revised_sql;
  char *cursor;			/* cursor name */
  short data_at_exec_state;	/*      > 0�� ���� ���� �������� bind parameter,
				   STMT_NEED_DATA ���� ������ ����,
				   STMT_NEED_NO_MORE_DATA ���� data�� ���ų�,
				   ��� ���� ���� */
  unsigned long tpl_number;	/* tpl_number - result set�� tuple ����, �Ǵ�
				 * update�� �ݿ��� row ��
				 * row_number - currunt row position */
  unsigned long current_tpl_pos;	/* result set���� cursor�� ��ġ,
					 * attr_row_number�� �ǹ̻� ����. 
					 * 0 Befor start, -1 After end */

  // param number�� column number�ʹ� �޸� ipd�� record������ �ƴϴ�.
  // �ֳĸ� ird�� �޸� auto ipd�� �ƴϱ� �����̴�.
  unsigned short param_number;
  T_CCI_CUBRID_STMT stmt_type;
  PARAM_DATA param_data;	/* For data at exec */
  char is_prepared;		// SQLPrepare�� ���ؼ��� prepare���¿� ���δ�.


  /* Supported attributes */
  unsigned long attr_metadata_id;	// Core
  unsigned long attr_cursor_scrollable;	// 1
  unsigned long attr_cursor_type;	// core, 2
  unsigned long attr_async_enable;	// 1, 2
  unsigned long attr_use_bookmark;	/* 2    
					 * bookmark�� ok�� ��� 
					 * prepare�� 0�� record�� �߰���
					 * �����Ѵ�. */


  /* Not supported attributes */
  unsigned long attr_noscan;	// Core
  unsigned long attr_concurrency;	// 1, 2
  unsigned long attr_cursor_sensitivity;	// 2
  unsigned long attr_enable_auto_ipd;	// 2
  void *attr_fetch_bookmark_ptr;	// 2
  unsigned long attr_keyset_size;	// 2
  unsigned long attr_max_length;	// 1
  unsigned long attr_max_rows;	// 1
  unsigned long attr_query_timeout;	// 2
  unsigned long attr_retrieve_data;	// 1
  unsigned short *attr_row_operation_ptr;	// 1
  unsigned long attr_simulate_cursor;	// 2



  // Descriptor section
  struct st_odbc_desc *i_apd;
  struct st_odbc_desc *i_ard;
  struct st_odbc_desc *i_ipd;
  struct st_odbc_desc *i_ird;

  struct st_odbc_desc *apd;
  struct st_odbc_desc *ard;
  struct st_odbc_desc *ipd;
  struct st_odbc_desc *ird;

} ODBC_STATEMENT;

PUBLIC RETCODE odbc_alloc_statement (struct st_odbc_connection *conn,
				     ODBC_STATEMENT ** stmt_ptr);
PUBLIC RETCODE odbc_free_statement (ODBC_STATEMENT * stmt);
PUBLIC RETCODE odbc_reset_statement (ODBC_STATEMENT * stmt,
				     unsigned short option);
PUBLIC RETCODE odbc_set_stmt_attr (ODBC_STATEMENT * stmt,
				   long attribute,
				   void *valueptr,
				   long stringlength, short is_driver);
PUBLIC RETCODE odbc_get_stmt_attr (ODBC_STATEMENT * stmt,
				   long attr,
				   void *value_ptr,
				   long buffer_length, long *length_ptr);
PUBLIC RETCODE odbc_set_cursor_name (ODBC_STATEMENT * stmt,
				     char *cursor_name, short name_length);
PUBLIC RETCODE odbc_get_cursor_name (ODBC_STATEMENT * stmt,
				     SQLCHAR * cursor_name,
				     SQLSMALLINT buffer_length,
				     SQLLEN * name_length_ptr);
PUBLIC RETCODE odbc_bind_parameter (ODBC_STATEMENT * stmt,
				    SQLUSMALLINT parameter_num,
				    SQLSMALLINT input_output_type,
				    SQLSMALLINT value_type,
				    SQLSMALLINT parameter_type,
				    SQLULEN column_size,
				    SQLSMALLINT decimal_digits,
				    SQLPOINTER parameter_value_ptr,
				    SQLLEN buffer_length,
				    SQLLEN * strlen_ind_ptr);
PUBLIC RETCODE odbc_num_params (ODBC_STATEMENT * stmt,
				short *parameter_count);
PUBLIC RETCODE odbc_prepare (ODBC_STATEMENT * stmt, char *statement_text);
PUBLIC RETCODE odbc_execute (ODBC_STATEMENT * stmt);
PUBLIC RETCODE odbc_param_data (ODBC_STATEMENT * stmt, void **valueptr_ptr);
PUBLIC RETCODE odbc_put_data (ODBC_STATEMENT * stmt,
			      void *data_ptr, SQLLEN strlen_or_ind);
PUBLIC RETCODE odbc_close_cursor (ODBC_STATEMENT * stmt);
PUBLIC RETCODE odbc_cancel (ODBC_STATEMENT * stmt);
PUBLIC RETCODE odbc_bulk_operations (ODBC_STATEMENT * stmt, short pperation);
PUBLIC RETCODE odbc_set_pos (ODBC_STATEMENT * stmt,
			     SQLSETPOSIROW row_number,
			     SQLUSMALLINT operation, SQLUSMALLINT lock_type);
PUBLIC void free_column_data (COLUMN_DATA * data, int option);
PUBLIC void reset_result_set (ODBC_STATEMENT * stmt);
PUBLIC void create_ird (ODBC_STATEMENT * stmt, T_CCI_COL_INFO * cci_col_info,
			int column_number);

#endif /* ! __ODBC_STMT_HEADER */
