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

#include		<windows.h>
#include		<stdio.h>

#include		"odbc_portable.h"
#include		"sqlext.h"
#include		"odbc_connection.h"
#include		"odbc_env.h"
#include		"odbc_statement.h"
#include		"odbc_result.h"
#include		"odbc_diag_record.h"
#include		"odbc_util.h"
#include		"odbc_catalog.h"
#include		"odbc_descriptor.h"
#include		"cas_cci.h"
#include		"odbc_resource.h"
//#include              "setup.h"

PUBLIC INT_PTR CALLBACK
ConfigDSNDlgProc (HWND hwndParent, UINT message, WPARAM wParam,
		  LPARAM lParam);

#define ODBC_RETURN(rc, handle)					\
  do {								\
    if ( handle != NULL )					\
	((ODBC_ENV*)handle)->diag->retcode = rc;		\
    return rc;							\
  } while (0 )

typedef struct __st_DriverConnectInfo
{
  /* Deprecated
     char db_name[VALUEBUFLEN];
   */
  char user[ITEMBUFLEN];
  char pwd[ITEMBUFLEN];
} DriverConnectInfo;

PRIVATE INT_PTR CALLBACK
ConnectDlgProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

PUBLIC HINSTANCE hInstance;

BOOL WINAPI
DllMain (HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

  if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
      cci_init ();
    }

  // hInstance is declared at resource_proc.c
  hInstance = (HINSTANCE) hModule;

  return TRUE;
}

const char *cci_client_name = "ODBC";

#if (ODBCVER >= 0x0300)

/************************************************************************
* name: SQLAllocHandle
* arguments:
* returns/side-effects:
* description:
* NOTE:
************************************************************************/

ODBC_INTERFACE RETCODE SQL_API
SQLAllocHandle (SQLSMALLINT HandleType,
		SQLHANDLE InputHandle, SQLHANDLE * OutputHandle)
{
  RETCODE rc = SQL_SUCCESS;
#ifdef _DEBUG
  ODBC_ENV *env = InputHandle;
#endif


  OutputDebugString ("SQLAllocHandle called\n");

  DEBUG_TIMESTAMP (START_SQLAllocHandle);


  if (InputHandle != NULL)
    {
      odbc_free_diag (((ODBC_ENV *) InputHandle)->diag, RESET);
    }

  if (OutputHandle == NULL)
    {
      odbc_set_diag (((ODBC_ENV *) InputHandle)->diag, "HY009", 0, NULL);
      return SQL_ERROR;
    }

  switch (HandleType)
    {
    case SQL_HANDLE_ENV:
      rc = odbc_alloc_env ((ODBC_ENV **) OutputHandle);
      break;
    case SQL_HANDLE_DBC:
      rc =
	odbc_alloc_connection ((ODBC_ENV *) InputHandle,
			       (ODBC_CONNECTION **) OutputHandle);
      break;
    case SQL_HANDLE_STMT:
      rc =
	odbc_alloc_statement ((ODBC_CONNECTION *) InputHandle,
			      (ODBC_STATEMENT **) OutputHandle);
      break;
    case SQL_HANDLE_DESC:
      rc =
	odbc_alloc_desc ((ODBC_CONNECTION *) InputHandle,
			 (ODBC_DESC **) OutputHandle);
      break;
    }

  DEBUG_TIMESTAMP (END_SQLAllocHandle);

  if (InputHandle != NULL)
    {
      ODBC_RETURN (rc, InputHandle);
    }
  else
    {
      return (rc);
    }

}
#endif


ODBC_INTERFACE RETCODE SQL_API
SQLBindCol (SQLHSTMT StatementHandle,
	    SQLUSMALLINT ColumnNumber,
	    SQLSMALLINT TargetType,
	    SQLPOINTER TargetValue,
	    SQLLEN BufferLength, SQLLEN * StrLen_or_Ind)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;



  OutputDebugString ("SQLBindCol called\n");

  DEBUG_TIMESTAMP (START_SQLBindCol);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_bind_col (stmt_handle, ColumnNumber, TargetType,
		      TargetValue, BufferLength, StrLen_or_Ind);

  DEBUG_TIMESTAMP (END_SQLAllocHandle);

  ODBC_RETURN (rc, StatementHandle);
}


ODBC_INTERFACE RETCODE SQL_API
SQLBindParameter (SQLHSTMT StatementHandle,
		  SQLUSMALLINT ParameterNumber,
		  SQLSMALLINT InputOutputType,
		  SQLSMALLINT ValueType,
		  SQLSMALLINT ParameterType,
		  SQLULEN ColumnSize,
		  SQLSMALLINT DecimalDigits,
		  SQLPOINTER ParameterValuePtr,
		  SQLLEN BufferLength, SQLLEN * StrLen_or_IndPtr)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLBindParameter called\n");

  DEBUG_TIMESTAMP (START_SQLBindParameter);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;

  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_bind_parameter (stmt_handle, ParameterNumber,
			    InputOutputType, ValueType,
			    ParameterType, ColumnSize,
			    DecimalDigits, ParameterValuePtr,
			    BufferLength, StrLen_or_IndPtr);

  DEBUG_TIMESTAMP (END_SQLBindParameter);

  ODBC_RETURN (rc, StatementHandle);
}


ODBC_INTERFACE RETCODE SQL_API
SQLCancel (SQLHSTMT StatementHandle)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLCancel called\n");

  DEBUG_TIMESTAMP (START_SQLCancel);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_cancel (stmt_handle);

  DEBUG_TIMESTAMP (END_SQLCancel);

  ODBC_RETURN (rc, StatementHandle);
}


#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLCloseCursor (SQLHSTMT StatementHandle)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLCloseCursor called\n");

  DEBUG_TIMESTAMP (START_SQLCloseCursor);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_close_cursor (stmt_handle);

  DEBUG_TIMESTAMP (END_SQLCloseCursor);

  ODBC_RETURN (rc, StatementHandle);
}



ODBC_INTERFACE RETCODE SQL_API
SQLColAttribute (SQLHSTMT StatementHandle,
		 SQLUSMALLINT ColumnNumber,
		 SQLUSMALLINT FieldIdentifier,
		 SQLPOINTER CharacterAttribute,
		 SQLSMALLINT BufferLength, SQLSMALLINT * StringLength,
#ifdef _WIN64
		 SQLLEN * NumericAttribute)
#else
		 SQLPOINTER NumericAttribute)
#endif
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLColAttribute called\n");

  DEBUG_TIMESTAMP (START_SQLColAttribute);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_col_attribute (stmt_handle, ColumnNumber,
			   FieldIdentifier, CharacterAttribute, BufferLength,
			   StringLength, NumericAttribute);

  DEBUG_TIMESTAMP (END_SQLColAttribute);

  ODBC_RETURN (rc, StatementHandle);
}
#endif


ODBC_INTERFACE RETCODE SQL_API
SQLColumns (SQLHSTMT StatementHandle,
	    SQLCHAR * CatalogName,
	    SQLSMALLINT NameLength1,
	    SQLCHAR * SchemaName,
	    SQLSMALLINT NameLength2,
	    SQLCHAR * TableName,
	    SQLSMALLINT NameLength3,
	    SQLCHAR * ColumnName, SQLSMALLINT NameLength4)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  SQLCHAR *stCatalogName = NULL;
  SQLCHAR *stSchemaName = NULL;
  SQLCHAR *stTableName = NULL;
  SQLCHAR *stColumnName = NULL;

  OutputDebugString ("SQLColumns called\n");

  DEBUG_TIMESTAMP (START_SQLColumns);

  stCatalogName = UT_MAKE_STRING (CatalogName, NameLength1);
  stSchemaName = UT_MAKE_STRING (SchemaName, NameLength2);
  stTableName = UT_MAKE_STRING (TableName, NameLength3);
  stColumnName = UT_MAKE_STRING (ColumnName, NameLength4);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc =
    odbc_columns (stmt_handle, stCatalogName, stSchemaName, stTableName,
		  stColumnName);

  NA_FREE (stCatalogName);
  NA_FREE (stSchemaName);
  NA_FREE (stTableName);
  NA_FREE (stColumnName);

  DEBUG_TIMESTAMP (END_SQLColumns);

  ODBC_RETURN (rc, StatementHandle);
}


ODBC_INTERFACE RETCODE SQL_API
SQLConnect (SQLHDBC ConnectionHandle,
	    SQLCHAR * DataSource,
	    SQLSMALLINT NameLength1,
	    SQLCHAR * UserName,
	    SQLSMALLINT NameLength2,
	    SQLCHAR * Authentication, SQLSMALLINT NameLength3)
{
  RETCODE rc = SQL_SUCCESS;
  SQLCHAR *stDataSource = NULL;
  SQLCHAR *stUserName = NULL;
  SQLCHAR *stAuthentication = NULL;
  SQLCHAR stDBName[ITEMBUFLEN];
  SQLCHAR stServerName[ITEMBUFLEN];
  SQLINTEGER Port, FetchSize;
  SQLCHAR stCharSet[ITEMBUFLEN];


  OutputDebugString ("SQLConnect called\n");

  DEBUG_TIMESTAMP (START_SQLConnect);

  stDataSource = UT_MAKE_STRING (DataSource, NameLength1);
  stUserName = UT_MAKE_STRING (UserName, NameLength2);
  stAuthentication = UT_MAKE_STRING (Authentication, NameLength3);

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  get_dsn_info (stDataSource, stDBName, sizeof (stDBName), NULL, 0, NULL, 0,
		stServerName, sizeof (stServerName), &Port, &FetchSize,
		stCharSet, sizeof (stCharSet));
  rc = odbc_connect_new ((ODBC_CONNECTION *) ConnectionHandle, stDataSource,
			 stDBName, stUserName, stAuthentication, stServerName,
			 Port, FetchSize, stCharSet);

  NA_FREE (stDataSource);
  NA_FREE (stUserName);
  NA_FREE (stAuthentication);

  DEBUG_TIMESTAMP (END_SQLConnect);

  ODBC_RETURN (rc, ConnectionHandle);
}

#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLCopyDesc (SQLHDESC SourceDescHandle, SQLHDESC TargetDescHandle)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLCopyDesc called\n");

  DEBUG_TIMESTAMP (START_SQLCopyDesc);

  odbc_free_diag (((ODBC_DESC *) TargetDescHandle)->diag, RESET);

  rc =
    odbc_copy_desc ((ODBC_DESC *) SourceDescHandle,
		    (ODBC_DESC *) TargetDescHandle);

  DEBUG_TIMESTAMP (END_SQLCopyDesc);

  ODBC_RETURN (rc, TargetDescHandle);
}
#endif




ODBC_INTERFACE RETCODE SQL_API
SQLDescribeCol (SQLHSTMT StatementHandle,
		SQLUSMALLINT ColumnNumber,
		SQLCHAR * ColumnName,
		SQLSMALLINT BufferLength,
		SQLSMALLINT * NameLength,
		SQLSMALLINT * DataType,
		SQLULEN * ColumnSize,
		SQLSMALLINT * DecimalDigits, SQLSMALLINT * Nullable)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLDescribeCol called\n");

  DEBUG_TIMESTAMP (START_SQLDescribeCol);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_describe_col (stmt_handle, ColumnNumber,
			  ColumnName, BufferLength, NameLength, DataType,
			  ColumnSize, DecimalDigits, Nullable);

  DEBUG_TIMESTAMP (END_SQLDescribeCol);

  ODBC_RETURN (rc, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLDisconnect (SQLHDBC ConnectionHandle)
{
  /* always return SQL_SUCCESS */
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLDisconnect called\n");

  DEBUG_TIMESTAMP (START_SQLDisconnect);

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  odbc_disconnect ((ODBC_CONNECTION *) ConnectionHandle);

  DEBUG_TIMESTAMP (END_SQLDisconnect);

  ODBC_RETURN (rc, ConnectionHandle);

}

/*
 *	SQLDriverConnect
 *		- PWD entry�� file DSN�� �߰����� �ʴ´�.
 */
ODBC_INTERFACE RETCODE SQL_API
SQLDriverConnect (HDBC hdbc,
		  HWND hWnd,
		  UCHAR * szConnStrIn,
		  SWORD cbConnStrIn,
		  UCHAR * szConnStrOut,
		  SWORD cbConnStrOut,
		  SQLSMALLINT * pcbConnStrOut, UWORD uwMode)
{
  RETCODE rc = ODBC_SUCCESS;

  INT_PTR dlgrc;
  char *pt;

  DriverConnectInfo dci;

  CUBRIDDSNItem dsn_item;

  const char *ptDriver;
  const char *ptDSN;
  const char *ptDBName;
  const char *ptUser;
  const char *ptPWD;
  const char *ptServer;
  const char *ptPort;
  const char *ptFileDSN;
  const char *ptSaveFile;
  const char *ptFetchSize;
  const char *ptDescription;
  const char *ptCharSet;

  // Deprecated : char            szDriver[ITEMBUFLEN];
  char ConnStrIn[4096];
  char buf[4096];
  char buf2[4096];
  char user[4096] = "";
  char pwd[ITEMBUFLEN] = "";
  char db_name[ITEMBUFLEN] = "";
  char server[ITEMBUFLEN] = "";
  char charset[ITEMBUFLEN] = "";
  int port = 0;
  int fetch_size = 0;

  OutputDebugString ("SQLDriverConnect called\n");

  DEBUG_TIMESTAMP (START_SQLDriverConnect);

  if ((cbConnStrIn == SQL_NTS) && (szConnStrIn))
    cbConnStrIn = strlen (szConnStrIn);

  strncpy (ConnStrIn, szConnStrIn, cbConnStrIn);
  ConnStrIn[cbConnStrIn] = '\0';	// for end of list, if cbConnStrIn isn't end
  // with ';'
  for (pt = ConnStrIn; *pt != '\0'; ++pt)
    {
      if (*pt == ';')		// connection string delimiter
	*pt = '\0';
    }
  ptDriver = element_value_by_key (ConnStrIn, KEYWORD_DRIVER);
  /*
     pt = strchr(ptDriver, '{');
     if ( pt == NULL ) {
     strcpy(szDriver, ptDriver);
     } else {
     pt2 = strchr(ptDriver, '}');
     // if ( pt2 == NULL ) error;
     strncpy(szDriver, pt+1, pt2 - pt -1);
     szDriver[pt2-pt -1] = '\0';
     }
   */
  ptDSN = element_value_by_key (ConnStrIn, KEYWORD_DSN);
  ptUser = element_value_by_key (ConnStrIn, KEYWORD_USER);
  ptPWD = element_value_by_key (ConnStrIn, KEYWORD_PASSWORD);
  ptSaveFile = element_value_by_key (ConnStrIn, KEYWORD_SAVEFILE);
  ptCharSet = element_value_by_key (ConnStrIn, KEYWORD_CHARSET);

  if (ptSaveFile == NULL)
    {				// for just connect

      // extracting user & pwd by dialog box
      if (ptUser == NULL || ptUser[0] == '\0' || ptPWD == NULL)
	{
	  if (ptDSN != NULL)
	    {
	      get_dsn_info (ptDSN, NULL, 0, user, sizeof (user),
			    pwd, sizeof (pwd), NULL, 0, NULL, NULL, NULL, 0);
	    }
	  if (ptUser == NULL || ptUser[0] == '\0')
	    {
	      sprintf (dci.user, "%s", user);
	      sprintf (dci.pwd, "%s", pwd);
	    }
	  else
	    {
	      sprintf (dci.user, "%s", ptUser);
	      sprintf (dci.pwd, "");
	    }

	  if (strcmp (dci.user, "") == 0)
	    {
	      DialogBoxParam (hInstance, (LPCTSTR) IDD_DRIVERCONNECT, hWnd,
			      ConnectDlgProc, (LPARAM) & dci);
	    }
	  ptUser = dci.user;
	  ptPWD = dci.pwd;
	}

      if (ptDSN == NULL)
	{
	  ptFileDSN = element_value_by_key (ConnStrIn, KEYWORD_FILEDSN);
	  ptDBName = element_value_by_key (ConnStrIn, KEYWORD_DBNAME);
	  ptServer = element_value_by_key (ConnStrIn, KEYWORD_SERVER);
	  ptPort = element_value_by_key (ConnStrIn, KEYWORD_PORT);
	  ptFetchSize = element_value_by_key (ConnStrIn, KEYWORD_FETCH_SIZE);
	  ptCharSet = element_value_by_key (ConnStrIn, KEYWORD_CHARSET);

	  port = ptPort ? atoi (ptPort) : 0;
	  fetch_size = ptFetchSize ? atoi (ptFetchSize) : 0;

	  rc = odbc_connect_new (hdbc, ptFileDSN, ptDBName, ptUser,
				 ptPWD, ptServer, port, fetch_size, ptCharSet);
	}
      else
	{
	  // with DSN
	  ptDBName = element_value_by_key (ConnStrIn, KEYWORD_DBNAME);
	  ptServer = element_value_by_key (ConnStrIn, KEYWORD_SERVER);
	  ptPort = element_value_by_key (ConnStrIn, KEYWORD_PORT);
	  ptFetchSize = element_value_by_key (ConnStrIn, KEYWORD_FETCH_SIZE);
	  ptCharSet = element_value_by_key (ConnStrIn, KEYWORD_CHARSET);

	  get_dsn_info (ptDSN, db_name, sizeof (db_name), NULL, 0,
			NULL, 0, server, sizeof (server), &port, &fetch_size,
			charset, sizeof(charset));

	  // cbConnStrIn�� DSN�� �������� �켱�ϴ�.
	  if (ptDBName == NULL)
	    {
	      ptDBName = db_name;
	    }
	  if (ptServer == NULL)
	    {
	      ptServer = server;
	    }
	  if (ptPort != NULL)
	    {
	      port = atoi (ptPort);
	    }
	  if (ptFetchSize != NULL)
	    {
	      fetch_size = atoi (ptFetchSize);
	    }
	  if (ptCharSet == NULL)
	    {
	      ptCharSet = charset;
	    }

	  rc = odbc_connect_new (hdbc, ptDSN, ptDBName, ptUser,
				 ptPWD, ptServer, port, fetch_size, ptCharSet);
	}

      // Building ConnStrOut

      if (ptDSN != NULL)
	{
	  sprintf (buf, "%s=%s;", KEYWORD_DSN, ptDSN);
	}
      if (ptDriver != NULL)
	{
	  sprintf (buf, "%s=%s;", KEYWORD_DRIVER, ptDriver);
	}
      else
	{
	  buf2[0] = '\0';
	}

      if (ptDBName != NULL)
	{
	  sprintf (buf2, "%s=%s;", KEYWORD_DBNAME, ptDBName);
	  strcat (buf, buf2);
	}
      if (ptUser != NULL)
	{
	  sprintf (buf2, "%s=%s;", KEYWORD_USER, ptUser);
	  strcat (buf, buf2);
	}
      if (ptPWD != NULL)
	{
	  sprintf (buf2, "%s=%s;", KEYWORD_PASSWORD, ptPWD);
	  strcat (buf, buf2);
	}
      if (ptServer != NULL)
	{
	  sprintf (buf2, "%s=%s;", KEYWORD_SERVER, ptServer);
	  strcat (buf, buf2);
	}
      if (port > 0)
	{
	  sprintf (buf2, "%s=%d;", KEYWORD_PORT, port);
	  strcat (buf, buf2);
	}
      if (fetch_size > 0)
	{
	  sprintf (buf2, "%s=%d;", KEYWORD_FETCH_SIZE, fetch_size);
	  strcat (buf, buf2);
	}
      if (ptCharSet != NULL)
	{
	  sprintf (buf2, "%s=%s;", KEYWORD_CHARSET, ptCharSet);
	  strcat (buf, buf2);
	}

      if ((szConnStrOut) && cbConnStrOut > 0)
	{
	  strncpy (szConnStrOut, buf,
		   MIN (strlen (buf), (unsigned) cbConnStrOut));
	  szConnStrOut[MIN (strlen (buf), (unsigned) cbConnStrOut)] = '\0';
	}

      if (pcbConnStrOut)
	*pcbConnStrOut = MIN (strlen (buf), (unsigned) cbConnStrOut);
    }
  else
    {
      // for creating & managing FILEDSN
      // SAVEFILE�� �ִ� ���, DSN�� ã�� �� ����.
      ptDBName = element_value_by_key (buf, KEYWORD_DBNAME);
      ptUser = element_value_by_key (buf, KEYWORD_USER);
      ptPWD = element_value_by_key (buf, KEYWORD_PASSWORD);
      ptServer = element_value_by_key (buf, KEYWORD_SERVER);
      ptPort = element_value_by_key (buf, KEYWORD_PORT);
      ptFetchSize = element_value_by_key (buf, KEYWORD_FETCH_SIZE);
      ptDescription = element_value_by_key (buf, KEYWORD_DESCRIPTION);
      ptCharSet = element_value_by_key (buf, KEYWORD_CHARSET);


      memset (&dsn_item, 0, sizeof (CUBRIDDSNItem));

      strcpy (dsn_item.save_file, ptSaveFile);
      if (ptDBName != NULL)
	strcpy (dsn_item.db_name, ptDBName);
      if (ptUser != NULL)
	strcpy (dsn_item.user, ptUser);
      if (ptPWD != NULL)
	strcpy (dsn_item.password, ptPWD);
      if (ptServer != NULL)
	strcpy (dsn_item.server, ptServer);
      if (ptPort != NULL)
	strcpy (dsn_item.port, ptPort);
      if (ptFetchSize != NULL)
	strcpy (dsn_item.fetch_size, ptFetchSize);
      if (ptDescription != NULL)
	strcpy (dsn_item.description, ptDescription);
      if (ptCharSet != NULL)
	strcpy (dsn_item.charset, ptCharSet);

      dlgrc = DialogBoxParam (hInstance, (LPCTSTR) IDD_CONFIGDSN, hWnd,
			      ConfigDSNDlgProc, (LPARAM) & dsn_item);

      sprintf (buf, "%s=%s;%s=%s;%s=%s;%s=%s;%s=%s;%s=%s;%s=%s;%s=%s;%s=%s;%s=%s",
	       KEYWORD_DRIVER, ptDriver,
	       KEYWORD_SAVEFILE, ptSaveFile,
	       KEYWORD_DESCRIPTION, dsn_item.description,
	       KEYWORD_DBNAME, dsn_item.db_name,
	       KEYWORD_PASSWORD, dsn_item.password,
	       KEYWORD_USER, dsn_item.user,
	       KEYWORD_SERVER, dsn_item.server,
	       KEYWORD_PORT, dsn_item.port,
	       KEYWORD_FETCH_SIZE, dsn_item.fetch_size,
	       KEYWORD_CHARSET, dsn_item.charset);

      if ((szConnStrOut) && cbConnStrOut > 0)
	{
	  strncpy (szConnStrOut, buf,
		   MIN (strlen (buf), (unsigned) cbConnStrOut));
	  szConnStrOut[MIN (strlen (buf), (unsigned) cbConnStrOut)] = '\0';
	}

      if (pcbConnStrOut)
	*pcbConnStrOut = MIN (strlen (buf), (unsigned) cbConnStrOut);

      // CHECK : return value
    }

  DEBUG_TIMESTAMP (END_SQLDriverConnect);

  ODBC_RETURN (rc, hdbc);
}

#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLEndTran (SQLSMALLINT HandleType,
	    SQLHANDLE Handle, SQLSMALLINT CompletionType)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLEndTran called\n");

  DEBUG_TIMESTAMP (START_SQLEndTran);

  odbc_free_diag (((ODBC_ENV *) Handle)->diag, RESET);

  rc = odbc_end_tran (HandleType, Handle, CompletionType);

  DEBUG_TIMESTAMP (END_SQLEndTran);

  ODBC_RETURN (rc, Handle);
}
#endif


// ���� SQLExecDirect�� prepare�� ���¸� Ǯ �� �ִ�.
ODBC_INTERFACE RETCODE SQL_API
SQLExecDirect (SQLHSTMT StatementHandle,
	       SQLCHAR * StatementText, SQLINTEGER TextLength)
{
  RETCODE rc = SQL_SUCCESS;
  SQLCHAR *stStatementText = NULL;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLExecDirect called\n");

  DEBUG_TIMESTAMP (START_SQLExecDirect);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  stStatementText = UT_MAKE_STRING (StatementText, TextLength);

  stmt_handle->is_prepared = _FALSE_;

  rc = odbc_prepare (stmt_handle, stStatementText);
  ERROR_GOTO (rc, error);

  rc = odbc_execute (stmt_handle);
  ERROR_GOTO (rc, error);

  DEBUG_TIMESTAMP (END_SQLExecDirect);

error:
  NA_FREE (stStatementText);
  ODBC_RETURN (rc, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLExecute (SQLHSTMT StatementHandle)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;
  char *buf = NULL;

  OutputDebugString ("SQLExecute called\n");

  DEBUG_TIMESTAMP (START_SQLExecute);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  // SQLEndTran()�� ���ؼ� cursor�� delete�� ����
  if (stmt_handle->is_prepared == _TRUE_ && stmt_handle->stmthd <= 0)
    {
      buf = UT_MAKE_STRING (stmt_handle->sql_text, -1);
      rc = odbc_prepare (stmt_handle, buf);
      NA_FREE (buf);
    }

  rc = odbc_execute (stmt_handle);

  DEBUG_TIMESTAMP (END_SQLExecute);

  ODBC_RETURN (rc, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLFetch (SQLHSTMT StatementHandle)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;

  OutputDebugString ("SQLFetch called\n");

  DEBUG_TIMESTAMP (START_SQLFetch);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_fetch (stmt_handle, SQL_FETCH_NEXT, 1, 0, 0);	// fetch_offset is ignored

  DEBUG_TIMESTAMP (END_SQLFetch);

  ODBC_RETURN (rc, StatementHandle);
}


#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLFetchScroll (SQLHSTMT StatementHandle,
		SQLSMALLINT FetchOrientation, SQLLEN FetchOffset)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLFetchScroll called\n");

  DEBUG_TIMESTAMP (START_SQLFetchScroll);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_fetch (stmt_handle, FetchOrientation, FetchOffset, 0, 0);

  DEBUG_TIMESTAMP (END_SQLFetchScroll);

  ODBC_RETURN (rc, StatementHandle);
}
#endif

/************************************************************************
 * name: SQLExtendedFetch
 * arguments:
 * returns/side-effects:
 * description:
 * NOTE:
 *	SQLExtenedtFetch�� SQLFetchScroll�� mapping�ϰ� �ִ�.
 *	�� �� odbc 2.x�� SQL_ATTR_ROWS_FETCHED_PTR�� SQL_ATTR_ROW_STATUS_PTR
 *	�� RowCountPtr, RowStatusArray�� ����Ͽ� ����ϹǷ� odbc_fetch()����
 *	�۵��ϱ� ���ؼ� odbc_set_stmt_attr()�� ����Ͽ� setting�� �ش�.
 *	���� SQL_ROWSET_SIZE�� SQL_ATTR_ROW_ARRAY_SIZE�� mapping�Ǿ� �ִ�.
 *
 ************************************************************************/
// for 2.x backward compatibility
ODBC_INTERFACE RETCODE SQL_API
SQLExtendedFetch (SQLHSTMT StatementHandle,
		  SQLUSMALLINT FetchOrientation,
		  SQLLEN FetchOffset,
		  SQLULEN * RowCountPtr, SQLUSMALLINT * RowStatusArray)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLExtendedFetch called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  odbc_set_stmt_attr (stmt_handle, SQL_ATTR_ROWS_FETCHED_PTR,
		      (void *) RowCountPtr, 0, 1);
  odbc_set_stmt_attr (stmt_handle, SQL_ATTR_ROW_STATUS_PTR,
		      (void *) RowStatusArray, 0, 1);

  rc = odbc_fetch (stmt_handle, FetchOrientation, FetchOffset, 0, 0);


  ODBC_RETURN (rc, StatementHandle);

  return (rc);
}

#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLFreeHandle (SQLSMALLINT HandleType, SQLHANDLE Handle)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLFreeHandle called\n");

  DEBUG_TIMESTAMP (START_SQLFreeHandle);

  switch (HandleType)
    {
    case SQL_HANDLE_ENV:
      rc = odbc_free_env ((ODBC_ENV *) Handle);
      break;
    case SQL_HANDLE_DBC:
      rc = odbc_free_connection ((ODBC_CONNECTION *) Handle);
      break;
    case SQL_HANDLE_STMT:
      rc = odbc_free_statement ((ODBC_STATEMENT *) Handle);
      break;
    case SQL_HANDLE_DESC:
      rc = odbc_free_desc ((ODBC_DESC *) Handle);
      break;
    }

  DEBUG_TIMESTAMP (END_SQLFreeHandle);

  return (rc);
}
#endif


ODBC_INTERFACE RETCODE SQL_API
SQLFreeStmt (SQLHSTMT StatementHandle, SQLUSMALLINT Option)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLFreeStmt called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_reset_statement (stmt_handle, Option);

  ODBC_RETURN (rc, StatementHandle);
}


#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLGetConnectAttr (SQLHDBC ConnectionHandle,
		   SQLINTEGER Attribute,
		   SQLPOINTER Value,
		   SQLINTEGER BufferLength, SQLINTEGER * StringLength)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLGetConnectAttr called\n");

  DEBUG_TIMESTAMP (START_SQLGetConnectAttr);

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  rc = odbc_get_connect_attr ((ODBC_CONNECTION *) ConnectionHandle,
			      Attribute, Value, BufferLength, StringLength);

  DEBUG_TIMESTAMP (END_SQLGetConnectAttr);

  ODBC_RETURN (rc, ConnectionHandle);
}
#endif


ODBC_INTERFACE RETCODE SQL_API
SQLGetCursorName (SQLHSTMT StatementHandle,
		  SQLCHAR * CursorName,
		  SQLSMALLINT BufferLength, SQLSMALLINT * NameLength)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;
  SQLLEN tmp_NameLength;

  OutputDebugString ("SQLGetCursorName called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc =
    odbc_get_cursor_name (stmt_handle, CursorName, BufferLength,
			  &tmp_NameLength);

  if (NameLength != NULL)
    {
      *NameLength = (short) tmp_NameLength;
    }

  return (rc);
}


ODBC_INTERFACE RETCODE SQL_API
SQLGetData (SQLHSTMT StatementHandle,
	    SQLUSMALLINT ColumnNumber,
	    SQLSMALLINT TargetType,
	    SQLPOINTER TargetValue,
	    SQLLEN BufferLength, SQLLEN * StrLen_or_Ind)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLGetData called\n");

  DEBUG_TIMESTAMP (START_SQLGetData);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_get_data (stmt_handle, ColumnNumber,
		      TargetType, TargetValue, BufferLength, StrLen_or_Ind);

  DEBUG_TIMESTAMP (END_SQLGetData);

  ODBC_RETURN (rc, StatementHandle);

}


#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLGetDescField (SQLHDESC DescriptorHandle,
		 SQLSMALLINT RecNumber,
		 SQLSMALLINT FieldIdentifier,
		 SQLPOINTER Value,
		 SQLINTEGER BufferLength, SQLINTEGER * StringLength)
{
  RETCODE rc = SQL_SUCCESS;
  SQLLEN tmp_StringLength;

  OutputDebugString ("SQLGetDescField called\n");

  DEBUG_TIMESTAMP (START_SQLGetDescField);

  odbc_free_diag (((ODBC_DESC *) DescriptorHandle)->diag, RESET);

  rc = odbc_get_desc_field ((ODBC_DESC *) DescriptorHandle, RecNumber,
			    FieldIdentifier, Value,
			    BufferLength, &tmp_StringLength);
  *StringLength = (SQLINTEGER) tmp_StringLength;

  DEBUG_TIMESTAMP (END_SQLGetDescField);

  ODBC_RETURN (rc, DescriptorHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLGetDescRec (SQLHDESC DescriptorHandle,
	       SQLSMALLINT RecNumber,
	       SQLCHAR * Name,
	       SQLSMALLINT BufferLength,
	       SQLSMALLINT * StringLength,
	       SQLSMALLINT * Type,
	       SQLSMALLINT * SubType,
	       SQLLEN * Length,
	       SQLSMALLINT * Precision,
	       SQLSMALLINT * Scale, SQLSMALLINT * Nullable)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLGetDescRec called\n");

  DEBUG_TIMESTAMP (START_SQLGetDescRec);

  odbc_free_diag (((ODBC_DESC *) DescriptorHandle)->diag, RESET);

  rc = odbc_get_desc_rec ((ODBC_DESC *) DescriptorHandle, RecNumber,
			  Name, BufferLength, StringLength,
			  Type, SubType, Length, Precision, Scale, Nullable);

  DEBUG_TIMESTAMP (END_SQLGetDescRec);

  ODBC_RETURN (rc, DescriptorHandle);
}


ODBC_INTERFACE RETCODE SQL_API
SQLGetDiagField (SQLSMALLINT HandleType,
		 SQLHANDLE Handle,
		 SQLSMALLINT RecNumber,
		 SQLSMALLINT DiagIdentifier,
		 SQLPOINTER DiagInfo,
		 SQLSMALLINT BufferLength, SQLSMALLINT * StringLength)
{
  RETCODE rc = SQL_SUCCESS;
  SQLLEN tmp_StringLength;	// for type compatibility between short*, int*

  OutputDebugString ("SQLGetDiagField called\n");

  DEBUG_TIMESTAMP (START_SQLGetDiagField);

  rc = odbc_get_diag_field (HandleType, Handle, RecNumber, DiagIdentifier,
			    DiagInfo, BufferLength, &tmp_StringLength);
  if (StringLength != NULL)
    {
      *StringLength = (short) tmp_StringLength;
    }

  DEBUG_TIMESTAMP (END_SQLGetDiagField);

  // SQLGetDiagRec SQLGetDiagField�� ���ؼ��� ODBC_RETURN()�� ����� �� ����.
  return rc;
}

ODBC_INTERFACE RETCODE SQL_API
SQLGetDiagRec (SQLSMALLINT HandleType,
	       SQLHANDLE Handle,
	       SQLSMALLINT RecNumber,
	       SQLCHAR * Sqlstate,
	       SQLINTEGER * NativeError,
	       SQLCHAR * MessageText,
	       SQLSMALLINT BufferLength, SQLSMALLINT * TextLength)
{
  RETCODE rc = SQL_SUCCESS;
  SQLLEN tmp_StringLength;

  OutputDebugString ("SQLGetDiagRec called\n");

  DEBUG_TIMESTAMP (START_SQLGetDiagRec);

  rc = odbc_get_diag_rec (HandleType, Handle, RecNumber, Sqlstate,
			  NativeError, MessageText, BufferLength,
			  &tmp_StringLength);
  if (rc != ODBC_SUCCESS)
    return rc;

  if (TextLength != NULL)
    {
      *TextLength = (short) tmp_StringLength;
    }

  DEBUG_TIMESTAMP (END_SQLGetDiagRec);

  // SQLGetDiagRec SQLGetDiagField�� ���ؼ��� ODBC_RETURN()�� ����� �� ����.
  return rc;
}

ODBC_INTERFACE RETCODE SQL_API
SQLGetEnvAttr (SQLHENV EnvironmentHandle,
	       SQLINTEGER Attribute,
	       SQLPOINTER Value,
	       SQLINTEGER BufferLength, SQLINTEGER * StringLength)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_ENV *env;

  OutputDebugString ("SQLGetEnvAttr called\n");

  DEBUG_TIMESTAMP (START_SQLGetEnvAttr);

  env = (ODBC_ENV *) EnvironmentHandle;

  if (env != NULL)
    {
      odbc_free_diag (env->diag, RESET);
    }

  if (env == NULL || env->handle_type != SQL_HANDLE_ENV)
    return SQL_INVALID_HANDLE;

  rc = odbc_get_env_attr (env, Attribute, Value, BufferLength, StringLength);

  DEBUG_TIMESTAMP (END_SQLGetEnvAttr);

  ODBC_RETURN (rc, env);
}
#endif /* ODBCVER >= 0x0300 */


ODBC_INTERFACE RETCODE SQL_API
SQLGetFunctions (SQLHDBC ConnectionHandle,
		 SQLUSMALLINT FunctionId, SQLUSMALLINT * Supported)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLGetFunctions called\n");

  DEBUG_TIMESTAMP (START_SQLGetFunctions);

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  rc = odbc_get_functions (ConnectionHandle, FunctionId, Supported);

  DEBUG_TIMESTAMP (END_SQLGetFunctions);

  ODBC_RETURN (rc, ConnectionHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLGetInfo (SQLHDBC ConnectionHandle,
	    SQLUSMALLINT InfoType,
	    SQLPOINTER InfoValue,
	    SQLSMALLINT BufferLength, SQLSMALLINT * StringLength)
{
  RETCODE rc = SQL_SUCCESS;
  SQLLEN tmp_StringLength;

  OutputDebugString ("SQLGetInfo called\n");

  DEBUG_TIMESTAMP (START_SQLGetInfo);

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  rc =
    odbc_get_info ((ODBC_CONNECTION *) ConnectionHandle, InfoType, InfoValue,
		   BufferLength, &tmp_StringLength);
  if (StringLength != NULL)
    {
      *StringLength = (short) tmp_StringLength;
    }

  DEBUG_TIMESTAMP (END_SQLGetInfo);

  ODBC_RETURN (rc, ConnectionHandle);
}

#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLGetStmtAttr (SQLHSTMT StatementHandle,
		SQLINTEGER Attribute,
		SQLPOINTER Value,
		SQLINTEGER BufferLength, SQLINTEGER * StringLength)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLGetStmtAttr called\n");

  DEBUG_TIMESTAMP (START_SQLGetStmtAttr);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_get_stmt_attr (stmt_handle, Attribute, Value,
			   BufferLength, StringLength);

  DEBUG_TIMESTAMP (END_SQLGetStmtAttr);

  ODBC_RETURN (rc, StatementHandle);
}
#endif /* ODBCVER >= 0x0300 */

ODBC_INTERFACE RETCODE SQL_API
SQLGetTypeInfo (SQLHSTMT StatementHandle, SQLSMALLINT DataType)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLGetTypeInfo called\n");

  DEBUG_TIMESTAMP (START_SQLGetTypeInfo);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_get_type_info (stmt_handle, DataType);

  DEBUG_TIMESTAMP (END_SQLGetTypeInfo);

  ODBC_RETURN (rc, StatementHandle);

}

ODBC_INTERFACE RETCODE SQL_API
SQLNativeSql (SQLHDBC ConnectionHandle,
	      SQLCHAR * InStatementText,
	      SQLINTEGER TextLength1,
	      SQLCHAR * OutStatementText,
	      SQLINTEGER BufferLength, SQLINTEGER * TextLength2Ptr)
{
  RETCODE rc = SQL_SUCCESS;
  SQLCHAR *stInStatementText = NULL;

  OutputDebugString ("SQLNativeSql called\n");

  DEBUG_TIMESTAMP (START_SQLNativeSql);

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  stInStatementText = UT_MAKE_STRING (InStatementText, TextLength1);


  rc =
    odbc_native_sql ((ODBC_CONNECTION *) ConnectionHandle, stInStatementText,
		     OutStatementText, BufferLength, TextLength2Ptr);

  DEBUG_TIMESTAMP (END_SQLNativeSql);

  NC_FREE (stInStatementText);

  return (rc);
}


ODBC_INTERFACE RETCODE SQL_API
SQLNumParams (SQLHSTMT StatementHandle, SQLSMALLINT * ParameterCountPtr)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLNumParams called\n");

  DEBUG_TIMESTAMP (START_SQLNumParams);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_num_params (stmt_handle, ParameterCountPtr);

  DEBUG_TIMESTAMP (END_SQLNumParams);

  ODBC_RETURN (rc, StatementHandle);
}



ODBC_INTERFACE RETCODE SQL_API
SQLNumResultCols (SQLHSTMT StatementHandle, SQLSMALLINT * ColumnCount)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLNumResultCols called\n");

  DEBUG_TIMESTAMP (START_SQLNumResultCols);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_num_result_cols (stmt_handle, ColumnCount);

  DEBUG_TIMESTAMP (END_SQLNumResultCols);

  ODBC_RETURN (rc, StatementHandle);
}


ODBC_INTERFACE RETCODE SQL_API
SQLParamData (SQLHSTMT StatementHandle, SQLPOINTER * Value)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLParamData called\n");

  DEBUG_TIMESTAMP (START_SQLParamData);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_param_data (stmt_handle, Value);

  DEBUG_TIMESTAMP (END_SQLParamData);

  ODBC_RETURN (rc, StatementHandle);
}

// ���� SQLPrepare�� prepared�� ���·� ����� �ִ�.
ODBC_INTERFACE RETCODE SQL_API
SQLPrepare (SQLHSTMT StatementHandle,
	    SQLCHAR * StatementText, SQLINTEGER TextLength)
{
  RETCODE rc = SQL_SUCCESS;
  SQLCHAR *stStatementText = NULL;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLPrepare called\n");

  DEBUG_TIMESTAMP (START_SQLPrepare);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  stStatementText = UT_MAKE_STRING (StatementText, TextLength);

  rc = odbc_prepare (stmt_handle, stStatementText);
  stmt_handle->is_prepared = _TRUE_;

  NA_FREE (stStatementText);

  DEBUG_TIMESTAMP (END_SQLPrepare);

  ODBC_RETURN (rc, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLPutData (SQLHSTMT StatementHandle, SQLPOINTER Data, SQLLEN StrLen_or_Ind)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLPutData called\n");

  DEBUG_TIMESTAMP (START_SQLPutData);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_put_data (stmt_handle, Data, StrLen_or_Ind);

  DEBUG_TIMESTAMP (END_SQLPutData);

  ODBC_RETURN (rc, StatementHandle);
}


ODBC_INTERFACE RETCODE SQL_API
SQLRowCount (SQLHSTMT StatementHandle, SQLLEN * RowCount)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLRowCount called\n");

  DEBUG_TIMESTAMP (START_SQLRowCount);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_row_count (stmt_handle, RowCount);

  DEBUG_TIMESTAMP (END_SQLRowCount);

  ODBC_RETURN (rc, StatementHandle);
}

#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLSetConnectAttr (SQLHDBC ConnectionHandle,
		   SQLINTEGER Attribute,
		   SQLPOINTER Value, SQLINTEGER StringLength)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLSetConnectAttr called\n");

  DEBUG_TIMESTAMP (START_SQLSetConnectAttr);

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  rc = odbc_set_connect_attr ((ODBC_CONNECTION *) ConnectionHandle, Attribute,
			      Value, StringLength);

  DEBUG_TIMESTAMP (END_SQLSetConnectAttr);

  ODBC_RETURN (rc, ConnectionHandle);
}
#endif /* ODBCVER >= 0x0300 */



ODBC_INTERFACE RETCODE SQL_API
SQLSetCursorName (SQLHSTMT StatementHandle,
		  SQLCHAR * CursorName, SQLSMALLINT NameLength)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLSetCursorName called\n");

  DEBUG_TIMESTAMP (START_SQLSetCursorName);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_set_cursor_name (stmt_handle, CursorName, NameLength);

  DEBUG_TIMESTAMP (END_SQLSetCursorName);

  return (rc);
}

#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLSetDescField (SQLHDESC DescriptorHandle,
		 SQLSMALLINT RecNumber,
		 SQLSMALLINT FieldIdentifier,
		 SQLPOINTER Value, SQLINTEGER BufferLength)
{
  RETCODE rc = SQL_SUCCESS;
  SQLSMALLINT is_driver;

  OutputDebugString ("SQLSetDescField called\n");

  DEBUG_TIMESTAMP (START_SQLSetDescField);

  odbc_free_diag (((ODBC_DESC *) DescriptorHandle)->diag, RESET);

  if (odbc_is_ird ((ODBC_DESC *) DescriptorHandle) &&
      (FieldIdentifier == SQL_DESC_ARRAY_STATUS_PTR &&
       FieldIdentifier == SQL_DESC_ROWS_PROCESSED_PTR))
    {
      is_driver = 1;
    }
  else
    {
      is_driver = 0;
    }

  rc = odbc_set_desc_field ((ODBC_DESC *) DescriptorHandle, RecNumber,
			    FieldIdentifier, Value, BufferLength, is_driver);

  DEBUG_TIMESTAMP (END_SQLSetDescField);

  ODBC_RETURN (rc, DescriptorHandle);

}

ODBC_INTERFACE RETCODE SQL_API
SQLSetDescRec (SQLHDESC DescriptorHandle,
	       SQLSMALLINT RecNumber,
	       SQLSMALLINT Type,
	       SQLSMALLINT SubType,
	       SQLLEN Length,
	       SQLSMALLINT Precision,
	       SQLSMALLINT Scale,
	       SQLPOINTER Data, SQLLEN * StringLength, SQLLEN * Indicator)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLSetDescRec called\n");

  DEBUG_TIMESTAMP (START_SQLSetDescRec);

  odbc_free_diag (((ODBC_DESC *) DescriptorHandle)->diag, RESET);

  rc = odbc_set_desc_rec ((ODBC_DESC *) DescriptorHandle, RecNumber, Type,
			  SubType, Length, Precision,
			  Scale, Data, StringLength, Indicator);

  DEBUG_TIMESTAMP (END_SQLSetDescRec);

  ODBC_RETURN (rc, DescriptorHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLSetEnvAttr (SQLHENV EnvironmentHandle,
	       SQLINTEGER Attribute,
	       SQLPOINTER Value, SQLINTEGER StringLength)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_ENV *env;

  OutputDebugString ("SQLSetEnvAttr called\n");

  DEBUG_TIMESTAMP (START_SQLSetEnvAttr);

  env = (ODBC_ENV *) EnvironmentHandle;

  if (env != NULL)
    {
      odbc_free_diag (env->diag, RESET);
    }

  if (env == NULL || env->handle_type != SQL_HANDLE_ENV)
    {
      return SQL_INVALID_HANDLE;
    }

  rc = odbc_set_env_attr (env, Attribute, Value, StringLength);

  DEBUG_TIMESTAMP (END_SQLSetEnvAttr);

  ODBC_RETURN (rc, env);
}
#endif /* ODBCVER >= 0x0300 */



#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLSetStmtAttr (SQLHSTMT StatementHandle,
		SQLINTEGER Attribute,
		SQLPOINTER Value, SQLINTEGER StringLength)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLSetStmtAttr called\n");

  DEBUG_TIMESTAMP (START_SQLSetStmtAttr);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_set_stmt_attr (stmt_handle, Attribute, Value, StringLength, 0);

  DEBUG_TIMESTAMP (END_SQLSetStmtAttr);

  ODBC_RETURN (rc, StatementHandle);
}
#endif


ODBC_INTERFACE RETCODE SQL_API
SQLSpecialColumns (SQLHSTMT StatementHandle,
		   SQLUSMALLINT IdentifierType,
		   SQLCHAR * CatalogName,
		   SQLSMALLINT NameLength1,
		   SQLCHAR * SchemaName,
		   SQLSMALLINT NameLength2,
		   SQLCHAR * TableName,
		   SQLSMALLINT NameLength3,
		   SQLUSMALLINT Scope, SQLUSMALLINT Nullable)
{
  RETCODE rc = SQL_SUCCESS;

  SQLCHAR *stCatalogName = NULL;
  SQLCHAR *stSchemaName = NULL;
  SQLCHAR *stTableName = NULL;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLSpecialColumns called\n");

  DEBUG_TIMESTAMP (START_SQLSpecialColumns);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  stCatalogName = UT_MAKE_STRING (CatalogName, NameLength1);
  stSchemaName = UT_MAKE_STRING (SchemaName, NameLength2);
  stTableName = UT_MAKE_STRING (TableName, NameLength3);

  rc = odbc_special_columns (stmt_handle, IdentifierType,
			     stCatalogName, stSchemaName, stTableName, Scope,
			     Nullable);

  NA_FREE (stCatalogName);
  NA_FREE (stSchemaName);
  NA_FREE (stTableName);

  DEBUG_TIMESTAMP (END_SQLSpecialColumns);

  ODBC_RETURN (rc, StatementHandle);
}


ODBC_INTERFACE RETCODE SQL_API
SQLStatistics (SQLHSTMT StatementHandle,
	       SQLCHAR * CatalogName,
	       SQLSMALLINT NameLength1,
	       SQLCHAR * SchemaName,
	       SQLSMALLINT NameLength2,
	       SQLCHAR * TableName,
	       SQLSMALLINT NameLength3,
	       SQLUSMALLINT Unique, SQLUSMALLINT Reserved)
{
  RETCODE rc = SQL_SUCCESS;
  SQLCHAR *stCatalogName = NULL;
  SQLCHAR *stSchemaName = NULL;
  SQLCHAR *stTableName = NULL;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLStatistics called\n");

  DEBUG_TIMESTAMP (START_SQLStatistics);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  stCatalogName = UT_MAKE_STRING (CatalogName, NameLength1);
  stSchemaName = UT_MAKE_STRING (SchemaName, NameLength2);
  stTableName = UT_MAKE_STRING (TableName, NameLength3);

  rc = odbc_statistics (stmt_handle, stCatalogName,
			stSchemaName, stTableName, Unique, Reserved);

  NA_FREE (stCatalogName);
  NA_FREE (stSchemaName);
  NA_FREE (stTableName);

  DEBUG_TIMESTAMP (END_SQLStatistics);

  ODBC_RETURN (rc, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLTablePrivileges (SQLHSTMT StatementHandle,
		    SQLCHAR * CatalogName,
		    SQLSMALLINT NameLength1,
		    SQLCHAR * SchemaName,
		    SQLSMALLINT NameLength2,
		    SQLCHAR * TableName, SQLSMALLINT NameLength3)
{
  RETCODE odbc_retval = SQL_SUCCESS;

  SQLCHAR *stCatalogName = NULL;
  SQLCHAR *stSchemaName = NULL;
  SQLCHAR *stTableName = NULL;

  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLTablePrivileges called\n");
  DEBUG_TIMESTAMP (START_SQLTablePrivileges);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  stCatalogName = UT_MAKE_STRING (CatalogName, NameLength1);
  stSchemaName = UT_MAKE_STRING (SchemaName, NameLength2);
  stTableName = UT_MAKE_STRING (TableName, NameLength3);

  odbc_retval =
    odbc_table_privileges (stmt_handle, stCatalogName, stSchemaName,
			   stTableName);

  NA_FREE (stCatalogName);
  NA_FREE (stSchemaName);
  NA_FREE (stTableName);

  DEBUG_TIMESTAMP (END_SQLTablePrivileges);

  ODBC_RETURN (odbc_retval, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLTables (SQLHSTMT StatementHandle,
	   SQLCHAR * CatalogName,
	   SQLSMALLINT NameLength1,
	   SQLCHAR * SchemaName,
	   SQLSMALLINT NameLength2,
	   SQLCHAR * TableName,
	   SQLSMALLINT NameLength3,
	   SQLCHAR * TableType, SQLSMALLINT NameLength4)
{
  RETCODE rc = SQL_SUCCESS;
  SQLCHAR *stCatalogName = NULL;
  SQLCHAR *stSchemaName = NULL;
  SQLCHAR *stTableName = NULL;
  SQLCHAR *stTableType = NULL;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLTables called\n");

  DEBUG_TIMESTAMP (START_SQLTables);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  stCatalogName = UT_MAKE_STRING (CatalogName, NameLength1);
  stSchemaName = UT_MAKE_STRING (SchemaName, NameLength2);
  stTableName = UT_MAKE_STRING (TableName, NameLength3);
  stTableType = UT_MAKE_STRING (TableType, NameLength4);

  rc = odbc_tables (stmt_handle, stCatalogName,
		    stSchemaName, stTableName, stTableType);

  NA_FREE (stCatalogName);
  NA_FREE (stSchemaName);
  NA_FREE (stTableName);
  NA_FREE (stTableType);

  DEBUG_TIMESTAMP (END_SQLTables);

  ODBC_RETURN (rc, StatementHandle);
}

/************************************************************************
 *																		*
 *                           LEVEL 1									*
 *																		*
 ************************************************************************/

ODBC_INTERFACE RETCODE SQL_API
SQLMoreResults (SQLHSTMT StatementHandle)
{
  ODBC_STATEMENT *stmt_handle;
  RETCODE rc;

  OutputDebugString ("SQLMoreResults called\n");

  DEBUG_TIMESTAMP (START_SQLMoreResults);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_more_results (stmt_handle);

  DEBUG_TIMESTAMP (END_SQLMoreResults);

  ODBC_RETURN (rc, StatementHandle);
}


#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLBulkOperations (SQLHSTMT StatementHandle, SQLSMALLINT Operation)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLBulkOperations called\n");

  DEBUG_TIMESTAMP (START_SQLBulkOperations);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_bulk_operations (stmt_handle, Operation);

  DEBUG_TIMESTAMP (END_SQLBulkOperations);

  ODBC_RETURN (rc, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLSetPos (SQLHSTMT StatementHandle,
	   SQLSETPOSIROW RowNumber,
	   SQLUSMALLINT Operation, SQLUSMALLINT LockType)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLSetPos called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  rc = odbc_set_pos (stmt_handle, RowNumber, Operation, LockType);

  DEBUG_TIMESTAMP (END_SQLBulkOperations);

  ODBC_RETURN (rc, StatementHandle);
}

#endif /* ODBCVER >= 0x0300 */

#if 0
ODBC_INTERFACE RETCODE SQL_API
SQLBrowseConnect (SQLHDBC ConnectionHandle,
		  SQLCHAR * InConnectionString,
		  SQLSMALLINT StringLength1,
		  SQLCHAR * OutConnectionString,
		  SQLSMALLINT BufferLength, SQLSMALLINT * StringLength2Ptr)
{
  RETCODE rc = SQL_SUCCESS;

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  OutputDebugString ("SQLAllocConnect called\n");

  return (rc);
}



#endif // Level 1

/************************************************************************
 *																		*
 *                           LEVEL 2									*
 *																		*
 ************************************************************************/
#if 0
ODBC_INTERFACE RETCODE SQL_API
SQLColumnPrivileges (SQLHSTMT StatementHandle,
		     SQLCHAR * CatalogName,
		     SQLSMALLINT NameLength1,
		     SQLCHAR * SchemaName,
		     SQLSMALLINT NameLength2,
		     SQLCHAR * TableName,
		     SQLSMALLINT NameLength3,
		     SQLCHAR * ColumnName, SQLSMALLINT NameLength4)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLBrowseConnect called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLDescribeParam (SQLHSTMT StatementHandle,
		  SQLUSMALLINT ParameterNumber,
		  SQLSMALLINT * DataTypePtr,
		  SQLUINTEGER * ParameterSizePtr,
		  SQLSMALLINT * DecimalDigitsPtr, SQLSMALLINT * NullablePtr)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLAllocConnect called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  return (rc);
}

#endif

ODBC_INTERFACE RETCODE SQL_API
SQLPrimaryKeys (SQLHSTMT StatementHandle,
		SQLCHAR * CatalogName, SQLSMALLINT NameLength1,
		SQLCHAR * SchemaName, SQLSMALLINT NameLength2,
		SQLCHAR * TableName, SQLSMALLINT NameLength3)
{
  RETCODE odbc_retval = SQL_SUCCESS;

  SQLCHAR *stCatalogName = NULL;
  SQLCHAR *stSchemaName = NULL;
  SQLCHAR *stTableName = NULL;

  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLPrimaryKeys called\n");
  DEBUG_TIMESTAMP (START_SQLPrimaryKeys);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  stCatalogName = UT_MAKE_STRING (CatalogName, NameLength1);
  stSchemaName = UT_MAKE_STRING (SchemaName, NameLength2);
  stTableName = UT_MAKE_STRING (TableName, NameLength3);

  odbc_retval =
    odbc_primary_keys (stmt_handle, stCatalogName, stSchemaName, stTableName);

  NA_FREE (stCatalogName);
  NA_FREE (stSchemaName);
  NA_FREE (stTableName);

  DEBUG_TIMESTAMP (END_SQLPrimaryKeys);

  ODBC_RETURN (odbc_retval, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLForeignKeys (SQLHSTMT StatementHandle,
		SQLCHAR * PKCatalogName, SQLSMALLINT NameLength1,
		SQLCHAR * PKSchemaName, SQLSMALLINT NameLength2,
		SQLCHAR * PKTableName, SQLSMALLINT NameLength3,
		SQLCHAR * FKCatalogName, SQLSMALLINT NameLength4,
		SQLCHAR * FKSchemaName, SQLSMALLINT NameLength5,
		SQLCHAR * FKTableName, SQLSMALLINT NameLength6)
{
  RETCODE odbc_retval = SQL_SUCCESS;

  SQLCHAR *stPKTableName = NULL;
  SQLCHAR *stFKTableName = NULL;

  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLForeignKeys called\n");
  DEBUG_TIMESTAMP (START_SQLForeignKeys);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  stPKTableName = UT_MAKE_STRING (PKTableName, NameLength3);
  stFKTableName = UT_MAKE_STRING (FKTableName, NameLength6);

  odbc_retval = odbc_foreign_keys (stmt_handle, stPKTableName, stFKTableName);

  NA_FREE (stPKTableName);
  NA_FREE (stFKTableName);

  DEBUG_TIMESTAMP (END_SQLForeignKeys);

  ODBC_RETURN (odbc_retval, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLProcedureColumns (SQLHSTMT StatementHandle,
		     SQLCHAR * CatalogName, SQLSMALLINT NameLength1,
		     SQLCHAR * SchemaName, SQLSMALLINT NameLength2,
		     SQLCHAR * ProcName, SQLSMALLINT NameLength3,
		     SQLCHAR * ColumnName, SQLSMALLINT NameLength4)
{
  RETCODE odbc_retval = SQL_SUCCESS;

  SQLCHAR *stCatalogName = NULL;
  SQLCHAR *stSchemaName = NULL;
  SQLCHAR *stProcName = NULL;
  SQLCHAR *stColumnName = NULL;

  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLProcedureColumns called\n");
  DEBUG_TIMESTAMP (START_SQLProcedureColumns);

  stCatalogName = UT_MAKE_STRING (CatalogName, NameLength1);
  stSchemaName = UT_MAKE_STRING (SchemaName, NameLength2);
  stProcName = UT_MAKE_STRING (ProcName, NameLength3);
  stColumnName = UT_MAKE_STRING (ColumnName, NameLength4);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  odbc_retval =
    odbc_procedure_columns (stmt_handle, stCatalogName, stSchemaName,
			    stProcName, stColumnName);

  NA_FREE (stCatalogName);
  NA_FREE (stSchemaName);
  NA_FREE (stProcName);
  NA_FREE (stColumnName);

  DEBUG_TIMESTAMP (END_SQLProcedureColumns);

  ODBC_RETURN (odbc_retval, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLProcedures (SQLHSTMT StatementHandle,
	       SQLCHAR * CatalogName,
	       SQLSMALLINT NameLength1,
	       SQLCHAR * SchemaName,
	       SQLSMALLINT NameLength2,
	       SQLCHAR * ProcName, SQLSMALLINT NameLength3)
{
  RETCODE odbc_retval = SQL_SUCCESS;

  SQLCHAR *stCatalogName = NULL;
  SQLCHAR *stSchemaName = NULL;
  SQLCHAR *stProcName = NULL;

  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLProcedures called\n");
  DEBUG_TIMESTAMP (START_SQLProcedures);

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  stCatalogName = UT_MAKE_STRING (CatalogName, NameLength1);
  stSchemaName = UT_MAKE_STRING (SchemaName, NameLength2);
  stProcName = UT_MAKE_STRING (ProcName, NameLength3);

  odbc_retval =
    odbc_procedures (stmt_handle, stCatalogName, stSchemaName, stProcName);

  NA_FREE (stCatalogName);
  NA_FREE (stSchemaName);
  NA_FREE (stProcName);

  DEBUG_TIMESTAMP (END_SQLProcedures);

  ODBC_RETURN (odbc_retval, StatementHandle);
}

/****************************************************************
 *                  Driver Manager implements                   *
 ***************************************************************/

/* Dummy Function.. This is implemented in Driver Manager */
#if 0
ODBC_INTERFACE RETCODE SQL_API
SQLDrivers (SQLHENV EnvironmentHandle,
	    SQLUSMALLINT Direction,
	    SQLCHAR * DriverDescription,
	    SQLSMALLINT BufferLength1,
	    SQLSMALLINT * DescriptionLengthPtr,
	    SQLCHAR * DriverAttributes,
	    SQLSMALLINT BufferLength2, SQLSMALLINT * AttributesLengthPtr)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLDrivers called\n");

  return (rc);
}

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
#endif


/************************************************************************
 *                                                                      *
 *                          ODBC 2.0, deprecated                        *
 *                                                                      *
 ************************************************************************/

#if 0
ODBC_INTERFACE RETCODE SQL_API
SQLAllocConnect (SQLHENV EnvironmentHandle, SQLHDBC * ConnectionHandle)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLAllocConnect called\n");

  odbc_free_diag (((ODBC_ENV *) EnvironmentHandle)->diag, RESET);

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLAllocEnv (SQLHENV * EnvironmentHandle)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLAllocEnv called\n");

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLAllocStmt (SQLHDBC ConnectionHandle, SQLHSTMT * StatementHandle)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLAllocStmt called\n");

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  return (rc);
}

#if (ODBCVER >= 0x0300)
ODBC_INTERFACE RETCODE SQL_API
SQLBindParam (SQLHSTMT StatementHandle,
	      SQLUSMALLINT ParameterNumber,
	      SQLSMALLINT ValueType,
	      SQLSMALLINT ParameterType,
	      SQLUINTEGER LengthPrecision,
	      SQLSMALLINT ParameterScale,
	      SQLPOINTER ParameterValue, SQLINTEGER * StrLen_or_Ind)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLBindParam called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  return (rc);
}
#endif

ODBC_INTERFACE RETCODE SQL_API
SQLColAttributes (SQLHSTMT StatementHandle,
		  SQLUSMALLINT icol,
		  SQLUSMALLINT fDescType,
		  SQLPOINTER rgbDesc,
		  SQLSMALLINT cbDescMax,
		  SQLSMALLINT * pcbDesc, SQLINTEGER * pfDesc)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLColAttributes called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  switch (fDescType)
    {
    case SQL_COLUMN_NAME:
      fDescType = SQL_DESC_NAME;
      break;
    case SQL_COLUMN_COUNT:
      fDescType = SQL_DESC_COUNT;
      break;
    case SQL_COLUMN_NULLABLE:
      fDescType = SQL_DESC_NULLABLE;
      break;
    }
  rc = odbc_col_attribute (StatementHandle, icol, fDescType,
			   rgbDesc, cbDescMax, pcbDesc, pfDesc);

  ODBC_RETURN (rc, StatementHandle);
}

ODBC_INTERFACE RETCODE SQL_API
SQLError (SQLHENV EnvironmentHandle,
	  SQLHDBC ConnectionHandle,
	  SQLHSTMT StatementHandle,
	  SQLCHAR * Sqlstate,
	  SQLINTEGER * NativeError,
	  SQLCHAR * MessageText,
	  SQLSMALLINT BufferLength, SQLSMALLINT * TextLength)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLError called\n");

  if (EnvironmentHandle != NULL)
    {
      SQLGetDiagRec (SQL_HANDLE_ENV, EnvironmentHandle, 1, Sqlstate,
		     NativeError, MessageText, BufferLength, TextLength);
      if (*NativeError != 0)
	return rc;
    }
  if (ConnectionHandle != NULL)
    {
      SQLGetDiagRec (SQL_HANDLE_DBC, ConnectionHandle, 1, Sqlstate,
		     NativeError, MessageText, BufferLength, TextLength);
      if (*NativeError != 0)
	return rc;
    }
  if (StatementHandle != NULL)
    {
      SQLGetDiagRec (SQL_HANDLE_STMT, StatementHandle, 1, Sqlstate,
		     NativeError, MessageText, BufferLength, TextLength);
    }
  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLFreeConnect (SQLHDBC ConnectionHandle)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLFreeConnect called\n");

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLFreeEnv (SQLHENV EnvironmentHandle)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLFreeEnv called\n");

  return (rc);
}



ODBC_INTERFACE RETCODE SQL_API
SQLGetConnectOption (SQLHDBC ConnectionHandle,
		     SQLUSMALLINT Option, SQLPOINTER Value)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLGetConnectOption called\n");

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLGetStmtOption (SQLHSTMT StatementHandle,
		  SQLUSMALLINT Option, SQLPOINTER Value)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLGetStmtOption called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLSetConnectOption (SQLHDBC ConnectionHandle,
		     SQLUSMALLINT Option, SQLUINTEGER Value)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLSetConnectOption called\n");

  odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLSetStmtOption (SQLHSTMT StatementHandle,
		  SQLUSMALLINT Option, SQLUINTEGER Value)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLSetStmtOption called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLParamOptions (SQLHSTMT StatementHandle,
		 SQLUINTEGER crow, SQLUINTEGER * pirow)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLParamOptions called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLTransact (SQLHENV EnvironmentHandle,
	     SQLHDBC ConnectionHandle, SQLUSMALLINT CompletionType)
{
  RETCODE rc = SQL_SUCCESS;

  OutputDebugString ("SQLTransact called\n");

  if (EnvironmentHandle != NULL)
    {
      odbc_free_diag (((ODBC_ENV *) EnvironmentHandle)->diag, RESET);
      rc = odbc_end_tran (SQL_HANDLE_ENV, EnvironmentHandle, CompletionType);
      ODBC_RETURN (rc, EnvironmentHandle);
    }
  else if (ConnectionHandle != NULL)
    {
      odbc_free_diag (((ODBC_CONNECTION *) ConnectionHandle)->diag, RESET);
      rc = odbc_end_tran (SQL_HANDLE_DBC, ConnectionHandle, CompletionType);
      ODBC_RETURN (rc, ConnectionHandle);
    }

  return (SQL_SUCCESS);
}




ODBC_INTERFACE RETCODE SQL_API
SQLSetParam (SQLHSTMT StatementHandle,
	     SQLUSMALLINT ParameterNumber,
	     SQLSMALLINT ValueType,
	     SQLSMALLINT ParameterType,
	     SQLUINTEGER LengthPrecision,
	     SQLSMALLINT ParameterScale,
	     SQLPOINTER ParameterValue, SQLINTEGER * StrLen_or_Ind)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLSetParam called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  return (rc);
}

ODBC_INTERFACE RETCODE SQL_API
SQLSetScrollOptions (SQLHSTMT StatementHandle,
		     SQLUSMALLINT fConcurrency,
		     SQLINTEGER crowKeyset, SQLUSMALLINT crowRowset)
{
  RETCODE rc = SQL_SUCCESS;
  ODBC_STATEMENT *stmt_handle;

  OutputDebugString ("SQLSetScrollOptions called\n");

  stmt_handle = (ODBC_STATEMENT *) StatementHandle;
  odbc_free_diag (stmt_handle->diag, RESET);

  return (rc);
}

#endif

/************************************************************************
 * name:  ConnectDlgProc
 * arguments:
 * returns/side-effects:
 * description:
 *  SQLDriverConnect�� ���Ǵ� dialog box�� ����.
 * NOTE:
 ************************************************************************/
PRIVATE INT_PTR CALLBACK
ConnectDlgProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  DriverConnectInfo *dci = NULL;
  char ptr[16];

  switch (message)
    {
      HCURSOR hOldCursor;	// Default Cursor Shape

    case WM_INITDIALOG:
      // display list of available data sources

      hOldCursor = SetCursor (LoadCursor ((HINSTANCE) NULL, IDC_WAIT));

      dci = (DriverConnectInfo *) lParam;

      sprintf (ptr, "%p", dci);

      SetDlgItemText (hWnd, IDC_DCI_PTR, ptr);
      SetDlgItemText (hWnd, IDTEXT_USERNAME, dci->user);
      SetDlgItemText (hWnd, IDTEXT_PASSWORD, dci->pwd);

      SetCursor (hOldCursor);
      break;

    case WM_COMMAND:

      switch (LOWORD (wParam))
	{
	case IDOK:		// make a connection using the supplied values

	  hOldCursor = SetCursor (LoadCursor ((HINSTANCE) NULL, IDC_WAIT));

	  GetDlgItemText (hWnd, IDC_DCI_PTR, ptr, sizeof (ptr) - 1);
	  sscanf (ptr, "%p", &dci);

	  GetDlgItemText (hWnd, IDTEXT_USERNAME, dci->user, ITEMBUFLEN - 1);
	  GetDlgItemText (hWnd, IDTEXT_PASSWORD, dci->pwd, ITEMBUFLEN - 1);

	  EndDialog (hWnd, FALSE);

	  SetCursor (hOldCursor);
	  break;

	case IDCANCEL:

	  EndDialog (hWnd, FALSE);
	  break;

	default:
	  return (FALSE);
	  break;
	}
      break;

    default:

      return (FALSE);
      break;
    }

  return (TRUE);
}
