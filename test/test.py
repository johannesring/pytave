#!/usr/bin/python
# -*- coding:utf-8 -*-

import pytave
import Numeric
import traceback

print "No messages indicates test pass."

arr0_0 = Numeric.zeros((0,0));
arr0_1 = Numeric.zeros((0,1));
arr1_0 = Numeric.zeros((1,0));
number = Numeric.array([1.32], Numeric.Float32)
arr1fT = Numeric.array([[1.32], [2], [3], [4]], Numeric.Float32)
arr1fT2 = Numeric.array([1.32, 2, 3, 4], Numeric.Float32)
arr1f = Numeric.array([1.32, 2, 3, 4], Numeric.Float32)
arr1b = Numeric.array([8, 2, 3, 256], Numeric.Int8)
arr1i = Numeric.array([17, 2, 3, 4], Numeric.Int)
arr1i32 = Numeric.array([32, 2, 3, 4], Numeric.Int32)
arr1a = Numeric.array([1, 2, 3, 4])
arr2f = Numeric.array([[1.32, 2, 3, 4],[5,6,7,8]], Numeric.Float32)
arr2d = Numeric.array([[1.17, 2, 3, 4],[5,6,7,8]], Numeric.Float)
arr3f = Numeric.array([[[1.32, 2, 3, 4],[5,6,7,8]],[[9, 10, 11, 12],[13,14,15,16]]], Numeric.Float32)
arr1c = Numeric.array([1+2j, 3+4j, 5+6j, 7+0.5j], Numeric.Complex)
arr1fc = Numeric.array([1+2j, 3+4j, 5+6j, 7+0.5j], Numeric.Complex32)
arr1ch = Numeric.array("abc",Numeric.Character)
arr2ch = Numeric.array(["abc","def"],Numeric.Character)
arr1o = Numeric.array([1.0,"abc",2+3j],Numeric.PyObject)
arr2o = Numeric.array([[1.0,"abc",2+3j],[4.0,arr1i,"def"]],Numeric.PyObject)

alimit_int32 = Numeric.array([-2147483648, 2147483647], Numeric.Int32);
alimit_int16 = Numeric.array([-32768, 32767, -32769, 32768], Numeric.Int16);
alimit_int8 = Numeric.array([-128, 127, -129, 128], Numeric.Int8);
alimit_uint8 = Numeric.array([0, 255, -1, 256], Numeric.UnsignedInt8);


# This eval call is not to be seen as a encouragement to use Pytave
# like this. Create a separate .m-file with your complex Octave code.
pytave.eval(0, "function [result] = test_return(arg); result = arg; endfunction")

pytave.feval(1, "test_return", 1)

def fail(msg, exc=None):
	print "FAIL:", msg
	traceback.print_stack()
	if exc is not None:
		traceback.print_exc(exc)
	print ""

def testequal(value):
	try:
		nvalue, = pytave.feval(1, "test_return", value)
		if nvalue != value:
			fail("as %s != %s" % (value, nvalue))
	except TypeError, e:
		fail(value, e)

def testexpect(value, expected):
	try:
		nvalue, = pytave.feval(1, "test_return", value)
		if nvalue != expected:
			fail("sent in %s, expecting %s, got %s", (value, expected, nvalue))
	except TypeError, e:
		fail(value, e)

def testmatrix(value):
	try:
		nvalue, = pytave.feval(1, "test_return", value)
		class1 = pytave.feval(1, "class", value)
		class2 = pytave.feval(1, "class", nvalue)
		if nvalue != value:
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
		if results != expectations:
			fail("eval: %s : because %s != %s" % (code, results, expectations))
	except Exception, e:
		fail("eval: %s" % code, e)

def testsetget(name,value):
    try:
	pytave.setvar(name,value)
	result, = pytave.feval(1, "isequal", value, pytave.getvar(name))
	if not result:
	    print "FAIL: set/get: ", name," -> ",value," results diverged"
    except Exception, e:
	print "FAIL: set/get: ", name, ":"

def testvarnameerror(name):
    try:
	pytave.setvar(name)
	print "FAIL: ", name
    except pytave.VarNameError:
	pass
    except Exception, e:
	print "FAIL: ", name
			fail("eval: %s : because %s != %s" % (code, results, expectations))
	except Exception, e:
		fail("eval: %s" % code, e)

def testlocalscope(x):

    @pytave.local_scope
    def sloppy_factorial(x):
	pytave.setvar("x",x)
	xm1, = pytave.eval(1,"x-1")
	if xm1 > 0:
	    fxm1 = sloppy_factorial(xm1)
	else:
	    fxm1 = 1
	pytave.locals["fxm1"] = fxm1
	fx, = pytave.eval(1,"x * fxm1")
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


testmatrix(alimit_int32)
testmatrix(alimit_int16)
testmatrix(alimit_int8)

# Strings
# Multi-row character matrix cannot be returned

testequal("mystring")
testequal("mystringåäöÅÄÖ")

testequal(1)
testequal(1L)
testequal(1.2)
testequal(1.2)

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

# Lists
testequal([1, 2])
testequal([[1, 2], [3, 4]])
testequal([[[1, 2], [3, 4]], [[5, 6], [7, 8]]])
testequal([])

# Return cells with OK dimensions
testvalueok("cell", 1, 3);
testvalueok("cell", 1, 0)
testvalueok("cell", 0, 0)
testvalueok("cell", 3, 1)
testvalueok("cell", 0, 1)

# Dictionaries

# Simple dictionary tests
testequal({"foo": 1, "bar": 2})
testequal({"x": [1, 3], "y": [2, 4]})
testequal({"x": [1, "baz"], "y": [2, "foobar"]})
testequal({"x": [arr1f], "y": [arr1i]})
testequal({})
testequal({"foo": arr1f,    "bar": arr2f})
testequal({"foo": 1,        "bar": [2]})
testexpect({"foo": [[1,2]],        "bar": [[3,2]]},
	   {"foo": [1,2],        "bar": [3,2]})

# Try some odd dictionaries
# The implicit conversion makes Pytave return cell-wrapped results.

# Try some invalid keys
testobjecterror({"this is not an Octave identifier": 1})
testobjecterror({1.22: 1})

# These should fail: No object conversion defined.
testobjecterror(None)
testobjecterror((1, ))
testobjecterror(())

result, = pytave.feval(1, "eval", "[1, 1, 1]")
if result.shape != (3,):
	print "FAIL: expected length-3 vector"

result, = pytave.feval(1, "eval", "[1; 2; 3]");
if result.shape != (3, 1):
	print "FAIL: expected 3x1 matrix"

testparseerror(1, "endfunction")
testevalexpect(1, "2 + 2", (4,))
testevalexpect(1, "{2}", ([2],))
testevalexpect(2, "struct('foo', 2)", ({'foo': 2},))

testlocalscope(5)

# Emacs
#	Local Variables:
#	fill-column:70
#	coding:utf-8
#	indent-tabs-mode:t
#	tab-width:8
#	python-indent:8
#	End:
# vim: set textwidth=70 noexpandtab tabstop=8 :
