dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libepc, 1.0)
dnl AC_PROG_CC
AC_PROG_CXX

dnl AC_CANONICAL_SYSTEM

dnl Initialize automake
AM_INIT_AUTOMAKE

dnl this allows us specify individual liking flags for each target
AM_PROG_CC_C_O 

dnl Initialize Libtool
dnl LT_INIT

dnl Check if Libtool is present
dnl Libtool is used for building share libraries 
dnl AC_PROG_LIBTOOL

AC_PROG_RANLIB

AC_CONFIG_FILES(Makefile
                src/Makefile
                include/Makefile
		exampleProgram/Makefile)

AC_CONFIG_COMMANDS([rapidjson],[git submodule update --init --recursive])

AC_OUTPUT
