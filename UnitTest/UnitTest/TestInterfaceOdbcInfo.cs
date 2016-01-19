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
        /// <summary>
        /// Test OdbcTransaction class
        /// </summary>
        /// 
        public static void case_connInfo()
        {
            try
            {
                DataTable dtTemp = conn.GetSchema();//OK

                dtTemp = conn.GetSchema("MetaDataCollections");//
                Assert.IsTrue(dtTemp.TableName == "MetaDataCollections");
                dtTemp = conn.GetSchema("DataSourceInformation");//
                Assert.IsTrue(dtTemp.TableName == "DataSourceInformation");
                dtTemp = conn.GetSchema("DataTypes");//
                Assert.IsTrue(dtTemp.TableName == "DataTypes");
                dtTemp = conn.GetSchema("Restrictions");//
                Assert.IsTrue(dtTemp.TableName == "Restrictions");
                dtTemp = conn.GetSchema("ReservedWords");//
                Assert.IsTrue(dtTemp.TableName == "ReservedWords");
                dtTemp = conn.GetSchema("Columns");//
                Assert.IsTrue(dtTemp.TableName == "Columns");
                dtTemp = conn.GetSchema("Indexes");//
                Assert.IsTrue(dtTemp.TableName == "Indexes");
                dtTemp = conn.GetSchema("Procedures");//
                Assert.IsTrue(dtTemp.TableName == "Procedures");
                dtTemp = conn.GetSchema("ProcedureColumns");//
                Assert.IsTrue(dtTemp.TableName == "ProcedureColumns");
                dtTemp = conn.GetSchema("ProcedureParameters");//
                Assert.IsTrue(dtTemp.TableName == "ProcedureParameters");

                dtTemp = conn.GetSchema("Tables");
                Assert.IsTrue(dtTemp.TableName == "Tables");
                dtTemp = conn.GetSchema("Views");
                Assert.IsTrue(dtTemp.TableName == "Views");

                string[] restrictions = new string[4];
                restrictions[2] = "CODE";
                dtTemp = conn.GetSchema("Tables", restrictions);//
                Assert.IsTrue(dtTemp.TableName == "Tables");
            }
            catch (Exception err)
            {
                Console.WriteLine(err.Message);
                Console.WriteLine(err.StackTrace);
            }
        }
        public static void case_GetDataTypeName()
        {
            // Open Connection
            string strConn = connString;
            OdbcConnection connCubrid = conn;

            // Create a test table with 4 columns in various data type
            string testTable = "table_data_type";
            string strDropTable = string.Format("DROP TABLE {0}", testTable);
            string strCreateTable = string.Format(@"CREATE TABLE {0}(id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,col_1 INT, col_2 VARCHAR, col_3 CHAR,col_4 bit);", testTable);
            string strSqlInsert = string.Format("INSERT INTO {0}(col_1, col_2, col_3) VALUE(123, 'varchar contents', 'a')", testTable);

            // Execute multiple queries
            ExecuteMultiQueries(connCubrid, new string[] { strDropTable, strCreateTable });

            string strSqlSelect = string.Format("SELECT * FROM {0} ORDER BY id DESC;", testTable);
            OdbcDataReader odbcReader = CreateReader(connCubrid, strSqlSelect);

            // Check for GetDataTypeName returned values
            // Assert.AreEqual("INT", odbcReader.GetDataTypeName(0));
            // Assert.AreEqual("INT", odbcReader.GetDataTypeName(1));
            // Assert.AreEqual("VARCHAR", odbcReader.GetDataTypeName(2));
            // Assert.AreEqual("CHAR", odbcReader.GetDataTypeName(3));

            Console.WriteLine(odbcReader.GetDataTypeName(0));
            Console.WriteLine(odbcReader.GetDataTypeName(1));
            Console.WriteLine(odbcReader.GetDataTypeName(2));
            Console.WriteLine(odbcReader.GetDataTypeName(3));
        }

        private static void case_OdbcInfoNotSupport()
        {
            //SQL_POS_OPERATIONS 79 SQL_POSITIONED_STATEMENTS 80  SQL_CONCAT_NULL_BEHAVIOR 22
            //SQL_STATIC_SENSITIVITY 83 SQL_CATALOG_LOCATION 114  SQL_CONVERT_BIT 55
            //SQL_CATALOG_USAGE 92 SQL_CONVERT_INTERVAL_YEAR_MONTH 124 SQL_CURSOR_SENSITIVITY 10001
            //SQL_CONVERT_INTERVAL_DAY_TIME 123 SQL_CONVERT_TINYINT 68 SQL_FILE_USAGE 84
            //SQL_CREATE_ASSERTION 127 SQL_CREATE_CHARACTER_SET 128
            //SQL_CREATE_COLLATION 129 SQL_CREATE_DOMAIN 130
            //SQL_CREATE_SCHEMA 131 SQL_CREATE_TRANSLATION 133
            //SQL_DROP_ASSERTION 136 SQL_DROP_CHARACTER_SET 137
            //SQL_DROP_COLLATION 138 SQL_DROP_DOMAIN 139 
            //SQL_DROP_SCHEMA 140 SQL_DROP_TRANSLATION 142 
            //SQL_INFO_SCHEMA_VIEWS 149 SQL_KEYSET_CURSOR_ATTRIBUTES2 151 
            //SQL_INSERT_STATEMENT 172 SQL_KEYSET_CURSOR_ATTRIBUTES1 150 
            //SQL_MAX_ASYNC_CONCURRENT_STATEMENTS 10022 SQL_MAX_CATALOG_NAME_LEN 34 
            //SQL_MAX_COLUMNS_IN_GROUP_BY 97 SQL_MAX_CURSOR_NAME_LEN 31
            //SQL_MAX_COLUMNS_IN_INDEX 98 SQL_MAX_COLUMNS_IN_ORDER_BY 99 
            //SQL_MAX_COLUMNS_IN_SELECT 100 SQL_MAX_COLUMNS_IN_TABLE 101 
            //SQL_MAX_DRIVER_CONNECTIONS 0 SQL_MAX_INDEX_SIZE 102 
            //SQL_MAX_PROCEDURE_NAME_LEN 33 SQL_MAX_ROW_SIZE 104 
            //SQL_MAX_SCHEMA_NAME_LEN 32 SQL_MAX_STATEMENT_LEN 105 
            //SQL_MAX_TABLES_IN_SELECT 106 SQL_MAX_USER_NAME_LEN 107 
            //SQL_NUMERIC_FUNCTIONS 49 SQL_SCHEMA_USAGE 91 
            //SQL_SQL92_FOREIGN_KEY_DELETE_RULE 156 SQL_SQL92_FOREIGN_KEY_UPDATE_RULE 157 
            //SQL_TIMEDATE_ADD_INTERVALS 109 SQL_TIMEDATE_DIFF_INTERVALS 110 
            //SQL_ACTIVE_ENVIRONMENTS 116 SQL_BATCH_ROW_COUNT 120 SQL_BATCH_SUPPORT 121 SQL_BOOKMARK_PERSISTENCE 82 
            
            int[] id={79,80,83,114,92,124,123,68,127,128,129,130,131,133,136,10001,84,
                         137,138,139,140,142,149,150,151,172,10022,34,97,31,98,99,100,101,0,
                     102,33,32,104,105,106,107,49,91,156,157,109,110,116,120,121,82,22,55};

            int res = 0;
            st_odbc_connection odbc_con = new st_odbc_connection();
            StringBuilder sb_info = new StringBuilder(16);
            StringBuilder sb_length = new StringBuilder(16);

            foreach (int i in id)
            {
                res = SQLGetInfo(ref odbc_con, i, sb_info, 16, sb_length);
                Assert.IsTrue(sb_info.Length == 0);
            }
        }

        //For backward compatibility
        private static void case_OdbcInfoCompat()
        {
            int res = 0;
            st_odbc_connection odbc_con = new st_odbc_connection();
            StringBuilder sb_info = new StringBuilder(16);
            StringBuilder sb_length = new StringBuilder(16);

            odbc_con.handle_type = 0;
            //SQL_FETCH_DIRECTION 8
            res = SQLGetInfo(ref odbc_con, 8, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_LOCK_TYPES 78
            res = SQLGetInfo(ref odbc_con, 78, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_ODBC_API_CONFORMANCE 9
            res = SQLGetInfo(ref odbc_con, 9, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_ODBC_SQL_CONFORMANCE 15
            res = SQLGetInfo(ref odbc_con, 15, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);         

            //SQL_SCROLL_CONCURRENCY 43
            res = SQLGetInfo(ref odbc_con, 43, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

        }

        //Database info
        private static void case_OdbcDataBaseInfo()
        {
            int res = 0;
            st_odbc_connection odbc_con = new st_odbc_connection();
            StringBuilder sb_info = new StringBuilder(16);
            StringBuilder sb_length = new StringBuilder(16);

            //SQL_CATALOG_NAME 10003
            res = SQLGetInfo(ref odbc_con, 10003, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString() == "N");
            res = SQLGetInfo(ref odbc_con, 10003, sb_info, 0, sb_length);

            //SQL_COLLATION_SEQ 10004
            res = SQLGetInfo(ref odbc_con, 10004, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);
            res = SQLGetInfo(ref odbc_con, 10004, sb_info, 0, sb_length);

            //SQL_CATALOG_NAME_SEPARATOR 41
            res = SQLGetInfo(ref odbc_con, 41, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);
            res = SQLGetInfo(ref odbc_con, 41, sb_info, 0, sb_length);

            //SQL_CATALOG_TERM 42
            res = SQLGetInfo(ref odbc_con, 42, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);
            res = SQLGetInfo(ref odbc_con, 42, sb_info, 0, sb_length);

            //SQL_DATA_SOURCE_NAME 2
            res = SQLGetInfo(ref odbc_con, 2, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);  

            odbc_con.data_source = "1.1.1.1";
            res = SQLGetInfo(ref odbc_con, 2, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString() == odbc_con.data_source);        

            res = SQLGetInfo(ref odbc_con, 2, sb_info, 0, sb_length);

            //SQL_DATA_SOURCE_READ_ONLY 25
            odbc_con.attr_access_mode = 0;
            res = SQLGetInfo(ref odbc_con, 25, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString() == "N");

            res = SQLGetInfo(ref odbc_con, 25, sb_info, 0, sb_length);

            //SQL_DATABASE_NAME 16
            res = SQLGetInfo(ref odbc_con, 16, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);

            res = SQLGetInfo(ref odbc_con, 16, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);

            res = SQLGetInfo(ref odbc_con, 16, sb_info, 0, sb_length);

            //SQL_CONVERT_FUNCTIONS 48
            res = SQLGetInfo(ref odbc_con, 48, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 2);

            //SQL_CORRELATION_NAME 74
            res = SQLGetInfo(ref odbc_con, 74, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 2);

            //SQL_CREATE_TABLE 132
            res = SQLGetInfo(ref odbc_con, 132, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_CREATE_VIEW 134
            res = SQLGetInfo(ref odbc_con, 134, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 3);

            //SQL_DATETIME_LITERALS 119
            res = SQLGetInfo(ref odbc_con, 119, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 7);

            //SQL_DDL_INDEX 170
            res = SQLGetInfo(ref odbc_con, 170, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 3);

            //SQL_DEFAULT_TXN_ISOLATION 26
            res = SQLGetInfo(ref odbc_con, 26, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 2);

            //SQL_DRIVER_VER 7
            res = SQLGetInfo(ref odbc_con, 7, sb_info, 0, sb_length);

            //SQL_DROP_TABLE 141
            res = SQLGetInfo(ref odbc_con, 141, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_DROP_VIEW 143
            res = SQLGetInfo(ref odbc_con, 143, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_DYNAMIC_CURSOR_ATTRIBUTES1 144
            res = SQLGetInfo(ref odbc_con, 144, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_DYNAMIC_CURSOR_ATTRIBUTES2 145
            res = SQLGetInfo(ref odbc_con, 145, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_EXPRESSIONS_IN_ORDERBY 27
            res = SQLGetInfo(ref odbc_con, 27, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length ==1);

            //SQL_XOPEN_CLI_YEAR 10000
            res = SQLGetInfo(ref odbc_con, 10000, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_USER_NAME 47
            res = SQLGetInfo(ref odbc_con, 47, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            res = SQLGetInfo(ref odbc_con, 47, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);

            //SQL_UNION 96
            res = SQLGetInfo(ref odbc_con, 96, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_TXN_ISOLATION_OPTION 72
            res = SQLGetInfo(ref odbc_con, 72, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 15);

            //SQL_TXN_CAPABLE 46 
            res = SQLGetInfo(ref odbc_con, 46, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 2);

            //SQL_TIMEDATE_FUNCTIONS 52
            res = SQLGetInfo(ref odbc_con, 52, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);

            //SQL_SYSTEM_FUNCTIONS 51
            res = SQLGetInfo(ref odbc_con, 51, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);

            //SQL_SUBQUERIES 95
            res = SQLGetInfo(ref odbc_con, 95, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 31);

            //SQL_STRING_FUNCTIONS 50
            res = SQLGetInfo(ref odbc_con, 50, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 72);

            //SQL_STATIC_CURSOR_ATTRIBUTES2 168
            res = SQLGetInfo(ref odbc_con, 168, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 3);

            //SQL_STATIC_CURSOR_ATTRIBUTES1 167
            res = SQLGetInfo(ref odbc_con, 167, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 7);

            //SQL_STANDARD_CLI_CONFORMANCE 166
            res = SQLGetInfo(ref odbc_con, 166, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 2);

            //SQL_SQL92_VALUE_EXPRESSIONS 165
            res = SQLGetInfo(ref odbc_con, 165, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 15);

            //SQL_SQL92_STRING_FUNCTIONS 164
            res = SQLGetInfo(ref odbc_con, 164, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_SQL92_ROW_VALUE_CONSTRUCTOR 163
            res = SQLGetInfo(ref odbc_con, 163, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 11);

            //SQL_SQL92_REVOKE 162
            res = SQLGetInfo(ref odbc_con, 162, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_SQL92_RELATIONAL_JOIN_OPERATORS 161
            res = SQLGetInfo(ref odbc_con, 161, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 63);

            //SQL_SQL92_PREDICATES 160
            res = SQLGetInfo(ref odbc_con, 160, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 7);

            //SQL_SQL92_NUMERIC_VALUE_FUNCTIONS 159
            res = SQLGetInfo(ref odbc_con, 159, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 51);

            //SQL_SQL92_GRANT 158
            res = SQLGetInfo(ref odbc_con, 158, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 63);
        }
        private static void case_OdbcDateType()
        {
            int res = 0;
            st_odbc_connection odbc_con = new st_odbc_connection();
            //odbc_con.diag = null;
            StringBuilder sb_info = new StringBuilder(16);
            StringBuilder sb_length = new StringBuilder(16);

            //SQL_CONVERT_BIGINT 53
            res = SQLGetInfo(ref odbc_con, 53, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 3);

            //SQL_CONVERT_BINARY
            res = SQLGetInfo(ref odbc_con, 54, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_CONVERT_CHAR 56
            res = SQLGetInfo(ref odbc_con, 56, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 63);

            //SQL_CONVERT_DATE 57
            res = SQLGetInfo(ref odbc_con, 57, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 63);

            //SQL_CONVERT_DECIMAL 58
            res = SQLGetInfo(ref odbc_con, 58, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 3);

            //SQL_CONVERT_DOUBLE 59
            res = SQLGetInfo(ref odbc_con, 59, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 3);
            //SQL_CONVERT_FLOAT 60
            res = SQLGetInfo(ref odbc_con, 60, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 3);

            //SQL_CONVERT_INTEGER 61
            res = SQLGetInfo(ref odbc_con, 61, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 3);

            //SQL_CONVERT_LONGVARBINARY 71
            res = SQLGetInfo(ref odbc_con, 71, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 15);

            //SQL_CONVERT_LONGVARCHAR 62
            res = SQLGetInfo(ref odbc_con, 62, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 63);

            //SQL_CONVERT_NUMERIC 63
            res = SQLGetInfo(ref odbc_con, 63, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 3);

            //SQL_CONVERT_REAL 64
            res = SQLGetInfo(ref odbc_con, 64, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 3);

            //SQL_CONVERT_SMALLINT 65
            res = SQLGetInfo(ref odbc_con, 65, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 3);

            //SQL_CONVERT_TIME 66
            res = SQLGetInfo(ref odbc_con, 66, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_CONVERT_TIMESTAMP 67
            res = SQLGetInfo(ref odbc_con, 67, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 63);

            //SQL_CONVERT_VARBINARY 69
            res = SQLGetInfo(ref odbc_con, 69, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 15);

            //SQL_CONVERT_VARCHAR 70
            res = SQLGetInfo(ref odbc_con, 70, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[1] == 63);
        }

        [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLGetInfo",CharSet =CharSet.Ansi)]
        public static extern int SQLGetInfo(ref st_odbc_connection con, int info_type,
            StringBuilder info_value_ptr, int buffer_length, StringBuilder string_length_ptr);
        public static void case_odbc_get_info()
        {
            int res = 0;
            st_odbc_connection odbc_con = new st_odbc_connection();
            //odbc_con.diag=null;
            StringBuilder sb_info = new StringBuilder(16);
            StringBuilder sb_length = new StringBuilder(16);

            //For backward compatibility
            case_OdbcInfoCompat();
            //not support
            case_OdbcInfoNotSupport();
            //the info of data type
            case_OdbcDateType();
            //DataBase Info
            case_OdbcDataBaseInfo();

            //SQL_ACCESSIBLE_PROCEDURES
            res = SQLGetInfo(ref odbc_con, 20, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString() == "N");
            res = SQLGetInfo(ref odbc_con, 20, sb_info, 0, sb_length);
            Assert.IsTrue(res == 1);

            //SQL_ACCESSIBLE_TABLES
            res = SQLGetInfo(ref odbc_con, 19, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString() == "Y");
            res = SQLGetInfo(ref odbc_con, 19, sb_info, 0, sb_length);
            Assert.IsTrue(res == 1);

            //SQL_AGGREGATE_FUNCTIONS
            res = SQLGetInfo(ref odbc_con, 169, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 127);

            //SQL_ALTER_DOMAIN
            res = SQLGetInfo(ref odbc_con, 117, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 14);

            //SQL_ALTER_TABLE 86
            res = SQLGetInfo(ref odbc_con, 86, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 107);

            //SQL_ASYNC_MODE 10021
            res = SQLGetInfo(ref odbc_con, 10021, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 2);        

            //SQL_COLUMN_ALIAS 87
            res = SQLGetInfo(ref odbc_con, 87, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString() == "N");
            res = SQLGetInfo(ref odbc_con, 87, sb_info, 0, sb_length);

            //SQL_DESCRIBE_PARAMETER 10002
            res = SQLGetInfo(ref odbc_con, 10002, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString() == "N");
            res = SQLGetInfo(ref odbc_con, 10002, sb_info, 0, sb_length);

            //SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1 146
            res = SQLGetInfo(ref odbc_con, 146, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2 147
            res = SQLGetInfo(ref odbc_con, 147, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 3);

            //SQL_INDEX_KEYWORDS 148
            res = SQLGetInfo(ref odbc_con, 148, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 3);

            //SQL_INTEGRITY 73
            res = SQLGetInfo(ref odbc_con, 73, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_LIKE_ESCAPE_CLAUSE 113
            res = SQLGetInfo(ref odbc_con, 113, sb_info, 0, sb_length);

            //SQL_MAX_BINARY_LITERAL_LEN 112
            res = SQLGetInfo(ref odbc_con, 112, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length ==4);

            //SQL_MAX_CHAR_LITERAL_LEN 108
            res = SQLGetInfo(ref odbc_con, 108, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 4);

            //SQL_MAX_COLUMN_NAME_LEN 30
            res = SQLGetInfo(ref odbc_con, 30, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_MAX_IDENTIFIER_LEN 10005
            res = SQLGetInfo(ref odbc_con, 10005, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_MAX_ROW_SIZE_INCLUDES_LONG 103
            res = SQLGetInfo(ref odbc_con, 103, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_MAX_TABLE_NAME_LEN 35
            res = SQLGetInfo(ref odbc_con, 35, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_MULT_RESULT_SETS 36
            res = SQLGetInfo(ref odbc_con, 36, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_MULTIPLE_ACTIVE_TXN 37 
            res = SQLGetInfo(ref odbc_con, 37, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_NEED_LONG_DATA_LEN 111
            res = SQLGetInfo(ref odbc_con, 111, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 78);

            //SQL_NON_NULLABLE_COLUMNS 75
            res = SQLGetInfo(ref odbc_con, 75, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_NULL_COLLATION 85
            res = SQLGetInfo(ref odbc_con, 85, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_ODBC_INTERFACE_CONFORMANCE 152
            res = SQLGetInfo(ref odbc_con, 152, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_PARAM_ARRAY_ROW_COUNTS 153
            res = SQLGetInfo(ref odbc_con, 153, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 2);

            //SQL_PARAM_ARRAY_ROW_COUNTS 154
            res = SQLGetInfo(ref odbc_con, 154, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 3);

            //SQL_PROCEDURE_TERM 40
            res = SQLGetInfo(ref odbc_con, 40, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_PROCEDURES 21
            res = SQLGetInfo(ref odbc_con, 21, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_ROW_UPDATES 11
            res = SQLGetInfo(ref odbc_con, 11, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_SCHEMA_TERM 39
            res = SQLGetInfo(ref odbc_con, 39, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);

            //SQL_SCROLL_OPTIONS 44
            res = SQLGetInfo(ref odbc_con, 44, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 23);

            //SQL_SEARCH_PATTERN_ESCAPE 14
            res = SQLGetInfo(ref odbc_con, 14, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 92);

            //SQL_SERVER_NAME 13
            res = SQLGetInfo(ref odbc_con, 13, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.Length == 0);

            res = SQLGetInfo(ref odbc_con, 13, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 0);

            //SQL_SPECIAL_CHARACTERS 94
            res = SQLGetInfo(ref odbc_con, 94, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 0);

            //SQL_SQL_CONFORMANCE 118
            res = SQLGetInfo(ref odbc_con, 118, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 1);

            //SQL_SQL92_DATETIME_FUNCTIONS 155
            res = SQLGetInfo(ref odbc_con, 155, sb_info, 16, sb_length);
            Assert.IsTrue(sb_info.ToString()[0] == 7);

            //SQL_TABLE_TERM 45
            res = SQLGetInfo(ref odbc_con, 45, sb_info, 0, sb_length);
            Assert.IsTrue(sb_info.Length == 1);
        }
    }
}