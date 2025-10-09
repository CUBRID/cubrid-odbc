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
extern BOOL _odbcinst_SystemINI (char *pszFileName, BOOL bVerify);
extern BOOL _odbcinst_UserINI (char *pszFileName, BOOL bVerify);
extern ODBCINI_DSN_LOOKUP_RESULT ini_fileopen (const char *dsn, HINI *hInip);

#define DSN_LOOKUP(conn,ptDSN,hIni,key,dsn_item)						\
	do {											\
	     char *pDSN_item;									\
	     if ((pDSN_item = find_key (conn, key)) == NULL)					\
	       {										\
		 if (iniPropertySeek( hIni, ptDSN, key, "" ) == INI_SUCCESS)			\
		   {										\
		     snprintf (dsn_item, ITEMBUFLEN, "%s", hIni->hCurProperty->szValue);	\
		   }										\
	       }										\
	     else										\
	       {										\
		 snprintf (dsn_item, ITEMBUFLEN, "%s", pDSN_item);				\
		 UT_FREE (pDSN_item);								\
	       }										\
	   } while (0)

