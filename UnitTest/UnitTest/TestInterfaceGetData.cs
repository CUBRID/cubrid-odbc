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
        [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLGetData", CharSet = CharSet.Ansi)]
        public static extern int SQLGetData(
                    ref st_odbc_statement statement,
                    short ColumnNumber,
	                short TargetType,
	                string TargetValue,
	                int BufferLength, StringBuilder StrLen_or_Ind);

        public static void case_odbc_get_data()
        {
        }
        /// <summary>
        /// Test OdbcInterface class
        /// </summary>
        public static void Test_GetData()
        {
            StringBuilder sb_length = new StringBuilder(16);
            st_odbc_statement odbc_statement = new st_odbc_statement();

            odbc_statement.attr_simulate_cursor = 123456;

            //SQLGetData(ref odbc_statement, 1, 1, "12",16,sb_length);
        }
    }
}