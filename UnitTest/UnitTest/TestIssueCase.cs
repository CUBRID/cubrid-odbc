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
        /// Test IssueCase class
        /// </summary>
        /// 

        public static void Test_multibyte_columns()
        {
            using (OdbcCommand cmd = new OdbcCommand("Drop table if exists [인적사항]", conn))
            {
                cmd.ExecuteNonQuery();
            }

            using (OdbcCommand cmd = new OdbcCommand("Create Table [인적사항] ([이름] varchar(16), [나이] integer)", conn))
            {
                cmd.ExecuteNonQuery();
            }

            using (OdbcCommand cmd = new OdbcCommand("desc [인적사항]", conn))
            {
                String column;

                OdbcDataReader reader = cmd.ExecuteReader();
                reader.Read(); //only two row will be available
                column = reader.GetString(0);
                Assert.IsTrue(column == "이름");
                reader.Read(); //only two row will be available
                column = reader.GetString(0);
                Assert.IsTrue(column == "나이");
            }

            using (OdbcCommand cmd = new OdbcCommand("drop table if exists [인적사항] ", conn))
            {
               cmd.ExecuteNonQuery();
            }
        }

        public static void Test_multibyte_binding()
        {
            using (OdbcCommand cmd = new OdbcCommand("Drop table if exists [인적사항]", conn))
            {
                cmd.ExecuteNonQuery();
            }

            using (OdbcCommand cmd = new OdbcCommand("Create Table [인적사항] ([이름] varchar(16), [나이] integer)", conn))
            {
                cmd.ExecuteNonQuery();
            }

            using (OdbcCommand cmd = new OdbcCommand("insert into [인적사항] values (?, ?)", conn))
            {
                OdbcParameter name = new OdbcParameter("?", "홍길동");
                OdbcParameter age = new OdbcParameter("?", 19);
                cmd.Parameters.Add(name);
                cmd.Parameters.Add(age);
                cmd.ExecuteNonQuery();
            }

            using (OdbcCommand cmd = new OdbcCommand("select * from [인적사항]", conn))
            {
                OdbcDataReader reader = cmd.ExecuteReader();
                reader.Read();
                Assert.IsTrue(reader.GetString(0) == "홍길동");
                Assert.IsTrue(reader.GetInt32(1) == 19);
            }

            using (OdbcCommand cmd = new OdbcCommand("drop table if exists [인적사항] ", conn))
            {
                cmd.ExecuteNonQuery();
            }
        }
    }
}