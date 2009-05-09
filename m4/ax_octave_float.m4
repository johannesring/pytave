# ===========================================================================
#
# ===========================================================================
#
# SYNOPSIS
#
#   AX_OCTAVE_FLOAT([ACTION_IF_FOUND], [ACTION_IF_NOT_FOUND])
#
# DESCRIPTION
#
#   Runs a simple test to determine whether or not the FloatNDArray
#   class is available.
#
#   Requires AX_OCTAVE.
#
# LAST MODIFICATION
#
#   2009-05-06
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

AC_DEFUN([AX_OCTAVE_FLOAT], [
	AC_REQUIRE([AX_OCTAVE])

	AS_IF([test -z "$ax_octave_ok" -a -n "$OCTAVE_CPPFLAGS"], [
		AC_CACHE_CHECK([whether Octave has float support],
		[ax_octave_float_cv_supported],
		[
			ax_octave_float_cv_supported=no

			AC_LANG_PUSH([C++])

			ax_octave_old_ldflags="$LDFLAGS"
			ax_octave_old_cppflags="$CPPFLAGS"
			ax_octave_old_libs="$LIBS"
			LDFLAGS="$OCTAVE_LDFLAGS $LDFLAGS"
			CPPFLAGS="$OCTAVE_CPPFLAGS $CPPFLAGS"
			LIBS="$OCTAVE_LIBS $LIBS"

			AC_LANG_ASSERT(C++)
			AC_COMPILE_IFELSE(
			AC_LANG_PROGRAM(
				[[#include <octave/oct.h>
				#include <octave/Matrix.h> ]],
				[[FloatNDArray()]]),
				[ax_octave_float_cv_supported=yes],
				[ax_octave_float_cv_supported=no])
			LDFLAGS="$ax_octave_old_ldflags"
			CPPFLAGS="$ax_octave_old_cppflags"
			LIBS="$ax_octave_old_libs"

			AC_LANG_POP([C++])
		])
	])

	# Execute ACTION_IF_FOUND or ACTION_IF_NOT_FOUND
	if test "x$ax_octave_float_cv_supported" = "xyes"; then
		m4_ifvaln([$1],[$1],[:])dnl
		m4_ifvaln([$2],[else $2])dnl
	fi
])
