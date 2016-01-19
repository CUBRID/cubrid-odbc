using System;
using System.Text;
using System.Data;
using System.Data.Odbc;
using System.Data.Common;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace UnitTest
{
    public partial class TestCases
    {
        [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLGetDescField", CharSet = CharSet.Ansi)]
        public static extern short SQLGetDescField(
                    ref st_odbc_desc desc,
                    short RecNumber,
                    short FieldIdentifier,
                    StringBuilder Value,
	                int BufferLength, StringBuilder StrLen_or_Ind);

        /// <summary>
        /// Test OdbcTransaction class
        /// </summary>
        /// 
        public static void Test_GetDescField()
        {
            StringBuilder sb_length = new StringBuilder(16);
            StringBuilder sb_value = new StringBuilder(16);
            st_odbc_desc odbc_descFiel = new st_odbc_desc();

            st_odbc_record record = new st_odbc_record();
            record.record_number = 1;
            record.datetime_interval_code = 101;

            //将托管内存转换为非托管内存
            MarshalManage mb = new MarshalManage(record);
            odbc_descFiel.records = mb.buffer;

            odbc_descFiel.alloc_type = 1;
            odbc_descFiel.array_size = 2;
            odbc_descFiel.bind_type = 3;
            odbc_descFiel.fetched_size = 5;
            odbc_descFiel.handle_type=6;
            odbc_descFiel.max_count = 7;
         
            //调用非托管方法
            //SQL_DESC_ALLOC_TYPE 1099
            SQLGetDescField(ref odbc_descFiel, 1, 1099, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 1);

            //SQL_DESC_ARRAY_SIZE 20
            SQLGetDescField(ref odbc_descFiel, 1, 20, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 2);

            //SQL_DESC_ARRAY_STATUS_PTR 21
            SQLGetDescField(ref odbc_descFiel, 1, 21, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_BIND_OFFSET_PTR 24
            SQLGetDescField(ref odbc_descFiel, 1, 24, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_BIND_TYPE 25
            SQLGetDescField(ref odbc_descFiel, 1, 25, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 3);

            //SQL_DESC_CASE_SENSITIVE 12
            SQLGetDescField(ref odbc_descFiel, 1, 12, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_DATETIME_INTERVAL_CODE 1007
            SQLGetDescField(ref odbc_descFiel, 1, 1007, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 101);

            //SQL_DESC_DATETIME_INTERVAL_PRECISION 26
            SQLGetDescField(ref odbc_descFiel, 1, 26, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_DISPLAY_SIZE 6
            SQLGetDescField(ref odbc_descFiel, 1, 6, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_FIXED_PREC_SCALE 9
            SQLGetDescField(ref odbc_descFiel, 1, 9, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_COLUMN_LENGTH 3
            SQLGetDescField(ref odbc_descFiel, 1, 3, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_LITERAL_PREFIX 27
            SQLGetDescField(ref odbc_descFiel, 1, 27, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_LITERAL_SUFFIX 28
            SQLGetDescField(ref odbc_descFiel, 1, 28, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_LOCAL_TYPE_NAME 29
            SQLGetDescField(ref odbc_descFiel, 1, 29, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_NUM_PREC_RADIX 32
            SQLGetDescField(ref odbc_descFiel, 1, 32, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_PARAMETER_TYPE 33
            SQLGetDescField(ref odbc_descFiel, 1, 33, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_SEARCHABLE 13
            SQLGetDescField(ref odbc_descFiel, 1, 13, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_TYPE 1002
            SQLGetDescField(ref odbc_descFiel, 1, 1002, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_UNNAMED 1012
            SQLGetDescField(ref odbc_descFiel, 1, 1012, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);

            //SQL_DESC_ROWVER 35
            SQLGetDescField(ref odbc_descFiel, 1, 35, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 0);
        }

        [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLGetDiagField", CharSet = CharSet.Ansi)]
        public static extern short SQLGetDiagField_s(short HandleType,
            ref st_odbc_statement env,
            short RecNumber,
            short DiagIdentifier,
            StringBuilder DiagInfo,
            short BufferLength, StringBuilder StringLength);

        [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLGetDiagField", CharSet = CharSet.Ansi)]
        public static extern short SQLGetDiagField_e(short HandleType,
            ref st_odbc_env env,
            short RecNumber,
            short DiagIdentifier,
            StringBuilder DiagInfo,
            short BufferLength, StringBuilder StringLength);

        public static void Test_GetDiagField()
        {
            StringBuilder sb_length = new StringBuilder(64);
            StringBuilder sb_diagInfo = new StringBuilder(64);
            
            //env
            st_odbc_env odbc_env = new st_odbc_env();

            st_diag_record record = new st_diag_record();
            record.number = 1;
            record.sql_state = "IM";
            record.native_code = 1;
            MarshalManage mb_record = new MarshalManage(record);

            st_diag diag = new st_diag();
            diag.record = mb_record.buffer;

            MarshalManage mb_diag = new MarshalManage(diag);
            odbc_env.diag = mb_diag.buffer;

            //statement
            st_odbc_statement statement = new st_odbc_statement();
            statement.current_tpl_pos = 1;
            st_diag_record record_s = new st_diag_record();
            record_s.number = 1;
            record_s.sql_state = "IM";
            record_s.native_code = 1;
            MarshalManage mb_record_s = new MarshalManage(record_s);

            st_diag diag_s = new st_diag();
            diag_s.record = mb_record_s.buffer;
            diag_s.rec_number = 1;
            diag_s.retcode = 2;
            MarshalManage mb_diag_s = new MarshalManage(diag_s);
            statement.diag = mb_diag_s.buffer;

            int res=0;
            //SQL_DIAG_ROW_COUNT 3         
            res = SQLGetDiagField_s(1, ref statement, 1, 3, sb_diagInfo, 16, sb_length);
            Assert.IsTrue(res == -1);

            //SQL_DIAG_CURSOR_ROW_COUNT -1249
            res = SQLGetDiagField_s(1, ref statement, 1, -1249, sb_diagInfo, 16, sb_length);
            Assert.IsTrue(res == -1);

            //SQL_DIAG_NUMBER 2
            res = SQLGetDiagField_s(1, ref statement, 1, 1, sb_diagInfo, 16, sb_length);
            Assert.IsTrue(sb_diagInfo.ToString()[0] == 2);

            //SQL_DIAG_RETURNCODE 1
            res = SQLGetDiagField_s(1, ref statement, 1,2, sb_diagInfo, 16, sb_length);
            Assert.IsTrue(sb_diagInfo.ToString()[0] == 1);

            //SQL_DIAG_CLASS_ORIGIN 8
            res = SQLGetDiagField_s(1, ref statement, 1, 8, sb_diagInfo, 0, sb_length);
            Assert.IsTrue(sb_diagInfo.ToString()[0] == 1);

            //SQL_DIAG_COLUMN_NUMBER -1247
            res = SQLGetDiagField_s(1, ref statement, 1, -1247, sb_diagInfo, 16, sb_length);
            Assert.IsTrue(sb_diagInfo.ToString()[0] == 63);

            //SQL_DIAG_CONNECTION_NAME 10
            res = SQLGetDiagField_s(1, ref statement, 1, 10, sb_diagInfo, 16, sb_length);
            Assert.IsTrue(res == 0);
            res = SQLGetDiagField_s(1, ref statement, 1, 10, sb_diagInfo, 0, sb_length);

            //SQL_DIAG_MESSAGE_TEXT 6
            res = SQLGetDiagField_s(1, ref statement, 1, 6, sb_diagInfo, 16, sb_length);
            Assert.IsTrue(sb_diagInfo.ToString()[0] == 91);
            res = SQLGetDiagField_s(1, ref statement, 1, 6, sb_diagInfo, 0, sb_length);

            //SQL_DIAG_NATIVE 5
            res = SQLGetDiagField_s(1, ref statement, 1, 5, sb_diagInfo, 16, sb_length);
            Assert.IsTrue(sb_diagInfo.ToString()[0] == 1);

            //SQL_DIAG_ROW_NUMBER -1248
            res = SQLGetDiagField_s(1, ref statement, 1, -1248, sb_diagInfo, 16, sb_length);
            Assert.IsTrue(sb_diagInfo.ToString()[0] == 63);

            //SQL_DIAG_SERVER_NAME 11
            res = SQLGetDiagField_s(1, ref statement, 1, 11, sb_diagInfo, 0, sb_length);
            Assert.IsTrue(sb_diagInfo.ToString()[0] == 63);
        }

        [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLSetConnectAttr", CharSet = CharSet.Ansi)]
        public static extern short SQLSetConnectAttr(ref st_odbc_connection ConnectionHandle,
           int Attribute,
           int Value, int StringLength);
        public static void Test_SetConnectAttr()
        {
            //st_odbc_connection
            st_odbc_connection conn = new st_odbc_connection();
            //SQL_ACCESS_MODE 101

            //SQL_MODE_READ_ONLY
            int res = SQLSetConnectAttr(ref conn, 101, 1, 0);
            Assert.IsTrue(conn.attr_txn_isolation == 2);

            //SQL_MODE_READ_WRITE
            res = SQLSetConnectAttr(ref conn, 101, 0, 0);
            Assert.IsTrue(conn.attr_txn_isolation == conn.old_txn_isolation);

            // default:error
            SQLSetConnectAttr(ref conn, 101, 2, 0);
            Assert.IsTrue(conn.attr_txn_isolation == 0);

            //SQL_ATTR_ASYNC_ENABLE 4
            res = SQLSetConnectAttr(ref conn, 4, 2, 0);
            Assert.IsTrue(conn.attr_async_enable == 0);

            res = SQLSetConnectAttr(ref conn, 4, 1, 0);
            Assert.IsTrue(conn.attr_async_enable == 1);

            //SQL_ATTR_AUTO_IPD 10001
            res = SQLSetConnectAttr(ref conn, 10001, 2, 0);
            Assert.IsTrue(conn.attr_async_enable == 1);

            //SQL_ATTR_AUTOCOMMIT 102
            res = SQLSetConnectAttr(ref conn, 102, 2, 0);
            Assert.IsTrue(conn.attr_autocommit == 0);

            //SQL_ATTR_CONNECTION_DEAD 1209 not support
            res = SQLSetConnectAttr(ref conn, 102, -1, 0);

            //SQL_ATTR_CONNECTION_TIMEOUT 113
            res = SQLSetConnectAttr(ref conn, 113, -1, 0);
            Assert.IsTrue(conn.attr_connection_timeout == 0);

            res = SQLSetConnectAttr(ref conn, 113, 1000, 0);
            Assert.IsTrue(conn.attr_connection_timeout == 1000);

            //SQL_ATTR_CURRENT_CATALOG 109
            res = SQLSetConnectAttr(ref conn, 109, 0, 0);
            Assert.IsTrue(conn.attr_current_catalog == null);

            //SQL_ATTR_LOGIN_TIMEOUT 103
            res = SQLSetConnectAttr(ref conn, 103, -1, 0);
            Assert.IsTrue(conn.attr_login_timeout == 0);

            //SQL_ATTR_METADATA_ID 10014
            SQLSetConnectAttr(ref conn, 10014, 2, 0);
            Assert.IsTrue(conn.attr_metadata_id == 0);

            res = SQLSetConnectAttr(ref conn, 10014, 1, 0);
            Assert.IsTrue(conn.attr_metadata_id == 1);

            //SQL_ATTR_ODBC_CURSORS 110
            SQLSetConnectAttr(ref conn, 110, 3, 0);
            Assert.IsTrue(conn.attr_odbc_cursors == 0);

            res = SQLSetConnectAttr(ref conn, 110, 2, 0);
            Assert.IsTrue(conn.attr_odbc_cursors == 2);

            //SQL_ATTR_PACKET_SIZE 112
            res = SQLSetConnectAttr(ref conn, 112, -1, 0);
            Assert.IsTrue(conn.attr_packet_size == 0);

            res = SQLSetConnectAttr(ref conn, 112, 1, 0);
            Assert.IsTrue(conn.attr_packet_size == 1);

            //SQL_ATTR_QUIET_MODE 111
            res = SQLSetConnectAttr(ref conn, 111, 10, 0);
            Assert.IsTrue(conn.attr_quiet_mode == (IntPtr)10);

            //SQL_ATTR_TRACE 104
            SQLSetConnectAttr(ref conn, 104, 2, 0);
            Assert.IsTrue(conn.attr_trace == 0);

            res = SQLSetConnectAttr(ref conn, 104, 1, 0);
            Assert.IsTrue(conn.attr_trace == 1);

            //SQL_ATTR_TRACEFILE 105
            res = SQLSetConnectAttr(ref conn, 105, 0, 0);
            Assert.IsTrue(conn.attr_tracefile == null);

            //SQL_ATTR_TRANSLATE_OPTION 107 not support
            res = SQLSetConnectAttr(ref conn, 107, 0, 0);

            //SQL_ATTR_MAX_ROWS 1
            res = SQLSetConnectAttr(ref conn, 1, 1, 0);
            Assert.IsTrue(conn.attr_trace == 1);

            //SQL_ATTR_QUERY_TIMEOUT 0
            res = SQLSetConnectAttr(ref conn, 0, 1, 0);
            Assert.IsTrue(conn.attr_trace == 1);
            res = SQLSetConnectAttr(ref conn, -1, 1, 0);
        }

        [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLGetConnectAttr", CharSet = CharSet.Ansi)]
        public static extern short SQLGetConnectAttr(ref st_odbc_connection ConnectionHandle,
           int Attribute,
           StringBuilder Value, Int32 BufferLength, StringBuilder StringLength);
        public static void Test_GetConnectAttr()
        {
            //st_odbc_connection
            st_odbc_connection conn = new st_odbc_connection();
            StringBuilder sb_length = new StringBuilder(64);
            StringBuilder sb_value = new StringBuilder(64);

            conn.attr_access_mode = 1;
            conn.attr_async_enable = 2;
            conn.attr_autocommit = 1;

            //SQL_ATTR_ACCESS_MODE 101
            int res = SQLGetConnectAttr(ref conn, 101, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 1);

            //SQL_ATTR_ASYNC_ENABLE 4
            res = SQLGetConnectAttr(ref conn, 4, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 2);

            //SQL_ATTR_AUTO_IPD 10001 not support
            res = SQLGetConnectAttr(ref conn, 10001, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 1);

            //SQL_ATTR_AUTOCOMMIT 102 
            res = SQLGetConnectAttr(ref conn, 102, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 1);

            //SQL_ATTR_CONNECTION_DEAD 1209
            conn.connhd = 1; //true false
            res = SQLGetConnectAttr(ref conn, 1209, sb_value, 16, sb_length);
            Assert.IsTrue(sb_length.Length == 1);

            conn.connhd = -2; //true false
            res = SQLGetConnectAttr(ref conn, 1209, sb_value, 16, sb_length);
            Assert.IsTrue(sb_length.Length == 1);

            //SQL_ATTR_CONNECTION_TIMEOUT 113 not support
            res = SQLGetConnectAttr(ref conn, 113, sb_value, 16, sb_length);
            Assert.IsTrue(sb_length.Length == 1);

            //SQL_ATTR_CURRENT_CATALOG  109
            res = SQLGetConnectAttr(ref conn, 4, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 2);
            res = SQLGetConnectAttr(ref conn, 4, sb_value, 0, sb_length);

            //SQL_ATTR_LOGIN_TIMEOUT 103 not support
            res = SQLGetConnectAttr(ref conn, 103, sb_value, 16, sb_length);
            Assert.IsTrue(sb_length.Length == 1);

            //SQL_ATTR_METADATA_ID 10014
            conn.attr_metadata_id = 3;
            res = SQLGetConnectAttr(ref conn, 10014, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 3);

            //SQL_ATTR_ODBC_CURSORS 110 not support
            res = SQLGetConnectAttr(ref conn, 110, sb_value, 16, sb_length);
            Assert.IsTrue(sb_length.Length == 1);

            //SQL_ATTR_PACKET_SIZE 112 not suppor
            res = SQLGetConnectAttr(ref conn, 112, sb_value, 16, sb_length);
            Assert.IsTrue(sb_length.Length == 1);

            //SQL_ATTR_QUIET_MODE 111
            res = SQLGetConnectAttr(ref conn, 111, sb_value, 16, sb_length);
            Assert.IsTrue(sb_length.Length == 1);

            //SQL_ATTR_TRACE 104
            conn.attr_trace = 5;
            res = SQLGetConnectAttr(ref conn, 104, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 5);

            //SQL_ATTR_TRACEFILE 105
            res = SQLGetConnectAttr(ref conn, 105, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 1);
            res = SQLGetConnectAttr(ref conn, 105, sb_value, 0, sb_length);

            //SQL_ATTR_TRANSLATE_LIB 106 not suppor
            res = SQLGetConnectAttr(ref conn, 106, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 1);

            //SQL_ATTR_TRANSLATE_OPTION 107 not suppor
            res = SQLGetConnectAttr(ref conn, 107, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.Length == 1);

            //SQL_ATTR_TXN_ISOLATION 108
            conn.attr_txn_isolation = 6;
            res = SQLGetConnectAttr(ref conn, 108, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 6);

            //SQL_ATTR_MAX_ROWS 1
            conn.attr_max_rows = 10;
            res = SQLGetConnectAttr(ref conn, 1, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 10);

            //SQL_ATTR_QUERY_TIMEOUT 0
            conn.attr_query_timeout = 30;
            res = SQLGetConnectAttr(ref conn, 0, sb_value, 16, sb_length);
            Assert.IsTrue(sb_value.ToString()[0] == 30);

            //default
            res = SQLGetConnectAttr(ref conn, -100, sb_value, 16, sb_length);
            //Assert.IsTrue(sb_length.Length == 0);
        }

        [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLGetStmtAttr", CharSet = CharSet.Ansi)]
        public static extern short SQLGetStmtAttr(ref st_odbc_statement ConnectionHandle,
           int Attribute,
           StringBuilder Value, Int32 BufferLength, IntPtr StringLength);
        public static void Test_GetStmtAttr()
        {
            MarshalManage sb_length = new MarshalManage(64);
            StringBuilder sb_value = new StringBuilder(64);

            st_odbc_desc desc = new st_odbc_desc();
            MarshalManage mb_desc = new MarshalManage(desc);
            st_odbc_statement statement = new st_odbc_statement();
            int res = 0,error=-1,success=0;

            //value == null
            res = SQLGetStmtAttr(ref statement, 1, null, 64, sb_length.buffer);
            Assert.IsTrue(res == error);
            //default
            res = SQLGetStmtAttr(ref statement, -10000, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == error);

            //SQL_ATTR_APP_PARAM_DESC 10011
            res = SQLGetStmtAttr(ref statement, 10011, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);
            //SQL_ATTR_APP_ROW_DESC 10010
            res = SQLGetStmtAttr(ref statement, 10010, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_IMP_PARAM_DESC 10013
            res = SQLGetStmtAttr(ref statement, 10013, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_IMP_ROW_DESC 10012
            res = SQLGetStmtAttr(ref statement, 10012, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            statement.apd = mb_desc.buffer;
            statement.ipd = mb_desc.buffer;
            statement.ard = mb_desc.buffer;
            statement.ird = mb_desc.buffer;

            //SQL_ATTR_ASYNC_ENABLE 4
            statement.attr_async_enable = 1;
            res = SQLGetStmtAttr(ref statement, 4, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(sb_value.ToString()[0] == 1);

            //SQL_ATTR_CURSOR_SCROLLABLE -1
            statement.attr_cursor_scrollable = 2;
            res = SQLGetStmtAttr(ref statement, -1, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(sb_value.ToString()[0] == 2);

            //SQL_ATTR_METADATA_ID 10014
            statement.attr_metadata_id = 3;
            res = SQLGetStmtAttr(ref statement, 10014, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(sb_value.ToString()[0] == 3);

            //SQL_ATTR_PARAM_BIND_OFFSET_PTR 17
            res = SQLGetStmtAttr(ref statement, 17, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_PARAM_BIND_TYPE 18
            res = SQLGetStmtAttr(ref statement, 18, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_PARAM_OPERATION_PTR 19
            res = SQLGetStmtAttr(ref statement, 19, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_PARAM_STATUS_PTR 20
            res = SQLGetStmtAttr(ref statement, 20, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_PARAMS_PROCESSED_PTR 21
            res = SQLGetStmtAttr(ref statement, 21, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_PARAMSET_SIZE 22
            res = SQLGetStmtAttr(ref statement, 22, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_RETRIEVE_DATA 11
            res = SQLGetStmtAttr(ref statement, 11, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(sb_value.ToString()[0] == 1);

            //SQL_ATTR_ROW_ARRAY_SIZE 27
            res = SQLGetStmtAttr(ref statement, 27, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_ROW_BIND_OFFSET_PTR 23
            res = SQLGetStmtAttr(ref statement, 23, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_ROW_BIND_TYPE 5
            res = SQLGetStmtAttr(ref statement, 5, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_ROW_STATUS_PTR 25
            res = SQLGetStmtAttr(ref statement, 25, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_ROW_OPERATION_PTR 24
            res = SQLGetStmtAttr(ref statement, 24, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_ROWS_FETCHED_PTR 26
            res = SQLGetStmtAttr(ref statement, 26, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_ROW_NUMBER 14
            statement.current_tpl_pos = 0;
            res = SQLGetStmtAttr(ref statement, 14, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            statement.current_tpl_pos = 1;
            res = SQLGetStmtAttr(ref statement, 14, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(sb_value.ToString()[0] == 1);

            //SQL_ATTR_CURSOR_TYPE 6
            statement.attr_cursor_type = 2;
            res = SQLGetStmtAttr(ref statement, 6, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(sb_value.ToString()[0] == 2);

            //SQL_ATTR_CONCURRENCY 7
            statement.attr_concurrency = 7;
            res = SQLGetStmtAttr(ref statement, 7, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(sb_value.ToString()[0] == 7);

            //SQL_ATTR_CURSOR_SENSITIVITY -2
            statement.attr_cursor_sensitivity = 2;
            res = SQLGetStmtAttr(ref statement, -2, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(sb_value.ToString()[0] == 2);

            //SQL_ATTR_ENABLE_AUTO_IPD 15
            res = SQLGetStmtAttr(ref statement, 15, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_FETCH_BOOKMARK_PTR 16
            res = SQLGetStmtAttr(ref statement, 16, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_KEYSET_SIZE 8
            res = SQLGetStmtAttr(ref statement, 8, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_MAX_LENGTH 3
            res = SQLGetStmtAttr(ref statement, 3, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_NOSCAN 2
            res = SQLGetStmtAttr(ref statement, 2, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);
            Assert.IsTrue(sb_value.ToString()[0] == 1); // on

            //SQL_ATTR_MAX_ROWS 1
            res = SQLGetStmtAttr(ref statement, 1, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_QUERY_TIMEOUT 0
            res = SQLGetStmtAttr(ref statement, 0, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_SIMULATE_CURSOR 10
            res = SQLGetStmtAttr(ref statement, 10, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(res == success);

            //SQL_ATTR_USE_BOOKMARKS 12
            statement.attr_use_bookmark = 12;
            res = SQLGetStmtAttr(ref statement, 12, sb_value, 64, sb_length.buffer);
            Assert.IsTrue(sb_value.ToString()[0] == 12);
        }

        [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLSetStmtAttr", CharSet = CharSet.Ansi)]
        public static extern short SQLSetStmtAttr(ref st_odbc_statement ConnectionHandle,
           int Attribute,
           Int32 Value, Int32 StringLength);
        public static void Test_SetStmtAttr()
        {
            Int32 length = 4;
            Int32 sb_value = 0;
            st_odbc_statement statement = new st_odbc_statement();
            int res = 0,success=0,attrid=0;

            //SQL_ATTR_APP_PARAM_DESC 10011
            statement.apd = new IntPtr(0);
            statement.i_apd = new IntPtr(1);
            Assert.IsTrue(statement.apd != statement.i_apd);
            sb_value = 2;
            res = SQLSetStmtAttr(ref statement, 10011, sb_value, length);
            Assert.IsTrue(statement.apd != statement.i_apd);

            statement.apd = new IntPtr(0);
            statement.i_apd = new IntPtr(1);
            sb_value = 0;
            res = SQLSetStmtAttr(ref statement, 10011, 0, 0);
            Assert.IsTrue(statement.apd == statement.i_apd);

            //SQL_ATTR_APP_ROW_DESC 10010
            statement.ard = new IntPtr(0);
            statement.i_ard = new IntPtr(1);
            sb_value = 2;
            Assert.IsTrue(statement.ard != statement.i_ard);
            res = SQLSetStmtAttr(ref statement, 10010, sb_value, length);
            Assert.IsTrue(statement.ard != statement.i_ard);

            statement.ard = new IntPtr(0);
            statement.i_ard = new IntPtr(1);
            sb_value = 0;
            res = SQLSetStmtAttr(ref statement, 10010, 0, 0);
            Assert.IsTrue(statement.ard == statement.i_ard);

            //SQL_ATTR_ASYNC_ENABLE 4
            sb_value = 0;
            res = SQLSetStmtAttr(ref statement, 4, sb_value, length);
            Assert.IsTrue(res == success);
            sb_value = 3;//error:default
            res = SQLSetStmtAttr(ref statement, 4, sb_value, length);
            Assert.IsTrue(res != success);

            //SQL_ATTR_CURSOR_SENSITIVITY -2
            attrid = -2;
            sb_value = 0;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 1;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 2;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 3;//error:default
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res != success);

            //SQL_ATTR_CURSOR_SCROLLABLE -1
            attrid = -1;
            sb_value = 0;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 1;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 3;//error:default
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res != success);

            //SQL_ATTR_CURSOR_TYPE 6
            attrid = 6;
            sb_value = 0;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 1;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 0;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 3;
            statement.attr_concurrency = 1;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            statement.attr_concurrency = 0;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 4;//error:default
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res != success);

            //SQL_ATTR_FETCH_BOOKMARK_PTR 16
            attrid = 16;
            sb_value = 0;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            //SQL_ATTR_IMP_PARAM_DESC 10013
            attrid = 10013;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res != success);

            // SQL_ATTR_IMP_ROW_DESC 10012
            attrid = 10012;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res != success);

            //SQL_ATTR_METADATA_ID 10014
            attrid = 10014;
            sb_value = 0;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 1;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            sb_value = 3;//error:default
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res != success);

            st_odbc_desc desc = new st_odbc_desc();
            MarshalManage mb_desc = new MarshalManage(desc);

            statement.apd = mb_desc.buffer;
            statement.ipd = mb_desc.buffer;
            statement.ard = mb_desc.buffer;
            statement.ird = mb_desc.buffer;

            //SQL_ATTR_PARAM_BIND_OFFSET_PTR 17
            attrid = 17;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            //SQL_ATTR_PARAM_BIND_TYPE 18
            attrid = 18;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);

            //SQL_ATTR_PARAM_OPERATION_PTR 19
            attrid = 19;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_PARAM_STATUS_PTR 20
            attrid = 20;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_PARAMS_PROCESSED_PTR 21
            attrid = 21;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_PARAMSET_SIZE 22
            attrid = 22;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_ROW_ARRAY_SIZE 27
            attrid = 27;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_ROW_BIND_OFFSET_PTR 23
            attrid = 23;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_ROW_BIND_TYPE 5
            attrid = 5;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_ROW_STATUS_PTR 25
            attrid = 25;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_ROW_OPERATION_PTR 24
            attrid = 24;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_ROWS_FETCHED_PTR 26
            attrid = 26;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res == success);
            //SQL_ATTR_ROW_NUMBER 14
            attrid = 14;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res != success);

            //default -10000
            attrid = -10000;
            res = SQLSetStmtAttr(ref statement, attrid, sb_value, length);
            Assert.IsTrue(res != success);
        }
    }
}