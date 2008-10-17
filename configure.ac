#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(pytave, 0.1.1, c04dgg@cs.umu.se)

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

# Look for Octave package
AC_MSG_CHECKING([for Octave package])
AC_ARG_WITH([octave],
            [AS_HELP_STRING([--with-octave],
                            [Optionally set Octave package to use.
							@<:@default=check@:>@])],
            [with_octave=$withval],
            [with_octave=check])
          
AS_IF([test "x$with_octave" == xno],
[
   AC_MSG_FAILURE([--without-octave was given but Octave required])
], [])

AS_IF([test "x$with_octave" == xyes],
[
   with_octave=check
], [])

AS_IF([test "x$with_octave" == xcheck],
[
   AC_PATH_TOOL(octaveconfig, octave-config, [eepnotfound])
],
[
   AC_PATH_TOOL(octaveconfig, octave-config, [eepnotfound], $with_octave/bin)
])

AS_IF([test "x$octaveconfig" == "xeepnotfound"],
[
   AC_MSG_WARN([Could not find octave-config.])
],
[ 

   PYTAVE_OCTAVE_RPATH=`$octaveconfig -p OCTLIBDIR`
   PYTAVE_OCTAVE_INCLUDE_PATH=`$octaveconfig -p OCTINCLUDEDIR`
   AC_MSG_RESULT([  results of the Octave check:])
   AC_MSG_RESULT([    octave-config: $octaveconfig])
   AC_MSG_RESULT([    Library:       $PYTAVE_OCTAVE_RPATH])
   AC_MSG_RESULT([    Include Dir:   $PYTAVE_OCTAVE_INCLUDE_PATH])

   # Try to compile and link against Octave libraries.
   old_flags="$LDFLAGS"
   LDFLAGS="-L$PYTAVE_OCTAVE_RPATH $LDFLAGS"

   old_cxx="$CPPFLAGS"
   CPPFLAGS="-I$PYTAVE_OCTAVE_INCLUDE_PATH $CXXFLAGS"

   old_libs="$LIBS"
   LIBS="-loctave -lcruft -loctinterp $LIBS"

   AC_CACHE_CHECK([whether linking to Octave library works], [my_cv_lib_octave],
   [
      my_cv_lib_octave=no
	  AC_LANG_ASSERT(C++)
	  AC_LINK_IFELSE(AC_LANG_PROGRAM([[#include <octave/oct.h>
									   #include <octave/Matrix.h> ]],
									 [[MatrixType()]]),
						   [my_cv_lib_octave=yes],
						   [my_cv_lib_octave=no])
   ])


   LDFLAGS="$old_flags"
   LIBS="$old_libs"
   CPPFLAGS="$old_cpp"


])

# Look for boost::python
AX_PYTHON()
AX_BOOST_PYTHON()

# Now check the two vital libs, if they have problems, halt.
pytave_fatal_errors=no

AS_IF(test "x$my_cv_lib_octave" != "xyes",
[
   AC_MSG_WARN([Linking to Octave failed.])
   pytave_fatal_errors=yes
], [])

# XXX: hohoho, this is ugly.
AS_IF(test "x$ac_cv_boost_python" != "xyes",
[
   AC_MSG_WARN([Boost::Python test failed.])
   pytave_fatal_errors=yes
], [])


AS_IF(test "x$pytave_fatal_errors" == "xyes",
[
   AC_MSG_ERROR([Configuration failed. Halt.])
], [])


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
AC_SUBST(PYTAVE_OCTAVE_INCLUDE_PATH)
AC_SUBST(PYTAVE_OCTAVE_RPATH)
AC_SUBST(PYTAVE_MODULE_INSTALL_PATH)

# Substitutes for the Jamfile. XXX: Replace lib*.so with OS independent name.
AC_SUBST(JAM_LIBOCTAVE, $PYTAVE_OCTAVE_RPATH/liboctave.so)
AC_SUBST(JAM_LIBCRUFT, $PYTAVE_OCTAVE_RPATH/libcruft.so)
AC_SUBST(JAM_LIBOCTINTERP, $PYTAVE_OCTAVE_RPATH/liboctinterp.so)

# Substitute in these files
AC_OUTPUT([Makefile Jamfile setup.py])
# Copy project-root.jam to VPATH too
AC_OUTPUT([project-root.jam])
