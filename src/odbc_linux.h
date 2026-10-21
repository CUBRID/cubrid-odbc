#include  "ini.h"
#include  "odbcinstext.h"
typedef enum
{
  ODBCINI_DSN_NOT_FOUND = 0,
  ODBCINI_DSN_FOUND_USER,
  ODBCINI_DSN_FOUND_SYSTEM
} ODBCINI_DSN_LOOKUP_RESULT;

PUBLIC INT_PTR CALLBACK ConfigDSNDlgProc (HWND hwndParent, UINT message, WPARAM wParam, LPARAM lParam);
extern void dsn2connstr (CUBRIDDSNItem *dsn, char *connstr);
extern char *odbcinst_system_file_path (char *b1);
extern char *odbcinst_system_file_name (char *b2);
extern char *odbcinst_user_file_path (char *buffer);
extern char *odbcinst_user_file_name (char *buffer);
extern char *find_key (char *string, char *key);
extern BOOL _odbcinst_SystemINI (char *pszFileName, BOOL bVerify);
extern BOOL _odbcinst_UserINI (char *pszFileName, BOOL bVerify);
extern ODBCINI_DSN_LOOKUP_RESULT ini_fileopen (const char *dsn, HINI *hInip);

/* odbc_linux_unicode.c - the ANSI entry points the W wrappers in unicode.c call */
ODBC_INTERFACE RETCODE SQL_API SQLDriverConnectLinux (HDBC hdbc, HWND hWnd,
						      UCHAR *szConnStrIn, SWORD cbConnStrIn,
						      UCHAR *szConnStrOut, SWORD cbConnStrOut,
						      SQLSMALLINT *pcbConnStrOut, UWORD uwMode);
ODBC_INTERFACE RETCODE SQL_API SQLConnectLinux (SQLHDBC ConnectionHandle,
						SQLCHAR *DataSource, SQLSMALLINT NameLength1,
						SQLCHAR *UserName, SQLSMALLINT NameLength2,
						SQLCHAR *Authentication, SQLSMALLINT NameLength3);
ODBC_INTERFACE RETCODE SQL_API SQLExecDirectLinux (SQLHSTMT StatementHandle,
						   SQLCHAR *StatementText, SQLINTEGER TextLength);
ODBC_INTERFACE RETCODE SQL_API SQLDescribeColLinux (SQLHSTMT StatementHandle, SQLUSMALLINT ColumnNumber,
						    SQLCHAR *ColumnName, SQLSMALLINT BufferLength,
						    SQLSMALLINT *NameLength, SQLSMALLINT *DataType,
						    SQLULEN *ColumnSize, SQLSMALLINT *DecimalDigits,
						    SQLSMALLINT *Nullable);

#define DSN_LOOKUP(conn,ptDSN,hIni,key,dsn_item)						\
	do {											\
	     char *pDSN_item;									\
	     if ((pDSN_item = find_key (conn, key)) == NULL)					\
	       {										\
		 if (ptDSN != NULL							\
		     && iniPropertySeek (hIni, (char *) (ptDSN), key, "") == INI_SUCCESS)	\
		   {										\
		     snprintf (dsn_item, ITEMBUFLEN, "%.*s", ITEMBUFLEN - 1,			\
			       hIni->hCurProperty->szValue);					\
		   }										\
	       }										\
	     else										\
	       {										\
		 snprintf (dsn_item, ITEMBUFLEN, "%.*s", ITEMBUFLEN - 1, pDSN_item);		\
		 UT_FREE (pDSN_item);								\
	       }										\
	   } while (0)

