#! /bin/bash

SHELL_DIR=$(dirname $(readlink -f $0))
DRIVER_INSTALL_DIR=$HOME/cubrid-odbc/lib
ODBCINST_INI_FILE=$HOME/.odbcinst.ini
ODBC_INI_FILE=$HOME/.odbc.ini
TEST_DSN_FILE=$SHELL_DIR/linux_test/dsn.txt
TEST_DB_SERVER=

ODBCINST_1=false
ODBCINST_2=false
ODBCINST_3=false
ODBCINI_1=false
ODBCINI_2=false
ONLY_TEST=0

function show_usage ()
{
  echo "Usage: $0 [OPTIONS]"
  echo " OPTIONS"
  echo "  -t      Only test (default: false)"
  echo "  -i      Install directory (default: $HOME/cubrid-odbc/lib)"
  echo "  -s      Test database server (default: "")"
  echo "  -? | -h Show this help message and exit"
  echo ""
}

function get_options ()
{
  while getopts ":ts:h" opt; do
    case $opt in
      t ) ONLY_TEST=1;;
      s ) TEST_DB_SERVER="$OPTARG" ;;
      i ) DRIVER_INSTALL_DIR="$OPTARG" ;;
      h|\?|* ) show_usage; exit 1;;
    esac
  done
  shift $(($OPTIND - 1))
}

get_options "$@"

if [ -z "$TEST_DB_SERVER" ]; then
  echo "Error: -s is required"
  show_usage
  exit 1
fi

if [ $ONLY_TEST -eq 0 ]; then
  if [ ! -d "$DRIVER_INSTALL_DIR" ]; then
    mkdir -p $DRIVER_INSTALL_DIR
  fi
  $SHELL_DIR/build.sh -i $DRIVER_INSTALL_DIR
fi

echo "========== Check and Edit $ODBCINST_INI_FILE =========="
if [ ! -f "$ODBCINST_INI_FILE" ]; then
  touch $ODBCINST_INI_FILE
fi

while read -r line; do
  if [ "$line" = "[CUBRID ODBC Driver]" ]; then
    echo "Found [CUBRID ODBC Driver]"
    ODBCINST_1=true
  fi
  if [ "$line" = "[CUBRID ODBC Driver Unicode]" ]; then
    echo "Found [CUBRID ODBC Driver Unicode]"
    ODBCINST_2=true
  fi
  if [ "$line" = "[CUBRID Driver Unicode]" ]; then
    echo "Found [CUBRID Driver Unicode]"
    ODBCINST_3=true
  fi
done < $ODBCINST_INI_FILE

if [ "$ODBCINST_1" = false ]; then
  echo "edit $ODBCINST_INI_FILE"
  echo "[CUBRID ODBC Driver]" >> $ODBCINST_INI_FILE
  echo "Description = CUBRID Linux ODBC Driver" >> $ODBCINST_INI_FILE
  echo "Driver = $DRIVER_INSTALL_DIR/libcubrid-odbc.so" >> $ODBCINST_INI_FILE
  echo "FileUsage = 1" >> $ODBCINST_INI_FILE
fi

if [ "$ODBCINST_2" = false ]; then
  echo "edit $ODBCINST_INI_FILE"
  echo "[CUBRID ODBC Driver Unicode]" >> $ODBCINST_INI_FILE
  echo "Description = CUBRID Linux ODBC Unicode Driver" >> $ODBCINST_INI_FILE
  echo "Driver = $DRIVER_INSTALL_DIR/libcubrid-odbcw.so" >> $ODBCINST_INI_FILE
  echo "IANAAppCodePage = 1" >> $ODBCINST_INI_FILE
fi

if [ "$ODBCINST_3" = false ]; then
  echo "edit $ODBCINST_INI_FILE"
  echo "[CUBRID Driver Unicode]" >> $ODBCINST_INI_FILE
  echo "Description = CUBRID Linux ODBC Unicode Driver" >> $ODBCINST_INI_FILE
  echo "Driver = $DRIVER_INSTALL_DIR/libcubrid-odbcw.so" >> $ODBCINST_INI_FILE
  echo "IANAAppCodePage = 1" >> $ODBCINST_INI_FILE
fi

echo "========== check and edit $ODBC_INI_FILE =========="
if [ ! -f "$ODBCINI_FILE" ]; then
  touch $ODBC_INI_FILE
fi

while read -r line; do
  if [ "$line" = "[link2]" ]; then
    echo "Found [link2]"
    ODBCINI_1=true
  fi
  if [ "$line" = "[link2u]" ]; then
    echo "Found [link2u]"
    ODBCINI_2=true
  fi
done < $ODBC_INI_FILE

if [ "$ODBCINI_1" = false ]; then
  echo "edit $ODBC_INI_FILE"
  echo "[link2]" >> $ODBC_INI_FILE
  echo "Driver = CUBRID ODBC Driver" >> $ODBC_INI_FILE
  echo "Description = CUBRID ODBC" >> $ODBC_INI_FILE
  echo "DB_NAME = demodb" >> $ODBC_INI_FILE
  echo "UID = dba" >> $ODBC_INI_FILE
  echo "PWD = " >> $ODBC_INI_FILE
  echo "SERVER = $TEST_DB_SERVER" >> $ODBC_INI_FILE
  echo "PORT = 33000" >> $ODBC_INI_FILE
  echo "FETCH_SIZE = 1" >> $ODBC_INI_FILE
  echo "AUTOCOMMIT = false" >> $ODBC_INI_FILE
  echo "OMIT_SCHEMA = no" >> $ODBC_INI_FILE
  echo "CHARSET = ko_KR.utf8" >> $ODBC_INI_FILE
fi

if [ "$ODBCINI_2" = false ]; then
  echo "edit $ODBC_INI_FILE"
  echo "[link2u]" >> $ODBC_INI_FILE
  echo "Driver = CUBRID ODBC Driver Unicode" >> $ODBC_INI_FILE
  echo "Description = CUBRID ODBC Unicode" >> $ODBC_INI_FILE
  echo "DB_NAME = demodb" >> $ODBC_INI_FILE
  echo "UID = dba" >> $ODBC_INI_FILE
  echo "PWD = " >> $ODBC_INI_FILE
  echo "SERVER = $TEST_DB_SERVER" >> $ODBC_INI_FILE
  echo "PORT = 33000" >> $ODBC_INI_FILE
  echo "FETCH_SIZE = 1" >> $ODBC_INI_FILE
  echo "AUTOCOMMIT = false" >> $ODBC_INI_FILE
  echo "OMIT_SCHEMA = no" >> $ODBC_INI_FILE
  echo "CHARSET = ko_KR.utf8" >> $ODBC_INI_FILE
fi

export LD_LIBRARY_PATH=$DRIVER_INSTALL_DIR:/usr/local/lib:$LD_LIBRARY_PATH
cd linux_test
cmake CMakeLists.txt
make

echo "========== run link2u test =========="
./odbc_test | tee $SHELL_DIR/test_result_linux.txt
rm ./odbc_test
cd $SHELL_DIR

echo "========== test completed =========="

