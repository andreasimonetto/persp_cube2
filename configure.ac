# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([persp_cube2], [0.3])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_LEX

AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([X11], [XDrawLine])
AC_CHECK_LIB([Xext], [main])
AC_CHECK_LIB([Xpm], [main])
AC_CHECK_LIB([m], [floor])

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([limits.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor strcasecmp strdup strtoul])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
