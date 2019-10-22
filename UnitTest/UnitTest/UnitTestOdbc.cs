using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace UnitTest
{
    /// <summary>
    /// Summary description for UnitTest1
    /// </summary>
    [TestClass]
    public class UnitTestOdbc
    {
        public UnitTestOdbc()
        {
            //
            // TODO: Add constructor logic here
            //
        }

        private TestContext testContextInstance;

        /// <summary>
        ///Gets or sets the test context which provides
        ///information about and functionality for the current test run.
        ///</summary>
        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }

        #region Additional test attributes
        //
        // You can use the following additional attributes as you write your tests:
        //
        // Use ClassInitialize to run code before running the first test in the class
         [ClassInitialize()]
         public static void MyClassInitialize(TestContext testContext)
         {
             TestCases.TestCase_init();
         }

        //
        // Use ClassCleanup to run code after all tests in a class have run
         [ClassCleanup()]
         public static void MyClassCleanup() 
         {
             TestCases.TestCase_dinit();
         }

        //
        // Use TestInitialize to run code before running each test 
        // [TestInitialize()]
        // public void MyTestInitialize() { }
        //
        // Use TestCleanup to run code after each test has run
        // [TestCleanup()]
        // public void MyTestCleanup() { }
        //
        #endregion
        [TestMethod]
        public void IssueCaseMethod()
        {
            //
            // TODO: Add test logic	here
            //
            return;
            TestCases.case_invalid_db();
            TestCases.case_select();        
            TestCases.case_dataset();
            TestCases.case_datatable();
            TestCases.case_GetInt16_OverBound_Min();
            TestCases.case_GetInt16_OverBound_Max();         
        }
        [TestMethod]
        public void PropertyCaseMethod()
        {
            return;
            TestCases.case_connInfo();
            TestCases.case_GetDataTypeName();
        }
        [TestMethod]
        public void InterfaceCaseMethod()
        {
            //TestCases.Test_bulk_operation();
            //TestCases.Test_SetStmtAttr();
            //TestCases.Test_GetStmtAttr();
            //return;
            //TestCases.Test_GetConnectAttr();
            //TestCases.Test_SetConnectAttr();           
            //TestCases.Test_GetDiagField();
            //TestCases.Test_GetDescField();
            //TestCases.Test_GetData();
			/* 
			Temporarily test case is closed for incorrect test environment 
			This test case will be fixed and reopened in the future on correct environment
			
			TestCases.case_odbc_get_info(); 
			*/
        }
        [TestMethod]
        public void InitCaseMethod()
        {
            //
            // TODO: Add test logic	here
            //
            return;
            TestCases.Test_Connect();
            TestCases.Test_Various_DataTypes();
            TestCases.Test_Various_DataTypes2();
            TestCases.Test_Command_ColumnProperties();
        }
        [TestMethod]
        public void TransactionCaseMethod()
        {
            //
            // TODO: Add test logic	here
            //
            //return;
            TestCases.Test_Transaction();
            TestCases.Test_Transaction_Rollback();
            TestCases.Test_Transaction_Parameters();
            TestCases.Test_Transaction_Level();
        }

        [TestMethod]
        public void IssueTestCaseMethod()
        {
            TestCases.Test_multibyte_columns();
            TestCases.Test_multibyte_binding();
        }
    }
}
