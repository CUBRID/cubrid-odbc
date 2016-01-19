using System;
using System.Text;
using System.Data;
using System.Data.Odbc;
using System.Data.Common;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace UnitTest
{
    public class MarshalManage
    {
        public IntPtr buffer;
        public MarshalManage(object ob)
        {
            int size = Marshal.SizeOf(ob);
            buffer = Marshal.AllocHGlobal(size);

            //将托管对象拷贝到非托管内存
            Marshal.StructureToPtr(ob, buffer, false);
        }
        public MarshalManage(int size)
        {
            buffer = Marshal.AllocHGlobal(size);
            Marshal.WriteByte(buffer,0);
        }
        ~MarshalManage()
        {
            if(buffer != null)
                Marshal.FreeHGlobal(buffer);
        }
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct D_STRING
    {
        public IntPtr value;
        public int totalSize;
        public int usedSize;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct tagCOLUMN_DATA
    {
        public short column_no;
        public IntPtr current_pt;
        public int remain_length;
        public short prev_return_status;
    } ;

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct  tagREVISED_SQL
    {
        public IntPtr sql_text;		// revised sql text
        public IntPtr org_param_pos;		// original param pos in revised sql text
        public IntPtr oid_param_pos;		// oid param pos in revised sql text
        public IntPtr oid_param_val;		// oid param val
        public short oid_param_num;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct tagCATALOG_RESULT
    {
        public IntPtr value;
        public IntPtr current;		// catalog result set cursor
    };
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct tagPARAM_DATA
    {
        public D_STRING val;
        public int index;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct st_odbc_statement
    {
        public UInt16 handle_type;
        public IntPtr diag;

        public IntPtr next;
        public IntPtr conn;
        int stmthd;			/* internal handle for CAS */
        tagCATALOG_RESULT catalog_result;
        tagCOLUMN_DATA column_data;

        public int result_type;	/* if NULL_RESULT, no result set. */
        public string sql_text;		/* origianl SQL statement */
        tagREVISED_SQL revised_sql;
        public string cursor;			/* cursor name */
        public short data_at_exec_state;
        public UInt32 tpl_number;
        public UInt32 current_tpl_pos;	

        public UInt16 param_number;
        public int stmt_type;
        tagPARAM_DATA param_data;	/* For data at exec */
        public char is_prepared;		

        /* Supported attributes */
        public UInt32 attr_metadata_id;	// Core
        public UInt32 attr_cursor_scrollable;	// 1
        public UInt32 attr_cursor_type;	// core, 2
        public UInt32 attr_async_enable;	// 1, 2
        public UInt32 attr_use_bookmark;	

        /* Not supported attributes */
        public UInt32 attr_noscan;	// Core
        public UInt32 attr_concurrency;	// 1, 2
        public UInt32 attr_cursor_sensitivity;	// 2
        public UInt32 attr_enable_auto_ipd;	// 2
        public IntPtr attr_fetch_bookmark_ptr;	// 2
        public UInt32 attr_keyset_size;	// 2
        public UInt32 attr_max_length;	// 1
        public UInt32 attr_max_rows;	// 1
        public UInt32 attr_query_timeout;	// 2
        public UInt32 attr_retrieve_data;	// 1
        public IntPtr attr_row_operation_ptr;	// 1
        public UInt32 attr_simulate_cursor;	// 2

        // Descriptor section
        public IntPtr i_apd;
        public IntPtr i_ard;
        public IntPtr i_ipd;
        public IntPtr i_ird;

        public IntPtr apd;
        public IntPtr ard;
        public IntPtr ipd;
        public IntPtr ird;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct st_odbc_connection
    {
        public UInt16 handle_type;
        public IntPtr diag;
        public int connhd;
        public IntPtr env;
        public IntPtr next;
        public IntPtr statements;
        public IntPtr descriptors;	/* external descriptor */

        public string data_source;		/* data source name */
        public string server;	/* odbc server address */
        public UInt32 port;			/* odbc server port number */
        public string db_name;		/* CUBRID db name */
        public string user;			/* CUBRID db user */
        public string password;		/* CUBRID db password */
        public int fetch_size;		/* fetch size */
        public string charset;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string db_ver;

        public UInt32 old_txn_isolation;	/* for read-only mode */

        // Maximum length of the string data type from UniCAS
        public UInt32 max_string_length;

        /* ODBC connection attributes */

        public UInt32 attr_access_mode;	/* CORE */
        public UInt32 attr_autocommit;	/* LEVEL 1 */
        //unsigned long         attr_connection_dead;   /* LEVEL 1 */
        //      attr_connection_dead绰 connhd肺 何磐 舅酒 尘 荐 乐促.
        // if connhd > 0, alive.

        public IntPtr attr_quiet_mode;	/* CORE */
        public UInt32 attr_metadata_id;	/* CORE */
        public UInt32 attr_odbc_cursors;	/* CORE, DM */
        public UInt32 attr_trace;	/* CORE, DM */
        public string attr_tracefile;		/* CORE, DM */
        public UInt32 attr_txn_isolation;	/* LEVEL 1 */
        public UInt32 attr_async_enable;	/* LEVEL 1 */

        /* Not supported */
        public UInt32 attr_auto_ipd;	/* LEVEL2, RDONLY */
        public UInt32 attr_connection_timeout;	/* LEVEL 2 */
        public string attr_current_catalog;	/* LEVEL 2 */
        public UInt32 attr_login_timeout;	/* LEVEL 2 */
        public UInt32 attr_packet_size;	/* LEVEL 2 */
        public string attr_translate_lib;	/* CORE */
        public UInt32 attr_translate_option;	/* CORE */

        /* stmt attributes */
        public UInt32 attr_max_rows;	// 1
        public UInt32 attr_query_timeout;	// 2
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct st_odbc_record
    {
      /* Driver-specific members */
      public IntPtr desc;
      public IntPtr next;
      public short record_number;

      /* supported descriptor record field */
      public string base_column_name;
      public UInt32 case_sensitive;
      public short concise_type;
      public IntPtr data_ptr;
      public short datetime_interval_code;
      public UInt32 datetime_interval_precision;
      public UInt32 display_size;
      public short fixed_prec_scale;
      public IntPtr indicator_ptr;
      public UInt32 length;
      public string literal_prefix;
      public string literal_suffix;
      public string local_type_name;
      public string name;
      public short nullable;
      public UInt32 num_prec_radix;
      public UInt32 octet_length;
      public IntPtr octet_length_ptr;
      public short parameter_type;
      public short precision;
      public short scale;
      public short searchable;
      public string table_name;
      public short type;
      public string type_name;
      public short unnamed;
      public short unsigned_type;

      /* not supported */
      public UInt32 auto_unique_value;
      public string base_table_name;
      public string catalog_name;
      public short rowver;
      public string schema_name;
      public short updatable;

    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct st_odbc_desc
    {
      public short handle_type;
      public IntPtr diag;

      public IntPtr stmt;
      public IntPtr conn;	// for explicitly allocated descriptor
      public IntPtr next;	// for explicitly allocated descriptor

      /* header fields */
      public short alloc_type;
      public UInt32 array_size;

      // the previous array_size of fetch call (SQLFetch, SQLExtendedFetch, SQLFetchScroll)
      public UInt32 fetched_size;

      public IntPtr array_status_ptr;
      public IntPtr bind_offset_ptr;
      public UInt32 bind_type;
      public short max_count;
      public IntPtr rows_processed_ptr;

      public IntPtr records;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct st_diag_record
    {
      public int native_code;
      public UInt32 number;
      public string sql_state;
      public string message;
      public IntPtr next; //st_diag_record
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct st_diag
    {
      public UInt32 rec_number;
      public short retcode;
      public IntPtr record; //st_diag_record
    };


    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct st_odbc_env
    {
        public short handle_type;
        public IntPtr diag;
        public IntPtr next;
        public IntPtr conn;
        /*
         struct odbc_connection *conn;
        */

        public string program_name;

        /* ODBC environment attributes */
        public UInt32 attr_odbc_version;
        public UInt32 attr_output_nts;

        /*  (Not supported) Optional features */
        public UInt32 attr_connection_pooling;
        public UInt32 attr_cp_match;
    };

    public struct stCUBRIDDSNItem
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string driver;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string dsn;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string db_name;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string user;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string password;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string server;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string port;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string fetch_size;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string save_file;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 520)]
        public string description;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string charset;
    } ;
}