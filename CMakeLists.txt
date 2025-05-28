#
#
#  Copyright 2016 CUBRID Corporation
# 
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# 
#
message(STATUS "=========== Linux ODBC ===========")

CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

PROJECT(cubrid-odbc)

find_package(Git)

set(CUBRID_ODBC_UNICODE "${CMAKE_PROJECT_NAME}w")

set(WITH_EXTERNAL_PREFIX "EXTERNAL")
set(WITH_LIBUNIXODBC_URL "https://github.com/CUBRID/3rdparty/raw/develop/unixODBC/unixODBC-2.3.9.tar.gz")
set(WITH_LIBUNIXODBC  ${WITH_EXTERNAL_PREFIX})

set(CUBRID_ODBC_SOURCES
  odbc_linux.c
  odbc_catalog.c
  odbc_connection.c
  odbc_descriptor.c
  odbc_diag_record.c
  odbc_env.c
  odbc_interface.c
  odbc_number.c
  odbc_result.c
  odbc_statement.c
  odbc_type.c
  odbc_util.c
  )
set(CUBRID_ODBC_UNICODE_SOURCES ${CUBRID_ODBC_SOURCES} unicode.c)

set(VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/odbc_version.i")
set(CCI_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/cci-src")
set(CCI_COPYING "${CCI_SOURCE}/COPYING")
set(CCI_LIBDIR "${CCI_SOURCE}/build_x86_64_release/cci")
set(CCI_SO_LIB "${CCI_LIBDIR}/libcascci.so")

include(ExternalProject)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/BUILD_NUMBER)
  set (CUBRID_ODBC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/BUILD_NUMBER)

file(STRINGS ${CUBRID_ODBC_SOURCE_DIR}/BUILD_NUMBER CUBRID_ODBC_VERSION)

if(CUBRID_ODBC_VERSION MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+(\\.[0-9]+)?(-[a-zA-Z0-9_]+)?")
  string(REGEX MATCHALL "[0-9]+|-([A-Za-z0-9_]+)" VERSION_MATCHES ${CUBRID_ODBC_VERSION})
  list(LENGTH VERSION_MATCHES VERSION_MATCHES_LENGTH)
  list(GET VERSION_MATCHES 0 CUBRID_ODBC_MAJOR_VERSION)
  list(GET VERSION_MATCHES 1 CUBRID_ODBC_MINOR_VERSION)
  list(GET VERSION_MATCHES 2 CUBRID_ODBC_PATCH_VERSION)
else()
  message(FATAL_ERROR "Failed to parse a version string from ${VERSION_FILE} file")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/.git" AND GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-list --count --all
	COMMAND awk "{ printf \"%04d\", $1 }"
	OUTPUT_VARIABLE commit_count RESULT_VARIABLE git_result
	ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

file(WRITE  ${VERSION_FILE} "#define MAJOR_VERSION ${CUBRID_ODBC_MAJOR_VERSION}\n")
file(APPEND ${VERSION_FILE} "#define MINOR_VERSION ${CUBRID_ODBC_MINOR_VERSION}\n")
file(APPEND ${VERSION_FILE} "#define PATCH_VERSION ${CUBRID_ODBC_PATCH_VERSION}\n")
file(APPEND ${VERSION_FILE} "#define BUILD_SERIAL_NUMBER ${commit_count}\n")

file(APPEND ${VERSION_FILE} "#define VERSION_STRING \"${CUBRID_ODBC_MAJOR_VERSION}")
file(APPEND ${VERSION_FILE} ".${CUBRID_ODBC_MINOR_VERSION}")
file(APPEND ${VERSION_FILE} ".${CUBRID_ODBC_PATCH_VERSION}")
file(APPEND ${VERSION_FILE} ".${commit_count}\"\n")

file(APPEND ${VERSION_FILE} "#ifdef CUBRID_ODBC_UNICODE\n")
file(APPEND ${VERSION_FILE} "#define LINUX_ODBC_DRIVER_NAME \"lib${CUBRID_ODBC_UNICODE}.so.${CUBRID_ODBC_VERSION}\"\n")
file(APPEND ${VERSION_FILE} "#else\n")
file(APPEND ${VERSION_FILE} "#define LINUX_ODBC_DRIVER_NAME \"lib${CMAKE_PROJECT_NAME}.so.${CUBRID_ODBC_VERSION}\"\n")
file(APPEND ${VERSION_FILE} "#endif\n")

# cci submodule update
if (NOT EXISTS ${CCI_COPYING})
	if (UNIX)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule deinit -f ${CCI_SOURCE}
        COMMAND ${GIT_EXECUTABLE} submodule update --init
        RESULT_VARIABLE git_result)
        if (git_result)
          message(FATAL "Could not update cubrid-cci submodule")
        endif(git_result)
    endif(UNIX)
endif(NOT EXISTS ${CCI_COPYING})

if(NOT EXISTS ${CCI_SO_LIB})
  execute_process(
  	COMMAND bash build.sh
  	WORKING_DIRECTORY ${CCI_SOURCE}
  	)
endif(NOT EXISTS ${CCI_SO_LIB})

if(EXISTS ${CCI_SO_LIB})
  execute_process(
      COMMAND readlink ${CCI_SO_LIB}
      OUTPUT_VARIABLE CCI_SO_REAL_FILE_NAME
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE readlink_result
      ERROR_VARIABLE readlink_error
      WORKING_DIRECTORY ${CCI_SOURCE}
    )
  if(readlink_result EQUAL 0)
    execute_process(
      COMMAND cp ${CCI_LIBDIR}/${CCI_SO_REAL_FILE_NAME} ${CMAKE_BINARY_DIR}/
      COMMAND cp -L -a ${CCI_SO_LIB} ${CMAKE_BINARY_DIR}/
      WORKING_DIRECTORY ${CCI_SOURCE}
      )
  else(readlink_result EQUAL 0)
    message(FATAL_ERROR "Failed to readlink -f ${CCI_SO_LIB}")
  endif(readlink_result EQUAL 0)
else(EXISTS ${CCI_SO_LIB})
  message(FATAL_ERROR "Failed to build cubrid-cci or ${CCI_SO_LIB} not found")
endif(EXISTS ${CCI_SO_LIB})

if(WITH_LIBUNIXODBC STREQUAL "EXTERNAL")
  set(LIBUNIXODBC_TARGET libodbc)
  if(UNIX)
    #compile unixODBC library given an internet url to a unixODBC archive
    #e.g. http://www.unixodbc.org/unixODBC-2.3.9.tar.gz
    externalproject_add(${LIBUNIXODBC_TARGET}
      URL                  ${WITH_LIBUNIXODBC_URL}
      TIMEOUT              600
      DOWNLOAD_NO_PROGRESS 1
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
      CONFIGURE_COMMAND    <SOURCE_DIR>/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/external # ${DEFAULT_CONFIGURE_OPTS}
      BUILD_COMMAND        $(MAKE) AUTOCONF=: AUTOHEADER=: AUTOMAKE=: ACLOCAL=:
      INSTALL_COMMAND      $(MAKE) install AUTOCONF=: AUTOHEADER=: AUTOMAKE=: ACLOCAL=:
      )
    set(LIBUNIXODBC_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/external/include)
  endif(UNIX)
else(WITH_LIBUNIXODBC STREQUAL "EXTERNAL")
    message(FATAL_ERROR "Invalid WITH_LIBUNIXODBC option value ${WITH_LIBUNIXODBC}")
endif(WITH_LIBUNIXODBC STREQUAL "EXTERNAL")

message(STATUS "BIN = ${CMAKE_CURRENT_BINARY_DIR}")
set (CMAKE_CXX_FLAGS "-Wall")
set (CMAKE_EXE_LINK_FLAGS "-m64 -Wall")
set (CMAKE_include_directories_BEFORE ON)

ADD_COMPILE_OPTIONS(-m64 -Wall)

INCLUDE_DIRECTORIES(${CCI_SOURCE}/src/cci ${LIBUNIXODBC_INCLUDES})

add_definitions(-DGCC -DLINUX -DCUBRID_ODBC_UNICODE)

LINK_DIRECTORIES(${CCI_LIBDIR})
ADD_LIBRARY(${CMAKE_PROJECT_NAME} SHARED ${CUBRID_ODBC_SOURCES})
ADD_LIBRARY(${CUBRID_ODBC_UNICODE} SHARED ${CUBRID_ODBC_UNICODE_SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} cascci pthread stdc++)
SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME} PROPERTIES SOVERSION ${CUBRID_ODBC_VERSION})
target_link_libraries(${CUBRID_ODBC_UNICODE} cascci pthread stdc++)
SET_TARGET_PROPERTIES(${CUBRID_ODBC_UNICODE} PROPERTIES SOVERSION ${CUBRID_ODBC_VERSION})

message("Target directory: ${CMAKE_BINARY_DIR}/package/lib")
add_custom_target(package
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/package/lib
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib${CMAKE_PROJECT_NAME}.so.${CUBRID_ODBC_VERSION} ${CMAKE_BINARY_DIR}/package/lib/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib${CUBRID_ODBC_UNICODE}.so.${CUBRID_ODBC_VERSION} ${CMAKE_BINARY_DIR}/package/lib/
  COMMAND ${CMAKE_COMMAND} -E create_symlink lib${CMAKE_PROJECT_NAME}.so.${CUBRID_ODBC_VERSION} ${CMAKE_BINARY_DIR}/package/lib/lib${CMAKE_PROJECT_NAME}.so
  COMMAND ${CMAKE_COMMAND} -E create_symlink lib${CUBRID_ODBC_UNICODE}.so.${CUBRID_ODBC_VERSION} ${CMAKE_BINARY_DIR}/package/lib/lib${CUBRID_ODBC_UNICODE}.so
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${CCI_SO_REAL_FILE_NAME} ${CMAKE_BINARY_DIR}/package/lib/
  COMMAND ${CMAKE_COMMAND} -E create_symlink  ${CMAKE_BINARY_DIR}/${CCI_SO_REAL_FILE_NAME} ${CMAKE_BINARY_DIR}/package/lib/libcascci.so
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/README.txt ${CMAKE_BINARY_DIR}/package/README.txt
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/package ${CMAKE_COMMAND} -E tar czvf ../CUBRID-ODBC-Linux-${CMAKE_SYSTEM_PROCESSOR}-${CUBRID_ODBC_VERSION}.tar.gz --format=gnutar lib README.txt
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Creating CUBRID-ODBC-Linux-${CMAKE_SYSTEM_PROCESSOR}-${CUBRID_ODBC_VERSION}.tar.gz"
)
