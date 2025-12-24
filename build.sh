#! /bin/bash

SHELL_DIR=$(pwd)
BUILD_DIR=$(pwd)/build
CCI_BUILD_DIR=$(pwd)/cci-src/build_x86_64_release
DRIVER_INSTALL_DIR=
BUILD_MODE=release

function show_usage ()
{
  echo "Usage: $0 [OPTIONS] [TARGET]"
  echo " OPTIONS"
  echo "  -m      Set build mode(release, debug) [default: release]"
  echo "  -i      Install directory [default: not copy driver files to install directory]"
  echo "  -? | -h Show this help message and exit"
  echo ""
  echo " TARGET"
  echo "  build   Build only (default)"
  echo ""
  echo " EXAMPLES"
  echo "  $0 -i /home/cubrid/cubrid-odbc/lib              # Build and copy driver files to /home/cubrid/cubrid-odbc/lib"
  echo "  $0 -m debug                                     # Debug build and package"
  echo ""
}

function get_options ()
{
  while getopts ":m:i:ph" opt; do
    case $opt in
      m ) BUILD_MODE="$OPTARG" ;;
      i ) DRIVER_INSTALL_DIR="$OPTARG" ;;
      h|\?|* ) show_usage; exit 1;;
    esac
  done
  shift $(($OPTIND - 1))

  case $BUILD_MODE in
    release|debug);;
    *) show_usage; exit 1;
  esac

}

get_options "$@"

echo "SHELL_DIR: $SHELL_DIR"
echo "BUILD_DIR: $BUILD_DIR"

if [ "x$DRIVER_INSTALL_DIR" = "x" ]; then
    echo "Install directory is not set"
else
  if [ ! -d "$DRIVER_INSTALL_DIR" ]; then
      echo "Install directory [$DRIVER_INSTALL_DIR] is not exist"
      echo "Please make directory [$DRIVER_INSTALL_DIR]"
      echo "e.g. mkdir -p $DRIVER_INSTALL_DIR"
      exit 1;
  fi
  echo "DRIVER_INSTALL_DIR: $DRIVER_INSTALL_DIR"
fi

if [ -d "$CCI_BUILD_DIR" ]; then
    rm -rf "$CCI_BUILD_DIR"
fi

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if [ "$build_mode" = "debug" ]; then
    cmake -DCMAKE_BUILD_TYPE=Debug ..
else
    cmake ..
fi

make
make package

if [ "x$DRIVER_INSTALL_DIR" != "x" ]; then
    cp -rfv $BUILD_DIR/libcascci.* "$DRIVER_INSTALL_DIR"
    cp -rfv $BUILD_DIR/libcubrid*.* "$DRIVER_INSTALL_DIR"
fi

cd "$SHELL_DIR"