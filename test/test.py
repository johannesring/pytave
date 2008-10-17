#!/usr/bin/python

import pytave
import Numeric

pytave.feval(0, "addpath", ".",);

arr0_0 = Numeric.zeros((0,0));
arr0_1 = Numeric.zeros((0,1));
arr1_0 = Numeric.zeros((1,0));
arr1f = Numeric.array([1, 2, 3, 4], Numeric.Float32)
arr1b = Numeric.array([1, 2, 3, 256], Numeric.Int8)
arr1i = Numeric.array([1, 2, 3, 4], Numeric.Int)
arr1i32 = Numeric.array([1, 2, 3, 4], Numeric.Int32)
arr1a = Numeric.array([1, 2, 3, 4])
arr2f = Numeric.array([[1, 2, 3, 4],[5,6,7,8]], Numeric.Float32)
arr2d = Numeric.array([[1, 2, 3, 4],[5,6,7,8]], Numeric.Float)
arr3f = Numeric.array([[[1, 2, 3, 4],[5,6,7,8]],[[9, 10, 11, 12],[13,14,15,16]]], Numeric.Float32)

alimit_int32 = Numeric.array([-2147483648, 2147483647], Numeric.Int32);
alimit_int16 = Numeric.array([-32768, 32767, -32769, 32768], Numeric.Int16);
alimit_int8 = Numeric.array([-128, 127, -129, 128], Numeric.Int8);
alimit_uint8 = Numeric.array([0, 255, -1, 256], Numeric.UnsignedInt8);


b = pytave.feval(1, "testfile", 1)
print "first exec ok:", b

def testequal(value):
	try:
		print "------------ test ", value
		nvalue = pytave.feval(1, "testfile", *value)
		if nvalue != value:
			print "Equal ", value, " == ", nvalue, ", ", (nvalue == value), " Equal"
	except TypeError, e:
		print "Execute failed: ", value,":", e

def testint(value):
	try:
		print "------------ int test ", value
		nvalue = pytave.feval(1, "testfile", *value)
		class1 = pytave.feval(1, "class", *value)
		class2 = pytave.feval(1, "class", *nvalue)
		if class1 != class2:
			print "Integer check failed, got ",class1, "and later", class2
	except TypeError, e:
		print "Execute failed: ", value,":", e
	
print "------------"

testint((alimit_int32, ))
testint((alimit_int16, ))
testint((alimit_int8, ))

testequal((alimit_int32, ))
testequal((alimit_int16, ))
testequal((alimit_int8, ))

testequal(("mystring", ))

testequal((1, ))
testequal((1L, ))
testequal((1.2, ))
testequal((1.2, ))
testequal((arr1a, ))

testequal((arr1f, ))
testequal((arr1i, ))
testequal((arr1b, ))
testequal((arr1i32, ))
testequal((arr2f, ))
testequal((arr2d, ))

testequal((arr3f, ))

print("Equality for these three tests fails")
testequal((arr0_0, ))
testequal((arr1_0, ))
testequal((arr0_1, ))

# these should fail.
print("These tests should fail.")
testequal(([1, 2],))
testequal((None,))
testequal(((1,),))
testequal(([],))
testequal(((),))
testequal((1, 8.9, 3, 4, "testfile", [], arr1f))

