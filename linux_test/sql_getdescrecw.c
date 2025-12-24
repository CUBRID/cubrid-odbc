#include <stdio.h>
#include <wchar.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "test_util.h"

/*
 * usage:
 *  2. run
 *       sql_getdescrecw dsn
 */

#define ID_LEN 255

int
sql_getdescrecw (int case_num, char *dsn)
{
  RETCODE retcode;
  SQLHENV           hEnv;
  SQLHDBC           hDbc;
  SQLHSTMT  hstmt;
  wchar_t *dsn_buf;
  SQLHDESC hIrd;

  SQLWCHAR *qry = "SELECT * from db_class";
  SQLCHAR *query_buf;

  SQLSMALLINT desc_count = -1;

  SQLWCHAR        Name[255];
  SQLSMALLINT     BufferLength = 255;
  SQLSMALLINT     StringLength;
  SQLSMALLINT     Type;
  SQLSMALLINT     SubType;
  SQLLEN          Length;
  SQLSMALLINT     Precision;
  SQLSMALLINT     Scale;
  SQLSMALLINT     Nullable;

  SQLCHAR *m_name;

  int id, i = 1;

  retcode = SQLAllocEnv (&hEnv);
  retcode = SQLSetEnvAttr (hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
  retcode = SQLAllocConnect (hEnv, &hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  bytes_to_wide_char (dsn, SQL_NTS, &dsn_buf, 0, NULL, "UCS2");
  retcode = SQLConnectW (hDbc, dsn_buf, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLAllocHandle (SQL_HANDLE_STMT, hDbc, &hstmt);

  retcode = bytes_to_wide_char (qry, strlen (qry), &query_buf, 0, NULL, "UCS2");
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLExecDirectW (hstmt, query_buf, SQL_NTS);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLGetStmtAttrW (hstmt, SQL_ATTR_IMP_ROW_DESC, &hIrd, 0, NULL);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLGetDescFieldW (hIrd, 0, SQL_DESC_COUNT, &desc_count, SQL_IS_POINTER, 0);
  AreNotEqual (retcode, SQL_ERROR);

  printf ("\tNum. desc fields (SQL_DESC_COUNT) = %d\n", desc_count);
  if (desc_count <= 0)
    {
      goto fin;
    }

  for (i = 1; i < desc_count; i++)
    {
      retcode = SQLGetDescRecW (hIrd, i + 1, Name, BufferLength,
				&StringLength, &Type,
				&SubType, &Length,
				&Precision, &Scale,
				&Nullable);
      if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO) )
	{
	  if (retcode==SQL_NO_DATA)
	    {
	      printf ("\nNo records !\n");
	    }
	  else
	    {
	      printf ("error\n");
	      break;
	    }
	}
      else
	{
	  wide_char_to_bytes (Name, SQL_NTS, &m_name, NULL, "UCS2");
	  printf ("\tName %s, ",      m_name);
	  printf ("\tType %i, ",        (int)Type);
	  printf ("\tSubType %i, ",     (int)SubType);
	  printf ("\tLength %i, ",      (int)Length);
	  printf ("\tPrecision %i, ",   (int)Precision);
	  printf ("\tScale %i, ",      (int)Scale);
	  printf ("\tNullable %i\n",    (int)Nullable);
	}
    }
fin:
  retcode = SQLDisconnect (hDbc);
  AreNotEqual (retcode, SQL_ERROR);

  retcode = SQLFreeHandle (SQL_HANDLE_DBC, hDbc);
  retcode = SQLFreeHandle (SQL_HANDLE_ENV, hEnv);
  AreNotEqual (retcode, SQL_ERROR);

  return retcode;
}
