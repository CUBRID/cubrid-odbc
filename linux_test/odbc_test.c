#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include "odbc_test.h"

testcase_t *odbc_testcases;
int num_testcases = 0;
int case_num = 1;
static int find_dsn (char *dsn);

int main (int argc, char *argv[])
{
  int i;
  int loaded_cases = 0;
  int run_all = 1;
  int name_len;
  char dsn[PATHMAX];

  int failed_count = 0;
  int failed_cases[MAX_TEST_CASES];


  if (argc != 2)
    {
      run_all = 0;
    }

  if (find_dsn (dsn))
    {
      return 1;
    }

  odbc_testcases = (testcase_t *) calloc (sizeof (testcase_t), MAX_TEST_CASES);
  loaded_cases = load_linux_odbc_testcases ();
  for  (i = 0; i < num_testcases; i++)
    {
      name_len = strlen (odbc_testcases[i].name);
      if (run_all && strncmp (odbc_testcases[i].name, argv[1], name_len) != 0)
	{
	  continue;
	}

      if (IS_LOADED (i))
	{
	  int rc;
	  printf ("running testcase #%d: %s\n", case_num, odbc_testcases[i].name);
	  rc = (odbc_testcases[i].func) (case_num, dsn);

	  if (rc != SQL_SUCCESS)
	    {
	      failed_cases[failed_count++] = case_num;
	    }

	  case_num++;
	}
    }

  if (failed_count > 0)
    {
      int j;

      printf ("========== TEST SUMMARY ==========\n");
      printf ("FAIL count: %d\n", failed_count);
      printf ("Failed testcases:\n");

      for (j = 0; j < failed_count; j++)
	{
	  printf ("%d, %s\n", failed_cases[j], odbc_testcases[failed_cases[j]-1].name);
	}
    }
  else
    {
      {
	printf ("========== All testcases passed ==========\n");
      }
    }
  return 0;
}

int
load_linux_odbc_testcases ()
{
  DIR *dirp;
  char cwd[PATHMAX];
  char path[PATHMAX];
  char *p;
  struct dirent *dp;
  void *dh;

  if (getcwd (cwd, PATHMAX) == NULL || (dirp = opendir (cwd)) == NULL)
    {
      return -1;
    }
 
  snprintf (path, sizeof (path), "%s/%s", cwd, LINUXODBC_TESTLIB);
  if ((dh = dlopen (path, RTLD_LAZY)) == NULL)
    {
      return -1;
    }

  while ((dp = readdir (dirp)) != NULL)
    {
      if (strncmp (dp->d_name, CASE_PREFIX, strlen (CASE_PREFIX)) != 0)
	{
	  continue;
	}

      p = strchr (dp->d_name, '.');
	if (p)
	  {
	    *p = '\0';
	  }

      if (testcase_exists (dp->d_name))
	{
	  continue;
	}

      strcpy (odbc_testcases[num_testcases].name, dp->d_name);
      odbc_testcases[num_testcases++].func = dlsym (dh, dp->d_name);
    }
}

int
testcase_exists (char *casename)
{
  int i;

  if (num_testcases == 0)
    {
      FALSE;
    }

  for (i = 0; i < num_testcases; i++)
    {
      if (strcmp (odbc_testcases[i].name, casename) == 0)
	{
	  return TRUE;
	}
    }

  return FALSE;
}

static int
find_dsn (char *dsn)
{
  FILE *fp;
  char buf [PATHMAX];
  char *p;

  if (dsn == NULL)
    {
      return 1;
    }

  fp = fopen (DSNFILE, "r");
  if (fp == NULL || fgets (buf, PATHMAX, fp) == NULL)
    {
      strcpy (dsn, DEFAULT_DSN);
      if (fp)
	{
	  fclose (fp);
	}
      return 0;
    }

  p = strchr (buf, '\n');
  if (p)
    {
      *p = '\0';
    }

  strcpy (dsn, buf);
  printf ("DSN = %s\n", dsn);

  fclose (fp);

  return 0;
}
