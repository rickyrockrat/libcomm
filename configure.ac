#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([libcomm], [0.2.0])
AC_CONFIG_SRCDIR([src/libcomm/libcomm.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_CC
AC_PROG_INSTALL

AM_INIT_AUTOMAKE([dist-bzip2])
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcomm':
AC_CHECK_LIB([comm], [main])
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([getcwd gethostbyaddr gethostbyname gethostname gettimeofday memset socket])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/libcomm/Makefile])
AC_CONFIG_FILES([src/libcomm/structs/Makefile])
AC_CONFIG_FILES([src/tests/Makefile])

CXXFLAGS=

AC_MSG_CHECKING([whether to build with debug information])
  AC_ARG_ENABLE([debug],
      [AS_HELP_STRING([--enable-debug],
        [enable debug data generation (def=no)])],
      [debugit="$enableval"],
      [debugit=no])
AC_MSG_RESULT([$debugit])

if test x"$debugit" = x"yes"; then
  AC_DEFINE([DEBUG],[],[Debug Mode])
  AM_CXXFLAGS="$AM_CXXFLAGS -g -Wall -Werror -Wno-uninitialized"
else
  AC_DEFINE([NDEBUG],[],[No-debug Mode])
  AM_CXXFLAGS="$AM_CXXFLAGS -O3"
fi

AC_SUBST([AM_CXXFLAGS])

AC_OUTPUT
