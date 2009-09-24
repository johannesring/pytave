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

AC_ARG_WITH(numpy,
	      [AS_HELP_STRING([--with-numpy],
			      [use NumPy module (experimental)
			      @<:@default=yes@:>@])],
			      [pytave_enable_numpy="$enableval"],
			      [pytave_enable_numpy=yes])

AC_ARG_WITH(numeric,
	      [AS_HELP_STRING([--with-numeric],
			      [use legacy Numeric module. NumPy must be disabled.
			      @<:@default=no@:>@])],
			      [pytave_enable_numeric="$enableval"],
			      [pytave_enable_numeric=no])

dnl Define a convenience m4 macro for appending to pytave_missing_libs
define(addmissing,pytave_missing_libs="$1
$pytave_missing_libs")
pytave_missing_libs=

# Find Octave
AX_OCTAVE([OCTAVE_CONFIG], [], [addmissing([Octave development files])])

# Pick a Python library to use
AX_PYTHON_DEVEL([], [], [addmissing([Python development files])])

# Check for bad arguments.
AS_IF(test "x$pytave_enable_numpy" == "xyes" -a "x$pytave_enable_numeric" == "xyes" -o dnl
"x$pytave_enable_numpy" == "xno" -a "x$pytave_enable_numeric" == "xno", 
[
	AC_ERROR([NumPy or Numeric: select one.])
])

# Select Python matrix interface, prefer numpy.
AS_IF(test "x$pytave_enable_numpy" == "xno",
[
	pytave_matrix_interface="numeric"
],
[
	pytave_matrix_interface="numpy"
])

# Look for selected matrix interface
AS_IF(test "$pytave_matrix_interace" == "numpy",
[
   AX_PYTHON_NUMPY(
   [
       AC_DEFINE([HAVE_NUMPY], 1, [Define if using NumPy])
   ],
   [
       addmissing([NumPy development files])
   ])
],
[
   AX_PYTHON_NUMERIC([], [addmissing([Numeric development files])])
])

# Look for boost::python
pytave_old_libs="$LIBS"
pytave_old_ldflags="$LDFLAGS"
pytave_old_cppflags="$CPPFLAGS"
LIBS="$LIBS $PYTHON_LIBS"
LDFLAGS="$LDFLAGS $PYTHON_LDFLAGS"
CPPFLAGS="$CPPFLAGS $PYTHON_CPPFLAGS"

AX_BOOST_PYTHON([], [addmissing([Boost Python development files])])

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

msg="One or more library dependencies could not be resolved.

Missing dependencies:
$pytave_missing_libs
Configuration failed. Halt."

AS_IF(test -z "$pytave_missing_libs",
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
  Python boost ....... $BOOST_PYTHON_LIB

Features
  matrix interface ... $pytave_matrix_interface
  uselocale .......... $pytave_have_uselocale

========================================================================])

AS_IF(test -n "$pytave_missing_libs",
[
	AC_MSG_ERROR([[Configuration failure. Halt.]])
])