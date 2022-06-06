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

			retcode = SQLDriverConnect(dbc, NULL, L"DSN=CUBRID Driver Unicode;DB_NAME=demodb;SERVER=192.168.2.32;PORT=33000;USER=dba;PWD=;CHARSET=utf-8;AUTOCOMMIT=ON", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (retcode == SQL_ERROR) {
				SQLGetDiagField(SQL_HANDLE_DBC, dbc, 0, SQL_DIAG_NUMBER, &diag_rec, 0, &plm_pcbErrorMsg);
			}

			retcode = SQLGetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, NULL);
			Assert::AreNotEqual((int)retcode, SQL_ERROR);
			Assert::AreEqual((int)autocommit, 1);
		}

		TEST_METHOD(QueryPlan)
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

		TEST_METHOD(QueryPlanMultiByte)
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

		TEST_METHOD(QueryPlanAPIS901)
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

		TEST_METHOD(NullAble_APIS875)
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
			SQLLEN pIndicator[8];	// StrLen_or_IndPtr array is required but values ​​are meaningless

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

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			wchar_t address[ARRAY_SIZE][STR_SZ] = { L"한1ab", L"ab", L"123", L"6234", L"9", L"db", L"Abc", L"Bbcd" };

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

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			retcode = SQLExecute(hStmt);

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			ppBuf = pBufData;
			for (int i = 0; i < ARRAY_SIZE; i++)	// insert additional rows
			{
				swprintf(address[i], L"1-한글 %d", i);
				memcpy(ppBuf, address[i], STR_SZ * sizeof(wchar_t));
				ppBuf = ppBuf + nMaxLen;
			}

			retcode = SQLExecute(hStmt);

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			ppBuf = pBufData;
			for (int i = 0; i < ARRAY_SIZE; i++)	// insert additional rows
			{
				swprintf(address[i], L"2-한글 %d", i);
				memcpy(ppBuf, address[i], STR_SZ * sizeof(wchar_t));
				ppBuf = ppBuf + nMaxLen;
			}

			retcode = SQLExecute(hStmt);

			Assert::AreNotEqual((int)retcode, SQL_ERROR);

			// 접속 해제 
			retcode = SQLEndTran(SQL_HANDLE_ENV, hEnv, SQL_COMMIT);
			SQLFreeStmt(hStmt, SQL_DROP);
			retcode = SQLDisconnect(hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			retcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

	};
}