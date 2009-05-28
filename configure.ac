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

pytave_libs_ok=

AX_OCTAVE([], [], [pytave_libs_ok=no])

# Pick a Python library to use
AX_PYTHON_DEVEL([], [], [pytave_libs_ok=no])

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

# Now check the libs, if they have problems, halt.
AS_IF(test -n "$pytave_libs_ok",
[
   AC_MSG_ERROR([
========================================================================
One or more library dependencies could not be resolved.

Configuration failed. Halt.
========================================================================])
])

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

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for library functions.

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

# Substitute in these files, copy project-root.jam to VPATH too
AC_OUTPUT([Makefile Jamfile setup.py project-root.jam])
