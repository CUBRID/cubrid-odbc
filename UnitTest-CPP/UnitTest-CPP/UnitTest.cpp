#include "stdafx.h"

#include <stdio.h>  
#include <string.h>  
#include <windows.h>  
#include <sql.h>  
#include <sqlext.h>  
#include <odbcss.h>

#define MAXBUFLEN	640
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestCPP
{		
	TEST_CLASS(UnitTest)
	{
	public:
		TEST_METHOD(TestClassInit)
		{
		}

		TEST_METHOD(TestAutoCommit)
		{
			RETCODE retcode;

			SQLHENV env;
			SQLHDBC dbc;
			SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SWORD plm_pcbErrorMsg = 0;
			SQLINTEGER diag_rec;
			SQLINTEGER autocommit;

			/* Allocate an environment handle */
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
			/* We want ODBC 3 support */
			SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			/* Allocate a connection handle */
			SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

			retcode = SQLDriverConnect(dbc, NULL, L"DSN=CUBRID Driver Unicode;DB_NAME=demodb;SERVER=test-db-server;PORT=33000;USER=dba;PWD=;CHARSET=utf-8;AUTOCOMMIT=ON", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (retcode == SQL_ERROR) {
				SQLGetDiagField(SQL_HANDLE_DBC, dbc, 0, SQL_DIAG_NUMBER, &diag_rec, 0, &plm_pcbErrorMsg);
			}
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLGetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, NULL);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			Assert::AreEqual((int)autocommit, 1);

			SQLDisconnect(dbc);
			SQLFreeHandle(SQL_HANDLE_DBC, dbc);
			SQLFreeHandle(SQL_HANDLE_ENV, env);
		}

		TEST_METHOD(APIS_797_QueryPlan)
		{
			RETCODE retcode;

			SQLWCHAR query_plan[32768] = { 0, };

			SQLHENV env;
			SQLHDBC dbc;
			SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SWORD plm_pcbErrorMsg = 0;
			SQLINTEGER diag_rec;

			SQLLEN len;

			/* Allocate an environment handle */
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
			/* We want ODBC 3 support */
			SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			/* Allocate a connection handle */
			SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

			//retcode = SQLDriverConnect(dbc, NULL, L"DSN=CUBRID Driver Unicode;DB_NAME=demodb;SERVER=test-db-server;PORT=33000;USER=dba;PWD=;CHARSET=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			retcode = SQLConnect(dbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);

			if (retcode == SQL_ERROR) {
				SQLGetDiagField(SQL_HANDLE_DBC, dbc, 0, SQL_DIAG_NUMBER, &diag_rec, 0, &plm_pcbErrorMsg);
			}

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			// Allocate statement handle and execute a query
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hStmt);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLExecDirect(hStmt, L"SELECT name as \"user\", db_user, groups FROM db_user", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLFetch(hStmt);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			// Get Query Plan
			retcode = SQLGetData(hStmt, 1, SQL_C_DEFAULT, (SQLPOINTER)query_plan, 0, &len);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			if (retcode == SQL_SUCCESS_WITH_INFO) {
				retcode = SQLGetData(hStmt, 1, SQL_C_DEFAULT, (SQLPOINTER)query_plan, len, &len);
			}

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			if (strlen((const char *)query_plan) > 0) {
				int c = strncmp((const char *)query_plan, (const char *)L"Join graph", 10);
				Assert::AreEqual(c, 0);
			}
			else {
				Assert::Fail(L"Query Plan is null");
			}
			// Clean up.   
			SQLDisconnect(dbc);
			SQLFreeHandle(SQL_HANDLE_DBC, dbc);
			SQLFreeHandle(SQL_HANDLE_ENV, env);
		}

		TEST_METHOD(APIS_926_Array_Bind_UNICODE)
		{
			SQLHENV		hEnv;
			SQLHDBC		hDbc;
			SQLHSTMT	hStmt;
#define ARRAY_SIZE	8
#define	MAXLEN		40
#define	STR_SZ		10
#define COL_SIZE	20

			RETCODE retcode(0);
			int nArraySize(ARRAY_SIZE); // number of rows to insert
			int nMaxLen(MAXLEN);		// offset for a record
			SQLUSMALLINT	ParamStatusArray[ARRAY_SIZE];
			SQLLEN			ParamsProcessed = 0;
			SQLLEN pIndicator[8];	// StrLen_or_IndPtr array is required but values ere meaningless

			// Connect DB

			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLConnect(hDbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
			retcode = SQLExecDirect(hStmt, L"drop table if exists apis926", SQL_NTS);

			// Create table apis926 (col1 int, col2 varchar(20));
			retcode = SQLExecDirect(hStmt, L"create table apis926 (col1 int auto_increment, col2 varchar(20))", SQL_NTS);

			retcode = SQLPrepare(hStmt, (SQLWCHAR*)(wchar_t*)L"INSERT INTO apis926(col2) VALUES(?)", SQL_NTS);

			// Prepare Bind Arrays
			retcode = SQLSetStmtAttr(hStmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0);
			retcode = SQLSetStmtAttr(hStmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)ARRAY_SIZE, 0);
			retcode = SQLSetStmtAttr(hStmt, SQL_ATTR_PARAM_STATUS_PTR, ParamStatusArray, ARRAY_SIZE);
			retcode = SQLSetStmtAttr(hStmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &ParamsProcessed, 0);

			wchar_t address[ARRAY_SIZE][STR_SZ] = { L"한ab", L"ab", L"123", L"6234", L"9", L"db", L"Abc", L"Bbcd" };

			BYTE* pBufData = (BYTE*)new wchar_t[MAXLEN * ARRAY_SIZE];
			BYTE *ppBuf = pBufData;
			memset(pBufData, 0, (MAXLEN * nArraySize) * sizeof(wchar_t));

			for (int i = 0; i < ARRAY_SIZE; i++)
			{
				memcpy(ppBuf, address[i], STR_SZ * sizeof(wchar_t));
				ppBuf = ppBuf + nMaxLen;
			}

			retcode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR,
				COL_SIZE, 0, pBufData, MAXLEN, pIndicator);

			retcode = SQLExecute(hStmt);

			ppBuf = pBufData;
			for (int i = 0; i < ARRAY_SIZE; i++)	// insert additional rows
			{
				swprintf(address[i], L"1-큐브 %d", i);
				memcpy(ppBuf, address[i], STR_SZ * sizeof(wchar_t));
				ppBuf = ppBuf + nMaxLen;
			}

			retcode = SQLExecute(hStmt);

			ppBuf = pBufData;
			for (int i = 0; i < ARRAY_SIZE; i++)	// insert additional rows
			{
				swprintf(address[i], L"2-리드 %d", i);
				memcpy(ppBuf, address[i], STR_SZ * sizeof(wchar_t));
				ppBuf = ppBuf + nMaxLen;
			}

			retcode = SQLExecute(hStmt);

			// Disconnect
			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			SQLFreeStmt(hStmt, SQL_DROP);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		TEST_METHOD(APIS_929_ODBC_VERSION)
		{
			SQLHENV         hEnv;
			SQLHDBC         hDbc;
			SQLHSTMT        hStmt;

			RETCODE retcode(0);
			WCHAR szDriver[256];
			WCHAR msg[512];
			SWORD nDriverLen;

			// Connect DB

			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLConnect(hDbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			memset(szDriver, 0, sizeof(szDriver));

			retcode = SQLGetInfo(hDbc, SQL_DRIVER_VER, szDriver, sizeof(szDriver), &nDriverLen);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			wsprintf(msg, L"++ CUBRID ODBC Version = %s", szDriver);
			Logger::WriteMessage(msg);

			// Disconnect
			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}
		TEST_METHOD(APIS_932_DEFAULT_AUTO_COMMIT_ON)
		{
			SQLHENV         hEnv;
			SQLHDBC         hDbc;
			SQLHSTMT        hStmt;
			WCHAR msg[512];
			SQLINTEGER		autocommit;
			int		AUTO_COMMIT_OFF = (int)SQL_AUTOCOMMIT_OFF;
			int		AUTO_COMMIT_ON = (int)SQL_AUTOCOMMIT_ON;

			RETCODE retcode(0);

			// case APIS_932_DEFAULT_AUTO_COMMIT_ON-1 (connection url with default):
			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLDriverConnect(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=public;pwd=;db_name=demodb;charset=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLGetConnectAttr (hDbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, NULL);
			wsprintf(msg, L"case 1 (url off by default): ++ AUTOCOMMIT = %s", autocommit ? L"SQL_AUTOCOMMIT_ON (1)" : L"SQL_AUTOCOMMIT_OFF (0)");
			Logger::WriteMessage(msg);
			Assert::AreEqual((int)autocommit, AUTO_COMMIT_ON);

			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

			// case APIS_932_DEFAULT_AUTO_COMMIT_ON-2 (connection url with autocommit=on):
			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLDriverConnect(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=public;pwd=;db_name=demodb;charset=utf-8;autocommit=on", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLGetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, NULL);
			wsprintf(msg, L"case 2 (url on): ++ AUTOCOMMIT = %s", autocommit ? L"SQL_AUTOCOMMIT_ON (1)" : L"SQL_AUTOCOMMIT_OFF (0)");
			Logger::WriteMessage(msg);
			Assert::AreEqual((int)autocommit, AUTO_COMMIT_ON);

			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

			// case APIS_932_DEFAULT_AUTO_COMMIT_ON-3 (connection url with autocommit=off):
			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLDriverConnect(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=public;pwd=;db_name=demodb;charset=utf-8;autocommit=off", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLGetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, NULL);
			wsprintf(msg, L"case 3 (url off): ++ AUTOCOMMIT = %s", autocommit ? L"SQL_AUTOCOMMIT_ON (1)" : L"SQL_AUTOCOMMIT_OFF (0)");
			Logger::WriteMessage(msg);
			Assert::AreEqual((int)autocommit, AUTO_COMMIT_OFF);

			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

			// case APIS_932_DEFAULT_AUTO_COMMIT_ON-4 (connection DSN with autocommit=off):
			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLConnect(hDbc, L"cubrid_autocommit_off", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);

			retcode = SQLGetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, NULL);
			wsprintf(msg, L"case 4 (DSN off): ++ AUTOCOMMIT = %s", autocommit ? L"SQL_AUTOCOMMIT_ON (1)" : L"SQL_AUTOCOMMIT_OFF (0)");
			Logger::WriteMessage(msg);
			Assert::AreEqual((int)autocommit, AUTO_COMMIT_OFF);

			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

			// case APIS_932_DEFAULT_AUTO_COMMIT_ON-5 (connection DSN with autocommit=on):
			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLConnect(hDbc, L"cubrid_autocommit_on", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);

			retcode = SQLGetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, NULL);
			wsprintf(msg, L"case 5 (DSN on): ++ AUTOCOMMIT = %s", autocommit ? L"SQL_AUTOCOMMIT_ON (1)" : L"SQL_AUTOCOMMIT_OFF (0)");
			Logger::WriteMessage(msg);
			Assert::AreEqual((int)autocommit, AUTO_COMMIT_ON);

			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

			// case APIS_932_DEFAULT_AUTO_COMMIT_ON-6 (connection DSN with autocommit=off):
			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLConnect(hDbc, L"cubrid_autocommit_unknown", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);

			retcode = SQLGetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, NULL);
			wsprintf(msg, L"case 6  (DSN on by default):++ AUTOCOMMIT = %s", autocommit ? L"SQL_AUTOCOMMIT_ON (1)" : L"SQL_AUTOCOMMIT_OFF (0)");
			Logger::WriteMessage(msg);
			Assert::AreEqual((int)autocommit, AUTO_COMMIT_ON);

			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		TEST_METHOD(APIS_936_TextLength2Ptr_NULL_CHK)
		{
			/*
			 * TEST Description
			 * 
			 * 1. Create user user1 in the dba account.
			 * 2. Log in to demodb as 'user1' and create the following table (omit_schema=yes)
			 *    'parent', 't1', 't2', 'code'
			 * 3. Request a table list.
			 *    - expected: ''parent', 't1', 't2', 'code''
			 *    - if omit_schema=no (default), you will receive
			 *      public.stadium
			 *      public.code
			 *      public.nation
			 *      public.event
			 *      public.athlete
			 *      public.participant
			 *      public.olympic
			 *      public.game
			 *      public.record
			 *      public.history
			 *      user1.parent
			 *      user1.t1
			 *      user1.t2
			 *      user1.code
			 * 4. Request Column names of table 'code'
			 *    - expected: 'id', 'city', 'col1'
			 *    - if omit_schema=no (default), you will receive (colums of 'user1.code' & 'public.code')
			 *       s_name
			 *       id
			 *       f_name
			 *       city
			 *       col1
			 * 5. Login to demodb as 'public' (omit_schema=yes)
			 * 6. Request table names with table 'user1.parent' as the parent table of the foreign key.
			 *    - exprect: returns -1 for SQLForeignKeysW () even 'public' has privilege on the table
			 */
			SQLHENV         hEnv;
			SQLHDBC         hDbc;
			SQLHSTMT        hStmt;
			WCHAR			query2 [512] = L"select date_format(now(),'%T')";
			WCHAR			query [512] = L"show tables";
			WCHAR			converted_qry [512];
			WCHAR			col1[512];
			WCHAR msg[512];
			SQLINTEGER		length;
			SQLINTEGER		retcode;
			SQLLEN			len;
			int				num_rows = 0;

			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLDriverConnect(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=public;pwd=;db_name=demodb;charset=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

			// Case-1: Check if SQL_ERROR is returned when TextLength2Ptr is NULL
			retcode = SQLNativeSqlW (hDbc, query, sizeof(query), converted_qry, sizeof(converted_qry), NULL);
			Assert::AreEqual((int)retcode, SQL_ERROR);

			// Case-2: Convert and execute with proper TextLength2Ptr
			retcode = SQLNativeSqlW(hDbc, query, sizeof(query), converted_qry, sizeof(converted_qry), &length);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			wsprintf(msg, L"len = %d, converted qry = '%s'", length, converted_qry);
			Logger::WriteMessage(msg);

			retcode = SQLPrepareW(hStmt, converted_qry, SQL_NTS);
			retcode = SQLExecute(hStmt);
			retcode = SQLBindCol(hStmt, 1, SQL_C_WCHAR, (SQLPOINTER)col1, sizeof(col1), &len);

			memset(col1, 0, sizeof(col1));
			while ((retcode = SQLFetch(hStmt)) == SQL_SUCCESS) {
				wsprintf (msg, L"rows [%2d]: col1 = '%s'\n", ++num_rows, col1);
				Logger::WriteMessage(msg);
				memset(col1, 0, sizeof(col1));
			}

			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		TEST_METHOD(APIS_900_CATALOG_AUTO_COMMIT)
		{
			SQLHENV         hEnv;
			SQLHDBC         hDbc;
			SQLHSTMT        hstmt;
			CHAR			msg[512];
			SQLINTEGER		length;
			SQLINTEGER		retcode;
			SQLLEN			len;
			SQLSMALLINT		columns;
			SQLCHAR			tablename [512];
			SQLWCHAR		tablenameW [512];
			SQLLEN			StrLen_or_IndPtr;
			int				MAX_NUM_STMTS = 100;
			SQLCHAR			foreign[512];

			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLDriverConnect(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=public;pwd=;db_name=demodb;charset=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);

			retcode = SQLTables(hstmt, NULL, 0, NULL, 0, NULL, 0, L"TABLE", SQL_NTS);
			retcode = SQLNumResultCols(hstmt, &columns);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			sprintf(msg, "num cols = %d", columns);
			Logger::WriteMessage(msg);

			while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) {
				retcode = SQLGetData (hstmt, 3, SQL_C_CHAR, tablename, sizeof(tablename), &StrLen_or_IndPtr);
				sprintf(msg, "%s", tablename);
				if (StrLen_or_IndPtr != NULL)
				{
					Logger::WriteMessage(msg);
				}
			}

			wsprintf(tablenameW, L"game");
			for (int i = 0; i < MAX_NUM_STMTS; i++) {
				retcode = SQLPrimaryKeysW(hstmt, NULL, 0, NULL, 0, tablenameW, SQL_NTS);
				Assert::AreNotEqual((int)retcode, SQL_ERROR);
				while ((retcode == SQL_SUCCESS) || (retcode == SQL_SUCCESS_WITH_INFO)) {
					retcode = SQLFetch(hstmt);
				}
			}

			retcode = SQLFreeStmt(hstmt, SQL_CLOSE);

			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

		}

		TEST_METHOD(APIS_939_TABLE_COLUMN_COMMENT)
		{
			SQLHENV         hEnv;
			SQLHDBC         hDbc;
			SQLHSTMT        hstmt;
			CHAR			msg[512];
			SQLINTEGER		retcode;
			SQLSMALLINT		columns;
			int				MAX_NUM_STMTS = 100;
			SQLLEN			indicator;
			SQLWCHAR		*qry_alt_table_comment = L"ALTER TABLE olympic COMMENT = 'this is new comment for olympic2';";
			SQLWCHAR		*qry_alt_column_comment = L"ALTER TABLE olympic COMMENT ON COLUMN host_year = 'The year the Olympics were held';";

			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLDriverConnect(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=public;pwd=;db_name=demodb;charset=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);

			retcode = SQLExecDirect(hstmt, qry_alt_table_comment, SQL_NTS);
			retcode = SQLExecDirect(hstmt, qry_alt_column_comment, SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			Logger::WriteMessage("Phase1: ********** Check for Table COMMENT **********");
			retcode = SQLTables(hstmt, NULL, 0, NULL, 0, NULL, 0, L"TABLE", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			// fetch table name and comments
			while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) {
				SQLUSMALLINT i;
				SQLCHAR table_name[256];
				SQLCHAR comments[2048];

				memset(table_name, 0, sizeof(table_name));
				memset(comments, 0, sizeof(comments));

				retcode = SQLGetData(hstmt, 3, SQL_C_CHAR, table_name, sizeof(table_name), &indicator);
				retcode = SQLGetData(hstmt, 5, SQL_C_CHAR, comments, sizeof(comments), &indicator);
				Assert::AreNotEqual((int)retcode, SQL_ERROR);

				if (SQL_SUCCEEDED(retcode)) {
					int ret = -1;

					if (strcmp((const char *) table_name, "olympic") == 0 ||
						strcmp((const char *) table_name, "public.olympic") == 0) {
						ret = strcmp((const char *)comments, "this is new comment for olympic2");
						Assert::AreEqual((int)ret, 0);
						Logger::WriteMessage("********** Passed for Table COMMENT **********");
					}

					sprintf(msg, "table = '%s', comment = '%s'", table_name, comments);
					Logger::WriteMessage(msg);
				}
			}
			Logger::WriteMessage("********** Passed for Table COMMENT **********");

			retcode = SQLFreeStmt(hstmt, SQL_CLOSE);
			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);

			Logger::WriteMessage("Phase2: ********** Check for Column COMMENT **********");

			retcode = SQLColumnsW(hstmt, NULL, 0, NULL, 0, L"olympic", SQL_NTS, L"host_year", SQL_NTS);

			while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) {
				SQLCHAR buf[255];
				int ret = -1;

				memset(buf, 0, sizeof(buf));

				retcode = SQLGetData(hstmt, 12, SQL_C_CHAR, buf, sizeof(buf), &indicator);

				if (SQL_SUCCEEDED(retcode)) {
					ret = strcmp((const char *)buf, "The year the Olympics were held");
					sprintf(msg, "COLUMN COMMENT: olympic (host_year) = '%s', compare result = %d", buf, ret);
					Logger::WriteMessage(msg);
					Assert::AreEqual((int)ret, 0);
				}
			}

			Logger::WriteMessage("********** Passed for Column COMMENT **********");

			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

		}
		TEST_METHOD(APIS_943_SQLCOLUMNSW)
		{
			SQLHENV         hEnv;
			SQLHDBC         hDbc;
			SQLHSTMT        hstmt;
			CHAR			msg[512];
			SQLINTEGER		retcode;
			SQLSMALLINT		num_cols = 0;
			SQLLEN			indicator;

			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLDriverConnectW(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=public;pwd=;db_name=demodb;charset=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);

			retcode = SQLColumnsW(hstmt, NULL, 0, NULL, 0, L"participant", SQL_NTS, NULL, 0);

			while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) {
				SQLCHAR buf[255];

				memset(buf, 0, sizeof(buf));

				retcode = SQLGetData(hstmt, 4, SQL_C_CHAR, buf, sizeof(buf), &indicator);

				if (SQL_SUCCEEDED(retcode)) {
					sprintf(msg, "COLUMN [%d]: %s", 1+num_cols++, buf);
					Logger::WriteMessage(msg);
				}
			}

			sprintf(msg, "Num Cols = %d", num_cols);
			Logger::WriteMessage(msg);

			Assert::AreNotEqual((int) num_cols, 0);

			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}
		TEST_METHOD(APIS_943_FOREIGNKEYSW)
		{
			SQLHENV         hEnv;
			SQLHDBC         hDbc;
			SQLHSTMT        hstmt;
			CHAR			msg[512];
			SQLINTEGER		retcode;
			SQLSMALLINT		num_tables = 0;
			SQLLEN			indicator;

			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLDriverConnectW(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=public;pwd=;db_name=demodb;charset=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);

			retcode = SQLForeignKeysW (hstmt, NULL, 0, NULL, 0, L"olympic", SQL_NTS, NULL, 0, NULL, 0, NULL, 0);

			while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) {
				SQLCHAR buf[255];

				memset(buf, 0, sizeof(buf));

				retcode = SQLGetData(hstmt, 7, SQL_C_CHAR, buf, sizeof(buf), &indicator);

				if (SQL_SUCCEEDED(retcode)) {
					sprintf(msg, "FK Table [%d]: %s", 1 + num_tables++, buf);
					Logger::WriteMessage(msg);
				}
			}

			sprintf(msg, "Num FK Tables = %d", num_tables);
			Logger::WriteMessage(msg);

			Assert::AreNotEqual((int)num_tables, 0);

			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

		}

		TEST_METHOD(APIS_942_OMIT_SCHEMA)
		{
			SQLHENV         hEnv;
			SQLHDBC         hDbc;
			SQLHSTMT        hstmt;
			CHAR			msg[512];
			SQLINTEGER		retcode;
			SQLLEN			StrLen_or_IndPtr;
			SQLCHAR			table_name[256];
			SQLCHAR			column_name[256];
			char			my_tables[100][256];
			int				num_table = 0;

			SQLWCHAR		*qry_create_user = L"CREATE USER user1 password '1234'";
			SQLWCHAR		*qry_drop_user = L"DROP USER user1";
			SQLWCHAR		*qry_create_table0 = 
								L"CREATE TABLE parent( \
									id INT NOT NULL PRIMARY KEY, \
									phome VARCHAR(10) \
									)";

			SQLWCHAR		*qry_create_table1 = 
								L"CREATE TABLE t1 (\
									id INT NOT NULL, \
									name VARCHAR(20) NOT NULL, \
									col1 INT REFERENCES PARENT(id)\
									)";
			SQLWCHAR		*qry_create_table2 = 
								L"CREATE TABLE t2 ( \
									id INT NOT NULL, \
									address VARCHAR(20) NOT NULL, \
									CONSTRAINT pk_id PRIMARY KEY(id), \
									CONSTRAINT fk_id FOREIGN KEY(ID) REFERENCES parent(id) \
									ON DELETE CASCADE ON UPDATE RESTRICT \
									); ";
			SQLWCHAR		*qry_create_table3 = 
								L"CREATE TABLE code ( \
									id INT NOT NULL, \
									city VARCHAR(20) NOT NULL, \
									col1 INT REFERENCES PARENT(id) \
									); ";

			SQLWCHAR		*drop_table_t1 = L"DROP table if exists user1.t1";
			SQLWCHAR		*drop_table_t2 = L"DROP table if exists user1.t2";
			SQLWCHAR		*drop_table_t3 = L"DROP table if exists user1.code";
			SQLWCHAR		*drop_table_p1 = L"DROP table if exists user1.parent";
			SQLWCHAR        *grant_parent_to_public[] = { L"parent", L"t1", L"t2", L"code" };
			SQLWCHAR		*p;
			int				i;

			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLDriverConnect(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=dba;pwd=;db_name=demodb;charset=utf-8", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);
			retcode = SQLExecDirect(hstmt, drop_table_t1, SQL_NTS);
			retcode = SQLExecDirect(hstmt, drop_table_t2, SQL_NTS);
			retcode = SQLExecDirect(hstmt, drop_table_t3, SQL_NTS);
			retcode = SQLExecDirect(hstmt, drop_table_p1, SQL_NTS);

			Logger::WriteMessage("Phase1: ********** CREATE USER **********");
			retcode = SQLExecDirect(hstmt, qry_drop_user, SQL_NTS);
			retcode = SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
			retcode = SQLExecDirect(hstmt, qry_create_user, SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLFreeStmt(hstmt, SQL_CLOSE);
			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);

			Logger::WriteMessage("Phase2: ********** Try to LOGIN AS 'user1' **********");
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);
			retcode = SQLDriverConnect(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=user1;pwd=1234;db_name=demodb;charset=utf-8;omit_schema=yes", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);
			retcode = SQLExecDirect(hstmt, qry_create_table0, SQL_NTS);
			retcode = SQLExecDirect(hstmt, qry_create_table1, SQL_NTS);
			retcode = SQLExecDirect(hstmt, qry_create_table2, SQL_NTS);
			retcode = SQLExecDirect(hstmt, qry_create_table3, SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			for (i = 0; i < sizeof(grant_parent_to_public) / sizeof(SQLWCHAR *); i++)
			{
				SQLWCHAR query[512];

				wsprintf(query, L"GRANT ALL PRIVILEGES on %s to public", grant_parent_to_public[i]);
				retcode = SQLExecDirect(hstmt, query, SQL_NTS);
			}

			Logger::WriteMessage("Phase3: ********** Tables were created **********");

			Logger::WriteMessage("Phase4: ********** Search for table names owned by the login user  **********");
			retcode = SQLTables(hstmt, NULL, 0, NULL, 0, NULL, 0, L"TABLE", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			
			while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) {
				retcode = SQLGetData(hstmt, 3, SQL_C_CHAR, table_name, sizeof(table_name), &StrLen_or_IndPtr);
				strcpy(my_tables[num_table++], (const char *) table_name);
				sprintf(msg, "Table = %s", table_name);
				if (StrLen_or_IndPtr != NULL)
				{
					Logger::WriteMessage(msg);
				}
			}
			sprintf(msg, "Num Tables = %d\n", num_table);
			Logger::WriteMessage(msg);

			/* user 'user1' have to get 4 table names, t1, t2, code, parent, only	*/
			/* even it has privilege on tables owned by the user 'public' (total 14 tables */

			Assert::AreEqual(num_table, 4);

			Logger::WriteMessage("Phase4: ********** get colulm of user1.code  **********");

			retcode = SQLColumns(hstmt, NULL, 0, NULL, 0, L"code", SQL_NTS, NULL, 0);

			i = 0;
			while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) {
				retcode = SQLGetData(hstmt, 4, SQL_C_CHAR, column_name, sizeof(column_name), &StrLen_or_IndPtr);
				sprintf(msg, "column name [%d] = %s", ++i, column_name);
				Logger::WriteMessage(msg);
			}

			sprintf(msg, "Num Cols = %d", i);
			Logger::WriteMessage(msg);

			retcode = SQLFreeStmt(hstmt, SQL_CLOSE);
			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);

			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);
			retcode = SQLDriverConnect(hDbc, NULL, L"DRIVER=CUBRID Driver Unicode;server=test-db-server;port=33000;uid=public;pwd=;db_name=demodb;charset=utf-8;omit_schema=yes", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hstmt);

			/* Get foreign key table names regarding parent table 'parent' */
			retcode = SQLForeignKeysW(hstmt, NULL, 0, NULL, 0, L"user1.parent", SQL_NTS, NULL, 0, NULL, 0, NULL, 0);
			Assert::AreEqual((int)retcode, SQL_ERROR);

			i = 0;
			sprintf(msg, "List FK tables");
			Logger::WriteMessage(msg);

			while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) {
				retcode = SQLGetData(hstmt, 7, SQL_C_CHAR, table_name, sizeof(table_name), &StrLen_or_IndPtr);
				sprintf(msg, "FK table_name [%d] = %s", ++i, table_name);
				Logger::WriteMessage(msg);
			}

			retcode = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

		}

		TEST_METHOD(APIS_794_QueryPlanMultiByte)
		{
			RETCODE retcode;

			SQLWCHAR query_plan[32768] = { 0, };

			SQLHENV env;
			SQLHDBC dbc;
			SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SWORD plm_pcbErrorMsg = 0;
			SQLINTEGER diag_rec;

			SQLLEN len;
			
			/* Allocate an environment handle */
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
			/* We want ODBC 3 support */
			SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			/* Allocate a connection handle */
			SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

			//retcode = SQLDriverConnect(dbc, NULL, L"DSN=CUBRID Driver Unicode;DB_NAME=demodb;SERVER=test-db-server;PORT=33000;USER=dba;PWD=;CHARSET=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			retcode = SQLConnect(dbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);

			if (retcode == SQL_ERROR) {
				SQLGetDiagField(SQL_HANDLE_DBC, dbc, 0, SQL_DIAG_NUMBER, &diag_rec, 0, &plm_pcbErrorMsg);
			}

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			// Allocate statement handle and execute a query
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hStmt);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLExecDirect(hStmt, L"DROP TABLE IF EXISTS [테이블] ", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLExecDirect(hStmt, L"CREATE TABLE [테이블] ([이름] varchar(16), [나이] integer)", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLExecDirect(hStmt, L"INSERT INTO [테이블] VALUES ('홍길동', 25)", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLExecDirect(hStmt, L"SELECT [이름], [나이] FROM [테이블] WHERE [나이] > 19", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLFetch(hStmt);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			//MessageBox(NULL, L"Contents", L"Title", MB_OK);

			// Get Query Plan
			retcode = SQLGetData(hStmt, 1, SQL_C_DEFAULT, (SQLPOINTER)query_plan, 0, &len);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			if (retcode == SQL_SUCCESS_WITH_INFO) {
				retcode = SQLGetData(hStmt, 1, SQL_C_DEFAULT, (SQLPOINTER)query_plan, len, &len);
			}

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			if (strlen((const char *)query_plan) > 0) {
				wchar_t expected_window[198] = L"Join graph segments (f indicates final):\r\nseg[0]: [0]\r\nseg[1]: 이름[0] (f)\r\nseg[2]: 나이[0] (f)\r\nJoin graph nodes:\r\nnode[0]: 테이블 테이블(1/1) (sargs 0) (loc 0)\r\nJoin graph terms:\r\nterm[0]: [테이블].[나이] range";
				wchar_t expected_linux[198] = L"Join graph segments (f indicates final):\nseg[0]: [0]\nseg[1]: 이름[0] (f)\nseg[2]: 나이[0] (f)\nJoin graph nodes:\nnode[0]: 테이블 테이블(1/1) (sargs 0) (loc 0)\nJoin graph terms:\nterm[0]: [테이블].[나이] range";
				
				//for windows
				int c = wcsncmp(query_plan, expected_window, wcslen(expected_window));
				if (c != 0) {
					// for linux
					c = wcsncmp(query_plan, expected_linux, wcslen(expected_linux));
				}
				if (c != 0) {
					wchar_t* wc = (wchar_t*)query_plan;
					std::wstring ws(wc);

					//for under version to 9.2.x
					if (((int)ws.find(L"테이블")) > 0 && ((int)ws.find(L"이름")) > 0 && ((int)ws.find(L"나이")) > 0){
						c = 0;
					}
				}
				Assert::AreEqual(c, 0);
			}
			else {
				Assert::Fail(L"Query Plan is null");
			}
			// Clean up.   
			SQLDisconnect(dbc);
			SQLFreeHandle(SQL_HANDLE_DBC, dbc);
			SQLFreeHandle(SQL_HANDLE_ENV, env);
		}

		TEST_METHOD(APIS_901_QueryPlan)
		{
			RETCODE retcode;

			SQLWCHAR query_plan[32768] = { 0, };

			SQLHENV env;
			SQLHDBC dbc;
			SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SWORD plm_pcbErrorMsg = 0;
			SQLINTEGER diag_rec;

			SQLLEN len;

			/* Allocate an environment handle */
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
			/* We want ODBC 3 support */
			SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			/* Allocate a connection handle */
			SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

			//retcode = SQLDriverConnect(dbc, NULL, L"DSN=CUBRID Driver Unicode;DB_NAME=demodb;SERVER=192.168.2.33;PORT=33000;USER=dba;PWD=;CHARSET=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			retcode = SQLConnect(dbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);

			if (retcode == SQL_ERROR) {
				SQLGetDiagField(SQL_HANDLE_DBC, dbc, 0, SQL_DIAG_NUMBER, &diag_rec, 0, &plm_pcbErrorMsg);
			}

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			// Allocate statement handle and execute a query
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hStmt);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLPrepare(hStmt, L"@QE@", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLExecute(hStmt);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLPrepare(hStmt, L"select * from public.code", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLExecute(hStmt);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLFetch(hStmt);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			// Get Query Plan
			retcode = SQLGetData(hStmt, 1, SQL_C_DEFAULT, (SQLPOINTER)query_plan, 0, &len);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			if (retcode == SQL_SUCCESS_WITH_INFO) {
				retcode = SQLGetData(hStmt, 1, SQL_C_DEFAULT, (SQLPOINTER)query_plan, len, &len);
			}

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			if (strlen((const char *)query_plan) > 0) {
				int c = strncmp((const char *)query_plan, (const char *)L"Join graph", 10);
				Assert::AreEqual(c, 0);
			}
			else {
				Assert::Fail(L"Query Plan is null");
			}
			// Clean up.
			SQLDisconnect(dbc);
			SQLFreeHandle(SQL_HANDLE_DBC, dbc);
			SQLFreeHandle(SQL_HANDLE_ENV, env);

		}

		TEST_METHOD(APIS_875_NullAble)
		{

#define MAX_COL_NAME_LEN  256

			RETCODE retcode;
			SQLCHAR name[MAX_COL_NAME_LEN];
			SQLSMALLINT name_len;
			SQLSMALLINT    data_type;
			SQLULEN        data_size;
			SQLSMALLINT    num_digits;
			SQLSMALLINT    nullable;


			SQLHENV env;
			SQLHDBC dbc;
			SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SWORD plm_pcbErrorMsg = 0;
			SQLINTEGER diag_rec;

			SQLLEN len;

			/* Allocate an environment handle */
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
			/* We want ODBC 3 support */
			SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			/* Allocate a connection handle */
			SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

			//retcode = SQLDriverConnect(dbc, NULL, L"DSN=CUBRID Driver Unicode;DB_NAME=demodb;SERVER=192.168.2.33;PORT=33000;USER=public;PWD=;CHARSET=utf-8;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			retcode = SQLConnect(dbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);

			if (retcode == SQL_ERROR) {
				SQLGetDiagField(SQL_HANDLE_DBC, dbc, 0, SQL_DIAG_NUMBER, &diag_rec, 0, &plm_pcbErrorMsg);
			}

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			// Allocate statement handle and execute a query
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hStmt);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLPrepare(hStmt, L"select * from public.athlete", SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLDescribeCol(
				hStmt,              // Select Statement (Prepared)
				1,                  // col1 (code): NOT NULL
				(SQLWCHAR *)name,               // Column Name (returned)
				MAX_COL_NAME_LEN,   // size of Column Name buffer
				&name_len,          // Actual size of column name
				&data_type,         // SQL Data type of column
				&data_size,         // Data size of column in table
				&num_digits,        // Number of decimal digits
				&nullable);         // Whether column nullable
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			Assert::AreEqual((int)nullable, SQL_NO_NULLS);

			retcode = SQLDescribeCol(
				hStmt,              // Select Statement (Prepared)
				3,                  // col23 (gender): NULLABLE
				(SQLWCHAR *)name,               // Column Name (returned)
				MAX_COL_NAME_LEN,   // size of Column Name buffer
				&name_len,          // Actual size of column name
				&data_type,         // SQL Data type of column
				&data_size,         // Data size of column in table
				&num_digits,        // Number of decimal digits
				&nullable);         // Whether column nullable
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			Assert::AreEqual((int)nullable, SQL_NULLABLE);

			// Clean up.
			SQLDisconnect(dbc);
			SQLFreeHandle(SQL_HANDLE_DBC, dbc);
			SQLFreeHandle(SQL_HANDLE_ENV, env);


		}

		TEST_METHOD(APIS_876_BindNull)
		{
			SQLHENV		hEnv;
			SQLHDBC		hDbc;
			SQLHSTMT	hStmt;

			RETCODE retcode(0);

			// DB Connection

			retcode = SQLAllocEnv(&hEnv);
			retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
			retcode = SQLAllocConnect(hEnv, &hDbc);
			retcode = SQLConnect(hDbc, L"CUBRID Driver Unicode", SQL_NTS, L"dba", SQL_NTS, NULL, SQL_NTS);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

			retcode = SQLExecDirect(hStmt, L"drop table if exists apis876", SQL_NTS);
			retcode = SQLExecDirect(hStmt, L"create table apis876 (col1 varchar (2), col2 varchar(20))", SQL_NTS);

			retcode = SQLPrepare(hStmt, (SQLWCHAR*)(wchar_t*)L"INSERT INTO apis876(col1, col2) VALUES(?, ?)", SQL_NTS);

			retcode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR,
				2, 0, NULL, 2, NULL);
			retcode = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR,
				20, 0, L"test", 4, NULL);

			retcode = SQLExecute(hStmt);

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			// Release Connection
			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			SQLFreeStmt(hStmt, SQL_DROP);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

	};
}
