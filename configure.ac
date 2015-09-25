# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.61])

AC_INIT([fsmlite], [0.1.0], [tkemmer@computer.org])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
AM_SILENT_RULES([yes])

AC_PROG_CXX
AC_PROG_MKDIR_P
AC_PROG_INSTALL

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AX_CXXFLAGS_WARN_ALL
AX_CXX_COMPILE_STDCXX_11

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        tests/Makefile
        docs/Makefile
])

AC_OUTPUT
