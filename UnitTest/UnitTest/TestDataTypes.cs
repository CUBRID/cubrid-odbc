using System;
using System.Text;
using System.Data;
using System.Data.Odbc;
using System.Data.Common;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace UnitTest
{
    public partial class TestCases
    {
        /// <summary>
        /// Test OdbcTransaction class
        /// </summary>
        public static void Test_Various_DataTypes()
        {

            TestCases.ExecuteSQL("drop table if exists t", conn);

            string sql = "create table t(";
            sql += "c_integer_ai integer AUTO_INCREMENT, "; //1
            sql += "c_smallint smallint, "; //2
            sql += "c_integer integer, "; //3
            sql += "c_bigint bigint, "; //4
            sql += "c_numeric numeric(15,1), "; //5
            sql += "c_float float, ";//6
            sql += "c_decimal decimal(15,3), ";//7
            sql += "c_double double, ";//8
            sql += "c_char char, ";//9
            sql += "c_varchar varchar(4096), ";//10
            sql += "c_time time, ";//11
            sql += "c_date date, ";//12
            sql += "c_timestamp timestamp, ";//13
            sql += "c_datetime datetime, ";//14
            sql += "c_bit bit(1), ";//15
            sql += "c_varbit bit varying(4096), ";//16
            sql += "c_monetary monetary, ";//17
            sql += "c_string string";//18
            sql += ")";
            TestCases.ExecuteSQL(sql, conn);

            
            sql = "insert into t values(";
            sql += "1, "; //0
            sql += "11, "; //1
            sql += "111, "; //2
            sql += "1111, ";//3
            sql += "1.1, ";//4
            sql += "1.11, ";//5
            sql += "1.111, ";//6
            sql += "1.1111, ";//7
            sql += "'a', ";//8
            sql += "'abcdfghijk', ";//9
            sql += "TIME '13:15:45 pm', ";//10
            sql += "DATE '00-10-31', ";//11
            sql += "TIMESTAMP '13:15:45 10/31/2008', ";//12
            sql += "DATETIME '13:15:45 10/31/2008', ";//13
            sql += "B'1', ";//14
            sql += "B'111', ";//15
            sql += "123456789, ";//16
            sql += "'qwerty'";//17
            sql += ")";
            TestCases.ExecuteSQL(sql, conn);
            

            sql = "select * from t";
            using (OdbcCommand cmd = new OdbcCommand(sql, conn))
            {
                OdbcDataReader reader = cmd.ExecuteReader();
                while (reader.Read()) //only one row will be available
                {
                    Assert.IsTrue(reader.GetInt32(0) == 1);
                    Assert.IsTrue(reader.GetInt16(1) == 11);
                    Assert.IsTrue(reader.GetInt32(2) == 111);
                    Assert.IsTrue(reader.GetInt64(3) == 1111);
                    Assert.IsTrue(reader.GetDecimal(4) == (decimal)1.1);
                    Assert.IsTrue(reader.GetFloat(5) == (float)1.11); //"Single"
                    Assert.IsTrue(reader.GetDecimal(6) == (decimal)1.111);
                    Assert.IsTrue(reader.GetDouble(7) == (double)1.1111);

                    //We use GetString() because GetChar() is not supported or System.Data.OleDb.
                    //http://msdn.microsoft.com/en-us/library/system.data.oledb.oledbdatareader.getchar
                    Assert.IsTrue(reader.GetString(8) == "a"); //"String" ("Char" in CUBRID)

                    Assert.IsTrue(reader.GetString(9) == "abcdfghijk"); //"String" ("String in CUBRID)

                    //GetGateTime cannot cast just the time value in a DateTime object, so we use TimeSpan
                    //Assert.IsTrue(reader.GetTimeSpan(10) == new TimeSpan(13, 15, 45));//"TimeSpan"

                    Assert.IsTrue(reader.GetDateTime(11) == new DateTime(2000, 10, 31)); //"DateTime"
                    Assert.IsTrue(reader.GetDateTime(12) == new DateTime(2008, 10, 31, 13, 15, 45)); //"DateTime"
                    //     Console.WriteLine(reader.GetValue(13));
                    //     Assert.IsTrue(reader.GetDateTime(13) == new DateTime(2008, 10, 31, 13, 15, 45)); //"DateTime"

                    //The GetByte() method does not perform any conversions and the driver does not give tha data as Byte
                    //http://msdn.microsoft.com/en-us/library/system.data.oledb.oledbdatareader.getbyte
                    //Use GetValue() or GetBytes() methods to retrieve BIT coulumn value
                    //     Assert.IsTrue((reader.GetValue(14) as byte[])[0] == (byte)0); //"Byte[]" ("bit(1)" in CUBRID)
                    //Or

                    Assert.IsTrue(reader.GetString(14)=="80");
                    Assert.IsTrue(reader.GetString(15) == "E0");
                    Assert.IsTrue(reader.GetString(16) == "123456789.0000000000000000");                 
                    Assert.IsTrue(reader.GetString(17) == "qwerty"); //"String"
                }
            }

            TestCases.ExecuteSQL("drop table if exists t", conn);

        }
        public static void Test_Various_DataTypes2()
        {

            TestCases.ExecuteSQL("drop table if exists t", conn);

            string sql = "create table t(";
            sql += "c_integer_ai integer AUTO_INCREMENT, "; //1
            sql += "c_smallint smallint, "; //2
            sql += "c_integer integer, "; //3
            sql += "c_bigint bigint, "; //4
            sql += "c_numeric numeric(15,1), "; //5
            sql += "c_float float, ";//6
            sql += "c_decimal decimal(15,3), ";//7
            sql += "c_double double, ";//8
            sql += "c_char char, ";//9
            sql += "c_varchar varchar(4096), ";//10
            sql += "c_time time, ";//11
            sql += "c_date date, ";//12
            sql += "c_timestamp timestamp, ";//13
            sql += "c_datetime datetime, ";//14
            sql += "c_bit bit(1), ";//15
            sql += "c_varbit bit varying(4096), ";//16
            sql += "c_monetary monetary, ";//17
            sql += "c_string string";//18
            sql += ")";
            TestCases.ExecuteSQL(sql, conn);

            using (OdbcCommand cmd = new OdbcCommand(null, conn))
            {
                sql = "insert into t values(?, ?, ?, ?, ?, ?,";
                sql += "?, ?, ?, ?, ?, ?,";
                sql += "?, ?, ?, ?, ?, ?";
                sql += ")";

                OdbcParameter p1 = new OdbcParameter("?p1", OdbcType.Int);
                p1.Value = 1;
                cmd.Parameters.Add(p1);

                OdbcParameter p2 = new OdbcParameter("?p2", OdbcType.SmallInt);
                p2.Value = 11;
                cmd.Parameters.Add(p2);

                OdbcParameter p3 = new OdbcParameter("?p3", OdbcType.Int);
                p3.Value = 111;
                cmd.Parameters.Add(p3);

                OdbcParameter p4 = new OdbcParameter("?p4", OdbcType.BigInt);
                p4.Value = 1111;
                cmd.Parameters.Add(p4);

                OdbcParameter p5 = new OdbcParameter("?p5", OdbcType.Numeric);
                p5.Value = 1.1;
                cmd.Parameters.Add(p5);

                OdbcParameter p6 = new OdbcParameter("?p6", OdbcType.Double);
                p6.Value = 1.11;
                cmd.Parameters.Add(p6);

                OdbcParameter p7 = new OdbcParameter("?p7", OdbcType.Decimal);
                p7.Value = 1.111;
                cmd.Parameters.Add(p7);

                OdbcParameter p8 = new OdbcParameter("?p8", OdbcType.Double);
                p8.Value = 1.1111;
                cmd.Parameters.Add(p8);

                OdbcParameter p9 = new OdbcParameter("?p9", OdbcType.Char);
                p9.Value = 'a';
                cmd.Parameters.Add(p9);

                OdbcParameter p10 = new OdbcParameter("?p10", OdbcType.VarChar);
                p10.Value = "abcdfghijk";
                cmd.Parameters.Add(p10);

                cmd.CommandText = sql;

                cmd.ExecuteNonQuery();
            }

            sql = "select * from t";
            using (OdbcCommand cmd = new OdbcCommand(sql, conn))
            {
                OdbcDataReader reader = cmd.ExecuteReader();
                while (reader.Read()) //only one row will be available
                {
                    Assert.IsTrue(reader.GetInt32(0) == 1);
                    Assert.IsTrue(reader.GetInt16(1) == 11);
                    Assert.IsTrue(reader.GetInt32(2) == 111);
                    Assert.IsTrue(reader.GetInt64(3) == 1111);
                    Assert.IsTrue(reader.GetDecimal(4) == (decimal)1.1);
                    Assert.IsTrue(reader.GetFloat(5) == (float)1.11); //"Single"
                    Assert.IsTrue(reader.GetDecimal(6) == (decimal)1.111);
                    Assert.IsTrue(reader.GetDouble(7) == (double)1.1111);
                    Assert.IsTrue(reader.GetString(8) == "a"); //"String" ("Char" in CUBRID)

                    Assert.IsTrue(reader.GetString(9) == "abcdfghijk"); //"String" ("String in CUBRID)
                }
            }

            TestCases.ExecuteSQL("drop table if exists t", conn);

        }
        public static void Test_Connect()
        {
            String strConn = @"Dsn=CUBRID_ODBC;";//db_name=demodb1;";      
            OdbcConnection connCubrid = new OdbcConnection(strConn);
            connCubrid.Open();

            strConn = @"Driver={CUBRID Driver};db_name=demodb;";

            strConn = @"Dsn=CUBRID_ODBC;";//db_name=demodb1;";      
            connCubrid = new OdbcConnection(strConn);
            connCubrid.Open();
            Assert.IsTrue(connCubrid.State == ConnectionState.Open);
        }
    }
}