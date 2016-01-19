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
    /// Test CUBRIDCommand column properties
    /// </summary>
      [DllImport(@"cubrid_odbc.dll", EntryPoint = "SQLBulkOperations", CharSet = CharSet.Ansi)]
      public static extern short SQLBulkOperations(ref st_odbc_statement statement, short operation);

      public static void Test_bulk_operation()
      {
          short operation=0,res =0;
          MarshalManage mb_int = new MarshalManage(4);

          MarshalManage mb_indicator = new MarshalManage(4);
          MarshalManage mb_data = new MarshalManage(16);

          st_odbc_record record2 = new st_odbc_record();
          record2.indicator_ptr = mb_indicator.buffer;
          record2.data_ptr = mb_data.buffer;
          record2.table_name = "test";
          record2.record_number = 1;
          MarshalManage mb_record2 = new MarshalManage(record2);

          st_odbc_record record = new st_odbc_record();
          record.indicator_ptr =mb_indicator.buffer;
          record.data_ptr = mb_data.buffer;
          record.next = mb_record2.buffer;
          MarshalManage mb_record= new MarshalManage(record);

          st_odbc_connection conn = new st_odbc_connection();
          MarshalManage mb_conn = new MarshalManage(conn);

          st_odbc_statement statement = new st_odbc_statement();
          statement.conn = mb_conn.buffer;
          st_odbc_desc desc = new st_odbc_desc();
          desc.rows_processed_ptr = mb_int.buffer;
          desc.max_count = 1;
          desc.array_size = 1;
          desc.records = mb_record.buffer;
          MarshalManage mb_array_status_ptr= new MarshalManage(16);
          desc.array_status_ptr = mb_array_status_ptr.buffer;
          MarshalManage mb_desc = new MarshalManage(desc);
          
          statement.apd = mb_desc.buffer;
          statement.ipd = mb_desc.buffer;
          statement.ard = mb_desc.buffer;
          statement.ird = mb_desc.buffer;

          res = SQLBulkOperations(ref statement, operation);

          operation = 5;
          res = SQLBulkOperations(ref statement, operation);

          operation = 4;
          res = SQLBulkOperations(ref statement, operation);

          operation = 6;
          res = SQLBulkOperations(ref statement, operation);

          operation = 7;
          res = SQLBulkOperations(ref statement, operation);
      }
  }
}