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

import UserDict
import _pytave
import atexit
import sys

arg0 = sys.argv[0]
interactive = sys.stdin.isatty() and (arg0 == '' or arg0 == '-')

_pytave.init(interactive)
(OctaveError, ValueConvertError, ObjectConvertError, ParseError, \
 VarNameError) = _pytave.get_exceptions();

# Dynamic import. *Must* go after _pytave.init() !
__modname__ = _pytave.get_module_name()
if __modname__ == 'numpy':
    from numpy import oldnumeric as Numeric
elif __modname__ == 'Numeric':
    import Numeric
elif __modname__ == 'numarray':
    # FIXME: Is this OK?
    import numarray as Numeric
else:
    raise ImportError("Failed to import module: %s" % __modname__)

def _atexit():
	_pytave.atexit()

atexit.register(_atexit)

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
		str                 character array
		dict                struct
		list                cell array
		
	Numeric Array:
		UBYTE, SBYTE,       matrix of correct type
		USHORT, SHORT,      -''-
		UINT, SINT,         -''-
		LONG,               -''-
		DOUBLE              -''-
		CHAR                character array
		OBJECT              cell array

	All other objects causes a pytave.ObjectConvertError to be
	raised. This exception inherits TypeError.

	When dicts are converted, all keys must be strings and
	constitute valid Octave identifiers. The behavior is
	analogical to the Octave "struct" function: values that
	evaluate to cells must have matching dimensions, singleton
	cells and non-cell values are expanded.
	
	Octave to Python
	================
	
        All scalar values are regarded as 1x1 matrices, as they are in
	Octave.

	Matrix values to Numeric arrays:
	   	double              DOUBLE
		single              FLOAT
		logical             DOUBLE
		int64               LONG
		int32, uint32       INT, UINT
		int16, uint16       SHORT, USHORT
		int8, unint8        SBYTE, UBYTE
		char                CHAR
		cell                OBJECT

	Structs are converted to dicts, where each value is an OBJECT
	array.

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

def stripdict(dictarray):
    """A helper function to convert structures obtained from Octave.
    Because in Octave, all structs are also arrays, they are returned
    as dicts of object arrays. In the common case of a 1x1 struct, 
    stripdict strips the values."""
    
    sdict = {}
    for key in dictarray:
	sdict[key] = dictarray[key][0,0]
    return sdict

def narrowlist(objarray):
    """A helper function to convert cell arrays obtained from Octave.
    Octave cells are returned as Numeric object arrays. This function
    will flatten the array and convert it into a 1D list."""

    return Numeric.ravel(objarray).tolist()

def simplify(obj):
    """A helper function to convert results obtained from Octave.
    This will convert all 1x1 arrays to scalars, vectors to 1D arrays,
    1xN and 0x0 character arrays to strings, 1xN, Nx1 and 0x0 cell
    arrays to lists, and strip scalar dicts. It will work recursively."""

    def vectordims(dims,column_allowed = True):
	return (len(dims) == 2 and 
		((dims[0] == 1 or (column_allowed and dims[1] == 1)) or
		(dims[0] == 0 and dims[1] == 0)))

    if isinstance(obj,Numeric.ArrayType):
	tc = obj.typecode()
	if tc == 'O':
	    if vectordims(Numeric.shape(obj)):
		return map(simplify,narrowlist(obj))
	elif tc == 'c':
	    if vectordims(Numeric.shape(obj), False):
		return obj.tostring()
	else:
	    dims = Numeric.shape(obj)
	    if dims == (1,1):
		return obj.toscalar()
	    elif vectordims(dims):
		return Numeric.ravel(obj)
    elif isinstance(obj,dict):
	sobj = {}
	for key in obj:
	    sval = simplify(obj[key])
	    if isinstance(sval,list) and len(sval) == 1:
		sval = sval[0]
	    sobj[key] = sval
	return sobj
    elif isinstance(obj,tuple):
	return tuple(map(simplify,obj))
    ## default.
    return obj


def addpath(*arguments):
	"""See Octave documentation"""
	return _pytave.feval(1, "addpath", arguments)[0]

def rmpath(*paths):
	"""See Octave documentation"""
	return _pytave.feval(1, "rmpath", paths)[0]

def path(*paths):
	"""See Octave documentation"""
	return _pytave.feval(1, "path", paths)[0]

def load_package(pkg_name):
    """Equivalent to pkg load. See Octave documentation."""
    return _pytave.feval(0, "pkg", ("load", pkg_name))

def unload_package(pkg_name):
    """Equivalent to pkg unload. See Octave documentation."""
    return _pytave.feval(0, "pkg", ("unload", pkg_name))

class _VariablesDict(UserDict.DictMixin):
	def __init__(self, global_variables, native=False):
		self.global_variables = global_variables
		self.native = native

	def __getitem__(self, name):
		if not isinstance(name, basestring):
			raise TypeError('Expected a string, not a ' + repr(type(name)))
		try:
			return _pytave.getvar(name, self.global_variables)
		except VarNameError:
			raise KeyError('No Octave variable named ' + name)

	def __setitem__(self, name, value):
		if not isinstance(name, basestring):
			raise TypeError('Expected a string, not a ' + repr(type(name)))
		_pytave.setvar(name, value, self.global_variables)

	def __contains__(self, name):
		if not isinstance(name, basestring):
			raise TypeError('Expected a string, not a ' + repr(type(name)))
		return _pytave.isvar(name, self.global_variables)

	def __delitem__(self, name):
		if not isinstance(name, basestring):
			raise TypeError('Expected a string, not a ' + repr(type(name)))
		# Octave does not gripe when clearing non-existent
		# variables. To be consistent with Python dict
		# behavior, we shall do so.
		if self.__contains__(name):
			_pytave.delvar(name, self.global_variables)
		else:
			raise KeyError('No Octave variable named ' + name)


locals = _VariablesDict(global_variables=False)
globals = _VariablesDict(global_variables=True)

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
 
class _LocalScope:
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
	 return _LocalScope(func)

# Emacs
#	Local Variables:
#	fill-column:70
#	coding:utf-8
#	indent-tabs-mode:t
#	tab-width:8
#	python-indent:8
#	End:
# vim: set textwidth=70 noexpandtab tabstop=8 :
