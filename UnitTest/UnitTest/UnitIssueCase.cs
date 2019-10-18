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
        private static readonly string connString =
         "Driver={CUBRID Driver Unicode};server=localhost;port=33000;uid=dba;pwd=;db_name=demodb1;charset=utf-8;";
        private static OdbcConnection conn = new OdbcConnection();

        public static void TestCase_init()
        {
            conn.ConnectionString = connString;
            conn.Open();
        }

        public static void  TestCase_dinit()
        {
            conn.Close();
        }

        public static void case_select()
        {
            using (OdbcCommand cmd = new OdbcCommand("select * from code;", conn))
            {
                OdbcDataReader reader = (OdbcDataReader)cmd.ExecuteReader();
                int i = 0;
                while (reader.Read())
                {
                    i++;
                    // Console.WriteLine(reader.GetInt32(0));
                    Console.WriteLine(reader.GetValue(0).ToString());
                    Assert.IsNotNull(reader);
                }
                if (0 == i)
                {
                    Console.WriteLine("no data.");
                }
            }
        }

        static public void case_connect_properties()
        {
            OdbcConnection conn = new OdbcConnection();
            conn.ConnectionString =
                "Driver={CUBRID Driver};server=1.1.1.1;port=30000;uid=dba;pwd=;db_name=demodb;Database=demodb;althosts=test-db-server:33000,2.3.4.5:33000;loginTimeout=600";
            conn.Open();

            Assert.AreEqual(conn.State,  ConnectionState.Open);
            conn.Close();
        }

        public static void case_dataset()
        {
            OdbcDataAdapter da=new OdbcDataAdapter("select * from code;",conn);

            DataSet ds = new DataSet();
            da.Fill(ds);

            DisplayData(ds.Tables[0]);
        }

        public static void case_datatable()
        {
            OdbcDataAdapter da = new OdbcDataAdapter("select * from code;", conn);

            DataTable dt = new DataTable();
            da.Fill(dt);

            DisplayData(dt);
        }

        static public void case_GetInt16_OverBound_Max()
        {
            // Open Connection
            string strConn = connString;
            OdbcConnection connCubrid = conn;

            int test_int = Int16.MaxValue + 1;  //Int32  32768
            string testTable = "t_type_int16_overbound";
            string strCreateTable = string.Format("CREATE TABLE {0}(id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, f_int INT)", testTable);
            string strDropTable = string.Format("DROP TABLE {0}", testTable);
            string strSqlInsert = string.Format("INSERT INTO {0}(f_int) VALUE({1})", testTable, test_int);

            ExecuteMultiQueries(connCubrid, new string[] { strDropTable, strCreateTable, strSqlInsert });

            string strSqlSelect = string.Format("SELECT f_int FROM {0} ORDER BY id DESC;", testTable);
            OdbcDataReader odbcReader = CreateReader(connCubrid, strSqlSelect);
            // Int16 result = odbcReader.GetInt16(0);
            Console.WriteLine(test_int);  // output:  32768
            try
            {
                Console.WriteLine(odbcReader.GetInt16(0));   // output:  -32768, expected result: OdbcException
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }

        }

        static public void case_GetInt16_OverBound_Min()
        {
            // Open Connection
            string strConn = connString;
            OdbcConnection connCubrid = conn;

            // int test_int = Int32.MinValue;  //Int32  -32769
            int test_int = Int32.MinValue;
            string testTable = "t_type_int16_overbound";
            string strCreateTable = string.Format("CREATE TABLE {0}(id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, f_int INT)", testTable);
            string strDropTable = string.Format("DROP TABLE {0}", testTable);
            string strSqlInsert = string.Format("INSERT INTO {0}(f_int) VALUE({1})", testTable, test_int);

            ExecuteMultiQueries(connCubrid, new string[] { strDropTable, strCreateTable, strSqlInsert });

            string strSqlSelect = string.Format("SELECT * FROM {0} ORDER BY id DESC;", testTable);
            OdbcDataReader odbcReader = CreateReader(connCubrid, strSqlSelect);

            Console.WriteLine(test_int);  // output:  -32769    
            try
            {
                Console.WriteLine(odbcReader.GetInt16(0));   // output:  -32768, expected result: OdbcException
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        static public void case_invalid_db()
        {
            // Invalid connection string - invalid db_name
            try
            {
                String strConn2 = @"Dsn=CUBRID_ODBC;db_name=invalid_db;uid=dba;pwd=;server=test-db-server;port=33000;fetch_size=100";
                OdbcConnection connCubrid2 = new OdbcConnection(strConn2);
                connCubrid2.Open();
                connCubrid2.Close();
            }
            catch (OdbcException e)
            {
                Console.WriteLine("Error message is : ");
                Console.WriteLine(e.Message);
            }

            // Invalid connection string - invalid uid
            try
            {
                String strConn3 = @"Dsn=CUBRID_ODBC;db_name=test_odbc;uid=invlid_uid;pwd=;server=test-db-server;port=33000;fetch_size=100";
                OdbcConnection connCubrid3 = new OdbcConnection(strConn3);
                connCubrid3.Open();
                connCubrid3.Close();
            }
            catch (OdbcException e)
            {
                Console.WriteLine("Error message is : ");
                Console.WriteLine(e.Message);
            }

            // Invalid connection string - invalid port
            try
            {
                String strConn3 = @"Dsn=CUBRID_ODBC;db_name=test_odbc;uid=invlid_uid;pwd=;server=test-db-server;port=330001;fetch_size=100";
                OdbcConnection connCubrid3 = new OdbcConnection(strConn3);
                connCubrid3.Open();
                connCubrid3.Close();
            }
            catch (OdbcException e)
            {
                Console.WriteLine("Error message is : ");
                Console.WriteLine(e.Message);
            }
        }
    }
}
