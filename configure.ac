dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(sqlite3_rsync, 0.1)

m4_include([src/m4/ax_cxx_compile_stdcxx_11.m4])
dnl AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([subdir-objects])

AC_PROG_CXX

# C++11
AC_LANG_CPLUSPLUS
# check C++11 compliance and flags
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

# libraries:
AC_SEARCH_LIBS([pthread_create], [pthread])

# SQLITE3:
SQLITE3_CFLAGS=`pkg-config sqlite3 --cflags`
SQLITE3_LIBS=`pkg-config sqlite3 --libs`

CFLAGS="${CFLAGS} ${SQLITE3_CFLAGS}"
LIBS="${LIBS} ${SQLITE3_LIBS}"

# Checks for libs
AC_CHECK_LIB(sqlite3,sqlite3_open_v2,,AC_MSG_ERROR([*** Can't find sqlite3]))

# Packages (.pc)
#PKG_CHECK_MODULES([libsqlite3],[sqlite3])


#debug:

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")


AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT
