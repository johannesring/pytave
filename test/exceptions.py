#!/usr/bin/python
import pytave

try:
 pytave.feval(1,"",)
except pytave.OctaveError, e:
 print "test ok"
except:
 print "test fail"

try:
 pytave.feval(1,"cell",)
except pytave.ValueConvertError, e:
 print "test ok"
except:
 print "test fail"

try:
 pytave.feval(1,"sin",{"asdf":"asdf"})
except pytave.ObjectConvertError, e:
 print "test ok"
except:
 print "test fail"
