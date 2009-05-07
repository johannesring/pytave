# -*- coding:utf-8 -*-
#
# Copyright 2008 David Grundberg, Håkan Fors Nilsson
#
# This file is part of Pytave.
#
# Pytave is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Pytave is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Pytave.  If not, see <http://www.gnu.org/licenses/>.

"""Python to Octave bridge"""

import _pytave

_pytave.init()
(OctaveError, ValueConvertError, ObjectConvertError, ParseError) \
				  = _pytave.get_exceptions();

def feval(nargout, funcname, *arguments):

	"""Executes an Octave function called funcname.

	The function is set to return nargout values. Returned values
	are stored in a tuple. If the nargout argument is less than 0,
	an empty tuple is returned.

	M-files are searched for in the Octave path.

	See also the Octave documentation for the builtin Octave function
	feval.

	Type conversions
	****************
	
	The following type conversions are supported:

	Python to Octave
	================
	
	Objects:
		int (32-bit)        int32
		float (64-bit)      double
		str                 string
		dict                struct
		list                cell
		
	Numeric Array:
		UBYTE, SBYTE,       matrix of correct type
		USHORT, SHORT,      -''-
		UINT, SINT,         -''-
		LONG,               -''-
		DOUBLE              -''-

	All other objects causes a pytave.ObjectConvertError to be
	raised. This exception inherits TypeError.
	
	Octave to Python
	================
	
	Scalar values to objects:
		bool                bool
		real scalar         float (64-bit)
		any string*         str
		struct              dict
		cell*               list

		* Cell arrays must be one-dimensional (row vector) and
                  character matrices must only have one row.  Any
                  other form will raise a ValueConvertError.
		
	Matrix values to Numeric arrays:
		int64               LONG
		int32, uint32       INT, UINT
		int16, uint16       SHORT, USHORT
		int8, unint8        SBYTE, UBYTE

	All other values causes a pytave.ValueConvertError to be
	raised. This exception inherits TypeError.

	Errors
	******

	Octave runtime errors are encapsulated into
	pytave.OctaveError exceptions, base class RuntimeError.

	"""

	return _pytave.feval(nargout, funcname, arguments)

def eval(nargout, code, silent=True):

	"""Executes a given Octave code.

	The expression is expected to return nargout values. Returned
	values are stored in a tuple. If the nargout argument is less
	than 0, an empty tuple is returned.

	All normal scope and function search rules apply. If silent is
	true (default), the result is not auto-printed, as if a
	semicolon was appended. Otherwise, auto-printing is enabled.

	See also the Octave documentation for the builtin Octave
	function eval.

	For information about returned value conversion, see
	pytave.feval.

	Errors
	******

	If the code cannot be parsed, a pytave.ParseError exception
	occurs.

	Octave runtime errors are encapsulated into pytave.OctaveError
	exceptions, base class RuntimeError.

	If the resulting values cannot be converted, a
	pytave.ValueConvertError is raised. This exception inherits
	TypeError.

	"""

	return _pytave.eval(nargout, code, silent)

def addpath(*arguments):
	"""See Octave documentation"""
	return _pytave.feval(1, "addpath", arguments)[0]

def rmpath(*paths):
	"""See Octave documentation"""
	return _pytave.feval(1, "rmpath", paths)[0]

def path(*paths):
	"""See Octave documentation"""
	return _pytave.feval(1, "path", paths)[0]

# Emacs
#	Local Variables:
#	fill-column:70
#	coding:utf-8
#	indent-tabs-mode:t
#	tab-width:8
#	End:
# vim: set textwidth=70 noexpandtab tabstop=3 :
