#include <sql.h>
#include <sqlext.h>

#define LINUXODBC_TESTLIB "libcubrid-odbc-test.so"
#define	DEFAULT_DSN "link2u"
#define IS_LOADED(idx)       (odbc_testcases[idx].func!=NULL)
#define CASE_PREFIX "sql_"
#define	MAX_TEST_CASES 512
#define PATHMAX 1024
#define DSNFILE "dsn.txt"

int load_linux_odbc_testcases (void);
int testcase_exists (char *);
enum
{ TRUE = 1, FALSE = 0 };
typedef struct
{
  char name[PATHMAX];
  char arg1[PATHMAX];
  char arg2[PATHMAX];

    SQLRETURN (*func) ();
} testcase_t;
