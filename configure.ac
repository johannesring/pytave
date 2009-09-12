#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(pytave, 0.1.1, [[https://bugs.launchpad.net/pytave]])

# Some important programs.
AC_LANG(C++)
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CC

# Load our custom m4 scripts (from the m4 directory)
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_SRCDIR([pytave.cc])
AC_CONFIG_HEADER([config.h])

AC_PRESERVE_HELP_ORDER

AC_ARG_ENABLE(numpy,
	      [AS_HELP_STRING([--enable-numpy],
			      [use NumPy module (experimental)
			      @<:@default=check@:>@])],
			      [pytave_enable_numpy="$enableval"],
			      [pytave_enable_numpy=check])
 
pytave_libs_ok=

AX_OCTAVE([OCTAVE_CONFIG], [], [pytave_libs_ok=no])

# Pick a Python library to use
AX_PYTHON_DEVEL([], [], [pytave_libs_ok=no])

AS_IF(test "x$pytave_enable_numpy" != "xno",
[
   AX_PYTHON_NUMPY(
   [
       AC_DEFINE([HAVE_NUMPY], 1, [Define if using NumPy])
       pytave_enable_numpy=yes
   ],
   [
       AS_IF(test "x$pytave_enable_numpy" == "xyes", [pytave_libs_ok=no])
       pytave_enable_numpy='not found'
   ])
])

# Look for boost::python
pytave_old_libs="$LIBS"
pytave_old_ldflags="$LDFLAGS"
pytave_old_cppflags="$CPPFLAGS"
LIBS="$LIBS $PYTHON_LIBS"
LDFLAGS="$LDFLAGS $PYTHON_LDFLAGS"
CPPFLAGS="$CPPFLAGS $PYTHON_CPPFLAGS"

AX_BOOST_PYTHON([], [pytave_libs_ok=no])

LIBS="$pytave_old_libs"
LDFLAGS="$pytave_old_ldflags"
CPPFLAGS="$pytave_old_cppflags"

# Do some more initializations

AM_INIT_AUTOMAKE()

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([locale.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([uselocale], [pytave_have_uselocale=yes], [pytave_have_uselocale=no])

# This needs a more usable, less unusual solution.
AS_IF(test "x${prefix}" == "xNONE",
[
   PYTAVE_MODULE_INSTALL_PATH=/../invalidpath,
],
[
   PYTAVE_MODULE_INSTALL_PATH=${prefix}
])

# Substitutes for the Makefile/Jamfile
#AC_SUBST(OCTAVE_INCLUDEDIR)
#AC_SUBST(OCTAVE_LIBRARYDIR)
PYTAVE_OCTAVE_RPATH="$OCTAVE_LIBRARYDIR"
AC_SUBST(PYTAVE_OCTAVE_RPATH)
AC_SUBST(PYTAVE_MODULE_INSTALL_PATH)

# Substitutes for the Jamfile. XXX: Replace lib*.so with OS independent name.
AC_SUBST(JAM_LIBOCTAVE, $OCTAVE_LIBRARYDIR/liboctave.so)
AC_SUBST(JAM_LIBCRUFT, $OCTAVE_LIBRARYDIR/libcruft.so)
AC_SUBST(JAM_LIBOCTINTERP, $OCTAVE_LIBRARYDIR/liboctinterp.so)

# setup.py
AC_SUBST(PYTHON)

msg='One or more library dependencies could not be resolved.

Configuration failed. Halt.'

AS_IF(test -z "$pytave_libs_ok",
[
# Substitute in these files, copy project-root.jam to VPATH too
AC_OUTPUT([Makefile Jamfile setup.py project-root.jam])

chmod u+x "setup.py"
msg='Pytave is configured with the following setup'
])

AC_MSG_NOTICE([
========================================================================
$msg

Dependencies
  Octave ............. $OCTAVE_INCLUDEDIR
  Python ............. $PYTHON_CPPFLAGS
    executable ....... $PYTHON

Features
  NumPy .............. $pytave_enable_numpy
  uselocale .......... $pytave_have_uselocale

========================================================================])

AS_IF(test -n "$pytave_libs_ok",
[
	AC_MSG_ERROR([[Configuration failure. Halt.]])
])