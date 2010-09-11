# ===========================================================================
#
# ===========================================================================
#
# SYNOPSIS
#
#   AX_OCTAVE([OCTAVE_CONFIG], [ACTION_IF_FOUND], [ACTION_IF_NOT_FOUND])
#
# DESCRIPTION
#
#   This macro does a Octave development environment check.
#
#   It accepts one optional argument, OCTAVE_CONFIG.  This is the full
#   path the the octave-config used to find out the environment.
#
#   If OCTAVE_CONFIG is not set, or empty, a octave-config executable
#   is searched for using AC_PATH_TOOL.  If the executable is not
#   found, or does not return results, the Octave interpreter will be
#   used instead.
#
#   As a final check, a simple test program is compiled and linked
#   against the found Octave installation.  If the check is
#   successful, ACTION_IF_FOUND is executed, otherwise
#   ACTION_IF_NOT_FOUND.
#
#   AX_OCTAVE substitutes OCTAVE_CPPFLAGS, OCTAVE_LDFLAGS and
#   OCTAVE_LIBS, along with the optional OCTAVE_LIBRARYDIR and
#   OCTAVE_INCLUDEDIR.
#
#   Typical usage:
#
#	AX_OCTAVE([], [], [
#		AC_MSG_ERROR([[Octave required but not available]])
#		])
#	LIBS="$LIBS $OCTAVE_LIBS"
#	CPPFLAGS="$CPPFLAGS $OCTAVE_CPPFLAGS"
#	LDFLAGS="$LDFLAGS $OCTAVE_LDFLAGS"
#
# LAST MODIFICATION
#
#   2009-05-04
#
# COPYING
#
#   Copyright (c) 2009 David Grundberg
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Macro Archive. When you make and
#   distribute a modified version of the Autoconf Macro, you may extend this
#   special exception to the GPL to apply to your modified version as well.

AC_DEFUN([AX_OCTAVE],[
	# These are the variables AX_OCTAVE uses
	OCTAVE_LDFLAGS=
	OCTAVE_LIBS=
	OCTAVE_CPPFLAGS=
	OCTAVE_INCLUDEDIR=
	OCTAVE_LIBRARYDIR=
	ax_octave_config="[$]$1"
	ax_octave_ok=

	AC_MSG_CHECKING([for octave-config filename])
	AS_IF([test -z "$ax_octave_config"], [
		AC_MSG_RESULT([determined from path])
		AC_PATH_TOOL([ax_octave_config], [octave-config], [])
		AS_IF([test -z "$ax_octave_config"], [
			AC_MSG_WARN([Could not find octave-config.])
		], [])
	], [
		AC_MSG_RESULT([specified $ax_octave_config])
	])

	AS_IF([test -n "$ax_octave_config"], [
		AC_MSG_CHECKING([for Octave library path])
		OCTAVE_LIBRARYDIR=`$ax_octave_config -p OCTLIBDIR`
		AC_MSG_RESULT([$OCTAVE_LIBRARYDIR])

		AC_MSG_CHECKING([for Octave include path])
		OCTAVE_INCLUDEDIR=`$ax_octave_config -p OCTINCLUDEDIR`
		AC_MSG_RESULT([$OCTAVE_INCLUDEDIR])
	])

	AS_IF([[test -z "$OCTAVE_LIBRARYDIR" -o -z "$OCTAVE_INCLUDEDIR"]], [
		AC_MSG_WARN([[
========================================================================
Octave library or include path not found using octave-config, trying
Octave interpreter.

This could be a sign that the Octave development package is missing.
========================================================================]])

		OCTAVE_LIBRARYDIR=
		OCTAVE_INCLUDEDIR=

		AC_MSG_NOTICE([[checking for Octave interpreter]])
		AC_PATH_TOOL([ax_octave_interpreter], [octave], [])
		AS_IF([test -z "$ax_octave_interpreter"], [
			AC_MSG_WARN([Could not find Octave interpreter.])
		], [
			AC_MSG_CHECKING([for Octave library path (alt)])
			OCTAVE_LIBRARYDIR=`$ax_octave_interpreter -q --eval "printf(octave_config_info.octlibdir)"`
			AC_MSG_RESULT([$OCTAVE_LIBRARYDIR])

			AC_MSG_CHECKING([for Octave include path (alt)])
			OCTAVE_INCLUDEDIR=`$ax_octave_interpreter -q --eval "printf(octave_config_info.octincludedir)"`
			AC_MSG_RESULT([$OCTAVE_INCLUDEDIR])
		])

		AS_IF([[test -z "$OCTAVE_LIBRARYDIR" -o -z "$OCTAVE_INCLUDEDIR"]], [
			ax_octave_ok=no
		])
	])

	AS_IF([test -z "$ax_octave_ok"], [
		# After the 3.2 series, the include path ends with
		# /octave, but that part we don't want.
		AC_MSG_CHECKING([[if the include directory is 3.3+ style]])
		if test -f "$OCTAVE_INCLUDEDIR/oct.h" ; then
			OCTAVE_INCLUDEDIR="$OCTAVE_INCLUDEDIR/.."
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
		fi

		OCTAVE_LDFLAGS="-L$OCTAVE_LIBRARYDIR"
		OCTAVE_LIBS="-loctave -lcruft -loctinterp"
		OCTAVE_CPPFLAGS="-I$OCTAVE_INCLUDEDIR"

		AC_CACHE_CHECK([whether linking to Octave works], [ax_octave_cv_lib_octave],
		[
			ax_octave_cv_lib_octave=no

			AC_LANG_PUSH([C++])

			ax_octave_old_ldflags="$LDFLAGS"
			ax_octave_old_cppflags="$CPPFLAGS"
			ax_octave_old_libs="$LIBS"

			LDFLAGS="$OCTAVE_LDFLAGS $ax_octave_old_ldflags"
			CPPFLAGS="$OCTAVE_CPPFLAGS $ax_octave_old_cppflags"
			LIBS="$OCTAVE_LIBS $ax_octave_old_libs"

			AC_LANG_ASSERT(C++)
			AC_LINK_IFELSE(
			AC_LANG_PROGRAM(
				[[#include <octave/oct.h>
				#include <octave/Matrix.h> ]],
				[[MatrixType()]]),
				[ax_octave_cv_lib_octave=yes],
				[ax_octave_cv_lib_octave=no])

			LDFLAGS="$ax_octave_old_ldflags"
			CPPFLAGS="$ax_octave_old_cppflags"
			LIBS="$ax_octave_old_libs"

			AC_LANG_POP([C++])
		])
		AS_IF([test "x$ax_octave_cv_lib_octave" != "xyes"], [
			ax_octave_ok=no
		])
	])

	AS_IF([test -n "$ax_octave_ok"], [
		OCTAVE_LDFLAGS=
		OCTAVE_LIBS=
		OCTAVE_CPPFLAGS=
		OCTAVE_INCLUDEDIR=
		OCTAVE_LIBRARYDIR=

		AC_MSG_WARN([[
========================================================================
Can not link with Octave.

Make sure the Octave development package is installed.
========================================================================]])
	])

	AC_SUBST([OCTAVE_LDFLAGS])
	AC_SUBST([OCTAVE_CPPFLAGS])
	AC_SUBST([OCTAVE_LIBS])
	AC_SUBST([OCTAVE_INCLUDEDIR])
	AC_SUBST([OCTAVE_LIBRARYDIR])

	# Execute ACTION_IF_FOUND or ACTION_IF_NOT_FOUND
	if test -z "$ax_octave_ok" ; then
		m4_ifvaln([$2],[$2],[:])dnl
		m4_ifvaln([$3],[else $3])dnl
	fi

])
