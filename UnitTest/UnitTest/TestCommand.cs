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
    /// Test CUBRIDCommand column properties
    /// </summary>
    public static void Test_Command_ColumnProperties()
    {
      using (OdbcConnection conn = new OdbcConnection())
      {
        conn.ConnectionString = TestCases.connString;
        conn.Open();

        String sql = "select * from nation";
        OdbcCommand cmd = new OdbcCommand(sql, conn);
        OdbcDataAdapter da = new OdbcDataAdapter();
        da.SelectCommand = cmd;
        DataTable dt = new DataTable("");
        da.FillSchema(dt, SchemaType.Source);//To retrieve all the column properties you have to use the FillSchema() method

        Assert.IsTrue(dt.Columns[0].ColumnName == "code");
      }
    }  
  }
}