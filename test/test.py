#!/usr/bin/python
# -*- coding:utf-8 -*-

import pytave
from pytave import Numeric
import numpy
import traceback

print "No messages indicates test pass."

arr0_0 = Numeric.zeros((0,0));
arr0_1 = Numeric.zeros((0,1));
arr1_0 = Numeric.zeros((1,0));
number = Numeric.array([[1.32]], Numeric.Float32)
arr1fT = Numeric.array([[1.32], [2], [3], [4]], Numeric.Float32)
arr1fT2 = Numeric.array([[1.32, 2, 3, 4]], Numeric.Float32)
arr1f = Numeric.array([[1.32, 2, 3, 4]], Numeric.Float32)
arr1b = Numeric.array([[8, 2, 3, 256]], Numeric.Int8)
arr1i = Numeric.array([[17, 2, 3, 4]], Numeric.Int)
arr1i32 = Numeric.array([[32, 2, 3, 4]], Numeric.Int32)
arr1i64 = numpy.array([[32, 2, 3, 4]], dtype=numpy.int64)
arr1a = Numeric.array([[1, 2, 3, 4]])
arr2f = Numeric.array([[1.32, 2, 3, 4],[5,6,7,8]], Numeric.Float32)
arr2d = Numeric.array([[1.17, 2, 3, 4],[5,6,7,8]], Numeric.Float)
arr3f = Numeric.array([[[1.32, 2, 3, 4],[5,6,7,8]],[[9, 10, 11, 12],[13,14,15,16]]], Numeric.Float32)
arr1c = Numeric.array([[1+2j, 3+4j, 5+6j, 7+0.5j]], Numeric.Complex)
arr1fc = Numeric.array([[1+2j, 3+4j, 5+6j, 7+0.5j]], Numeric.Complex32)
arr1ch = Numeric.array(["abc"],Numeric.Character)
arr2ch = Numeric.array(["abc","def"],Numeric.Character)
arr1o = Numeric.array([[1.0,"abc",2+3j]],Numeric.PyObject)
arr2o = Numeric.array([[1.0,"abc",2+3j],[4.0,arr1i,"def"]],Numeric.PyObject)

alimit_int64 = numpy.array([[-9223372036854775808L, 9223372036854775807L]], dtype=numpy.int64);
alimit_int32 = Numeric.array([[-2147483648, 2147483647]], Numeric.Int32);
alimit_int16 = Numeric.array([[-32768, 32767, -32769, 32768]], Numeric.Int16);
alimit_int8 = Numeric.array([[-128, 127, -129, 128]], Numeric.Int8);
alimit_uint8 = Numeric.array([[0, 255, -1, 256]], Numeric.UnsignedInt8);


# This eval call is not to be seen as a encouragement to use Pytave
# like this. Create a separate .m-file with your complex Octave code.
pytave.eval(0, "function [result] = test_return(arg); result = arg; endfunction")

pytave.feval(1, "test_return", 1)

def equals(a,b):
    return Numeric.alltrue(Numeric.ravel(a == b))

def fail(msg, exc=None):
	print "FAIL:", msg
	traceback.print_stack()
	if exc is not None:
		traceback.print_exc(exc)
	print ""

def testequal(value):
	try:
		nvalue, = pytave.feval(1, "test_return", value)
		if not equals(value, nvalue):
			fail("as %s != %s" % (value, nvalue))
	except TypeError, e:
		fail(value, e)

def testexpect(value, expected):
	try:
		nvalue, = pytave.feval(1, "test_return", value)
		if not equals(value, nvalue):
			fail("sent in %s, expecting %s, got %s", (value, expected, nvalue))
	except TypeError, e:
		fail(value, e)

def testmatrix(value):
	try:
		nvalue, = pytave.feval(1, "test_return", value)
		class1, = pytave.feval(1, "class", value)
		class1 = class1.tostring()
		class2, = pytave.feval(1, "class", nvalue)
		class2 = class2.tostring()
		if not equals(value, nvalue):
			fail("as %s != %s" % (value, nvalue))
		if value.shape != nvalue.shape:
			fail("Size check failed for: %s. Expected shape %s, got %s  with shape %s" \
			%(value, value.shape, nvalue, nvalue.shape))
		if class1 != class2:
			fail( "Type check failed for: %s. Expected %s. Got %s."
			%(value, class1, class2))
	except TypeError, e:
		fail("Execute failed: %s" % value, e)

def testobjecterror(value):
	try:
		pytave.feval(1, "test_return", value);
		print "FAIL:", (value,)
	except pytave.ObjectConvertError:
		pass
	except Exception, e:
		print "FAIL", (value,), e

def testvalueerror(*value):
	try:
		pytave.feval(1, *value);
		fail(value)
	except pytave.ValueConvertError:
		pass
	except Exception, e:
		fail(value, e)

def testparseerror(*value):
	try:
		pytave.eval(*value);
		print "FAIL:", (value,)
	except pytave.ParseError:
		pass
	except Exception, e:
		print "FAIL", (value,), e

def testvalueok(*value):
	try:
		pytave.feval(1, *value);
	except Exception, e:
		print "FAIL", (value,), e

def testevalexpect(numargout, code, expectations):
	try:
		results = pytave.eval(numargout, code);
		if not equals(results, expectations):
			fail("eval: %s : because %s != %s" % (code, results, expectations))
	except Exception, e:
		fail("eval: %s" % code, e)

def testsetget(variables, name, value):
	try:
		variables[name] = value
		if name not in variables:
			fail("set/get: %s: Should exist, not there." % name)
		result, = pytave.feval(1, "isequal", value, variables[name])
		if not result:
			fail("set/get: %s -> %s: results diverged" % (name, value))
	except Exception, e:
		fail("set/get: %s" % name, e)

def testexception(exception, func):
	try:
		func()
		fail("Expecting %s but nothing was raised." % repr(exception))
	except Exception, e:
		if not isinstance(e, exception):
			fail("Expecting %s but got %s instead" % (repr(exception), repr(e)), e)

def testlocalscope(x):

    @pytave.local_scope
    def sloppy_factorial(x):
	pytave.locals["x"] = x
	xm1, = pytave.eval(1,"x-1")
	xm1 = xm1.toscalar()
	if xm1 > 0:
	    fxm1 = sloppy_factorial(xm1)
	else:
	    fxm1 = 1
	pytave.locals["fxm1"] = fxm1
	fx, = pytave.eval(1,"x * fxm1")
	fx = fx.toscalar()
	return fx

    try:
	fx = sloppy_factorial(x)
	fx1 = 1.0
	for k in range(1,x+1):
	    fx1 = k * fx1
	if fx != fx1:
	    fail('testlocalscope: result incorrect')
    except Exception, e:
	fail("testlocalscope: %s" % (x,), e)

def objarray(obj):
    return Numeric.array(obj,Numeric.PyObject)

def charray(obj):
    return Numeric.array(obj,Numeric.Character)


testmatrix(alimit_int64)
testmatrix(alimit_int32)
testmatrix(alimit_int16)
testmatrix(alimit_int8)

# Strings

#FIXME: These tests are not working.
#testequal(["mystring"])
#testequal(["mystringåäöÅÄÖ"])

testexpect(1,Numeric.array([[1]],Numeric.Int))
testexpect(1.0,Numeric.array([[1]],Numeric.Float))

# Vector arrays
testmatrix(arr1a)
testmatrix(arr1f)
testmatrix(arr1fT)
testmatrix(arr1fT2)
testmatrix(arr1i)
testmatrix(arr1b)
testmatrix(arr1fc)

# 2d arrays
testmatrix(arr2f)
testmatrix(arr2d)
testmatrix(arr2ch)

# 3d arrays
testmatrix(arr3f)

# Note, both arr0_0 == arr0_0 and arr0_0 != arr0_0 are false!
if (arr0_0 != arr0_0) or (arr0_0 == arr0_0):
	print "FAIL: Zero test", 

testmatrix(arr0_0)
testmatrix(arr0_1)
testmatrix(arr1_0)

# Lists
testexpect([1, 2],objarray([[1,2]]))
testexpect([],objarray([[]]))

# Return cells with OK dimensions
testvalueok("cell", 1, 3);
testvalueok("cell", 1, 0)
testvalueok("cell", 0, 0)
testvalueok("cell", 3, 1)
testvalueok("cell", 0, 1)

# Dictionaries


# Simple dictionary tests
testexpect({"foo": 1, "bar": 2},
	   {"foo": objarray([[1]]), "bar": objarray([[2]])})
#testexpect({"x": [1, 3], "y": [2, 4]},
#	   {"x": objarray([[1,3]]), "y": objarray([[2,4]])})
# just constructing the second value with Numeric 24.2!
#testexpect({"x": [1, "baz"], "y": [2, "foobar"]},
#          {"x": objarray([[1, charray(["baz"])]]), 
#	   "y": objarray([[2, charray(["foobar"])]])})

testequal({"x": objarray([[arr1f]]), "y": objarray([[arr1i]])})
testequal({})
testequal({"foo": arr2o,    "bar": arr2o})

# Try some invalid keys
testobjecterror({"this is not an Octave identifier": 1})
testobjecterror({1.22: 1})

# These should fail: No object conversion defined.
testobjecterror(None)
testobjecterror((1, ))
testobjecterror(())

result, = pytave.feval(1, "eval", "[1, 1, 1]")
if result.shape != (1,3):
	print "FAIL: expected length-3 vector"

result, = pytave.feval(1, "eval", "[1; 2; 3]");
if result.shape != (3, 1):
	print "FAIL: expected 3x1 matrix"

testparseerror(1, "endfunction")
testevalexpect(1, "2 + 2", (4,))
testevalexpect(1, "{2}", (objarray([[2]]),))
testevalexpect(1, "struct('foo', 2)", ({'foo': objarray([[2]])},))

testsetget(pytave.locals, "xxx", arr1f)
testsetget(pytave.globals, "xxx", arr2o)

def func():
	pytave.locals["this is not a valid Octave identifier"] = 1
testexception(pytave.VarNameError, func)

def func():
	pytave.locals["nonexistentvariable"]
testexception(KeyError, func)

def func(key):
	pytave.locals[key] = 1
testexception(TypeError, lambda: func(0.1))
testexception(TypeError, lambda: func(1))
testexception(TypeError, lambda: func([]))

def func(key):
	pytave.locals[key]
testexception(TypeError, lambda: func(0.1))
testexception(TypeError, lambda: func(1))
testexception(TypeError, lambda: func([]))

testlocalscope(5)

testexception(KeyError, lambda: pytave.locals["localvariable"])
pytave.locals["localvariable"] = 1
if "localvariable" in pytave.globals:
	fail("Local variable in globals")
del pytave.locals["localvariable"]
if "localvariable" in pytave.locals:
	fail("Could not clear local variable")
testexception(KeyError, lambda: pytave.locals["localvariable"])
def func():
	del pytave.locals["localvariable"]
testexception(KeyError, lambda: func())

testexception(KeyError, lambda: pytave.globals["globalvariable"])
pytave.globals["globalvariable"] = 1
if "globalvariable" in pytave.locals:
	fail("Global variable in locals")
del pytave.globals["globalvariable"]
if "globalvariable" in pytave.globals:
	fail("Could not clear global variable")
testexception(KeyError, lambda: pytave.globals["globalvariable"])
def func():
	del pytave.globals["globalvariable"]
testexception(KeyError, lambda: func())

# Emacs
#	Local Variables:
#	fill-column:70
#	coding:utf-8
#	indent-tabs-mode:t
#	tab-width:8
#	python-indent:8
#	End:
# vim: set textwidth=70 noexpandtab tabstop=8 :
