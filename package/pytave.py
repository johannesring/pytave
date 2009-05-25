# -*- coding:utf-8 -*-
#
# Copyright 2008 David Grundberg, Håkan Fors Nilsson
# Copyright 2009 Jaroslav Hajek, VZLU Prague
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
import sys

arg0 = sys.argv[0]
interactive = sys.stdin.isatty() and (arg0 == '' or arg0 == '-')

_pytave.init(interactive)
(OctaveError, ValueConvertError, ObjectConvertError, ParseError, \
 VarNameError) = _pytave.get_exceptions();

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

def getvar(name, fglobal = False):
	 """Queries a variable by name from the current Octave scope.
	 This is pretty much equivalent to calling eval(name), but is
	 much faster because the Octave parser is bypassed. 
	 
	 global specifies that a global variable should be looked up;
	 otherwise, local variable (in the current scope) is always
	 searched for.

	 If the variable is not defined, VarNameError exception is raised.
	 """
	 return _pytave.getvar(name, fglobal)

def setvar(name, val, fglobal = False):
	 """Sets a variable by name from the current Octave scope.
	 It is quite fast because the Octave parser is bypassed. 

	 global specifies that a global variable should be assigned to;
	 otherwise, local variable (in the current scope) is always
	 searched for.

	 If the variable is not defined, a new variable is created.

	 If the variable name is not valid, VarNameError exception is raised.
	 """

	 return _pytave.setvar(name, val, fglobal)

def isvar(name, fglobal = False):
	 """Checks whether a variable exists in the current Octave scope.
	 It is quite fast because the Octave parser is bypassed. 

	 global specifies that a global variable should be looked up;
	 otherwise, local variable (in the current scope) is always
	 searched for.

	 If the variable is defined, returns True, otherwise returns False.
	 """

	 return _pytave.isvar(name, fglobal)

def push_scope():
	 """Creates a new anonymous local variable scope on the Octave call
	 stack and sets it as the current Octave scope. Subsequent eval,
	 getvar and setvar calls will affect variables within this scope.

	 This is useful to do if you call the Octave engine from within
	 multiple Python functions, to prevent them from hampering each
	 other's data. As such, it is advisable to always create a local
	 scope in a production code.
	 """
	 return _pytave.push_scope()

def pop_scope():
	 """Pops the current active scope (created previously by
	 push_scope) off the Octave call stack. The previous scope
	 will become the active scope.

	 If already at the top-level scope, this function does nothing.
	 """
	 _pytave.pop_scope()
 
class _local_scope:
	 def __init__(self, func):
		  self.func = func
		  self.__name__ = func.__name__
		  self.__doc__ = func.__doc__

	 def __call__(self, *args, **kwargs):
		  try:
				_pytave.push_scope()
				return self.func(*args, **kwargs)
		  finally:
				_pytave.pop_scope()

def local_scope(func):
	 """Decorates a function to use local Octave scope.
	 Example:

	 @pytave.local_scope
	 def myfunc(a,b):
		  <function body>

	 is equivalent to:

	 def myfunc(a,b):
		  try:
				pytave.push_scope()
				<function body>
		  finally:
			  pytave.pop_scope()
	 """
	 return _local_scope(func)

# Emacs
#	Local Variables:
#	fill-column:70
#	coding:utf-8
#	indent-tabs-mode:t
#	tab-width:8
#	End:
# vim: set textwidth=70 noexpandtab tabstop=3 :
