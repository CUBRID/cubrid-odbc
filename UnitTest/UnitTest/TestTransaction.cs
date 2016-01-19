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
    public static void Test_Transaction()
    {
        using (OdbcConnection conn = new OdbcConnection())
        {
            OdbcTransaction transaction = null;
            conn.ConnectionString = TestCases.connString;
            conn.Open();

            TestCases.ExecuteSQL("drop table if exists t", conn);

            transaction = conn.BeginTransaction();

            string sql = "create table t(idx integer)";
            using (OdbcCommand command = new OdbcCommand(sql, conn))
            {
                command.Transaction = transaction;
                command.ExecuteNonQuery();
            }

            int tablesCount = GetTablesCount("t", conn, transaction);
            Assert.IsTrue(tablesCount == 1);

            transaction.Rollback();

            //Verify the table does not exist
            tablesCount = GetTablesCount("t", conn, transaction);
            Assert.IsTrue(tablesCount == 0);

            transaction = conn.BeginTransaction();

            sql = "create table t(idx integer)";
            using (OdbcCommand command = new OdbcCommand(sql, conn))
            {
                command.Transaction = transaction;
                command.ExecuteNonQuery();
            }

            tablesCount = GetTablesCount("t", conn, transaction);
            Assert.IsTrue(tablesCount == 1);

            transaction.Commit();

            tablesCount = GetTablesCount("t", conn, transaction);
            Assert.IsTrue(tablesCount == 1);

            transaction = conn.BeginTransaction();

            TestCases.ExecuteSQL("drop table t", conn, transaction);

            transaction.Commit();

            tablesCount = GetTablesCount("t", conn);
            Assert.IsTrue(tablesCount == 0);
        }
    }

    /// <summary>
    /// Test OdbcTransaction class, using parameters
    /// </summary>
    public static void Test_Transaction_Parameters()
    {
      OdbcTransaction tran = null;

      using (OdbcConnection conn = new OdbcConnection())
      {
        conn.ConnectionString = TestCases.connString;
        conn.Open();

        CreateTestTable(conn);

        tran = conn.BeginTransaction();

        string sql = "insert into t values(?, ?, ?, ?, ?, ?)";
        using (OdbcCommand cmd = new OdbcCommand(sql, conn))
        {
          OdbcParameter p1 = new OdbcParameter("?p1", OdbcType.Int);
          p1.Value = 1;
          cmd.Parameters.Add(p1);

          OdbcParameter p2 = new OdbcParameter("?p2", OdbcType.Char);
          p2.Value = 'A';
          cmd.Parameters.Add(p2);

          OdbcParameter p3 = new OdbcParameter("?p3", OdbcType.VarChar);
          p3.Value = "Odbc";
          cmd.Parameters.Add(p3);

          OdbcParameter p4 = new OdbcParameter("?p4", OdbcType.Double);
          p4.Value = 1.1f;
          cmd.Parameters.Add(p4);

          OdbcParameter p5 = new OdbcParameter("?p5", OdbcType.Double);
          p5.Value = 2.2d;
          cmd.Parameters.Add(p5);

          OdbcParameter p6 = new OdbcParameter("?p6", OdbcType.DateTime);
          p6.Value = DateTime.Now;
          cmd.Parameters.Add(p6);
          cmd.Transaction = tran; 
          cmd.ExecuteNonQuery();

          tran.Commit();
        }

        Assert.IsTrue(GetTableRowsCount("t", conn) == 1);

        CleanupTestTable(conn);
      }
    }
  }
}