/*
 *  Copyright 2008 David Grundberg, Håkan Fors Nilsson
 *
 *  This file is part of Pytave.
 *
 *  Pytave is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Pytave is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Pytave.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <boost/python.hpp>
#include <boost/python/numeric.hpp>
#include "arrayobjectdefs.h"
#undef HAVE_STAT /* both boost.python and octave defines HAVE_STAT... */
#include <octave/oct.h>
#include <octave/Matrix.h>
#include <octave/ov.h>

#include "pytavedefs.h"
#include "exceptions.h"

using namespace std;
using namespace boost::python;

namespace pytave {

   template <class PythonPrimitive, class OctaveBase>
   static void copy_pyarrobj_to_octarray(OctaveBase &matrix,
                                  const PyArrayObject* const pyarr,
                                  const int unsigned matindex,
                                  const unsigned int matstride,
                                  const int dimension,
                                  const unsigned int offset) {
      unsigned char *ptr = (unsigned char*) pyarr->data;
      if (dimension == pyarr->nd - 1) {
         // Last dimension, base case
         for (int i = 0; i < pyarr->dimensions[dimension]; i++) {
            matrix.elem(matindex + i*matstride)
               = *(PythonPrimitive*)
               &ptr[offset + i*pyarr->strides[dimension]];
         }
      } else {
         for (int i = 0; i < pyarr->dimensions[dimension]; i++) {
            copy_pyarrobj_to_octarray<PythonPrimitive, OctaveBase>(
               matrix,
               pyarr,
               matindex + i*matstride,
               matstride * pyarr->dimensions[dimension],
               dimension + 1,
               offset + i*pyarr->strides[dimension]);
         }
      }
   }

   template <class OctaveBase>
   static void copy_pyarrobj_to_octarray_boot(OctaveBase &matrix,
                                       const PyArrayObject* const pyarr) {

#define ARRAYCASE(AC_pyarrtype, AC_primitive) case AC_pyarrtype: \
         copy_pyarrobj_to_octarray<AC_primitive, OctaveBase>\
         (matrix, pyarr, 0, 1, 0, 0); \
         break; \

      switch (pyarr->descr->type_num) {
//         ARRAYCASE(PyArray_CHAR, )
         ARRAYCASE(PyArray_UBYTE,  unsigned char)
         ARRAYCASE(PyArray_SBYTE,  signed   char)
         ARRAYCASE(PyArray_SHORT,  signed   short)
         ARRAYCASE(PyArray_USHORT, unsigned short)
         ARRAYCASE(PyArray_INT,    signed   int)
         ARRAYCASE(PyArray_UINT,   unsigned int)
         ARRAYCASE(PyArray_LONG,   signed   long)

         /* Commonly Numeric.array(..., Numeric.Float32) */
         ARRAYCASE(PyArray_FLOAT,  float)

         /* Commonly Numeric.array(..., Numeric.Float) */
         ARRAYCASE(PyArray_DOUBLE, double)
//         ARRAYCASE(PyArray_CFLOAT, )
//         ARRAYCASE(PyArray_CDOUBLE, )
//         ARRAYCASE(PyArray_OBJECT, )
         default:
            throw object_convert_exception(
               PyEval_GetFuncName((PyObject*)pyarr)
               + (PyEval_GetFuncDesc((PyObject*)pyarr)
               + string(": Unsupported Python array type")));
      }
   }

   template <class OctaveBase>
   static void pyarrobj_to_octvalueNd(octave_value &octvalue,
                               const PyArrayObject* const pyarr,
                               dim_vector dims) {
      OctaveBase array(dims);
      copy_pyarrobj_to_octarray_boot<OctaveBase>(array, pyarr);
      octvalue = array;
   }

   static void pyarr_to_octvalue(octave_value &octvalue,
                          const PyArrayObject *pyarr) {
      if (pyarr->nd < 1)
         throw object_convert_exception("Less than 1 dimensions not supported");

      dim_vector dims;
      switch (pyarr->nd) {
         case 1:
            // Always make PyArray vectors row vectors.
            dims = dim_vector(1, pyarr->dimensions[0]);
            break;
         default:
            dims.resize(pyarr->nd);
            for (int d = 0; d < pyarr->nd; d++) {
               dims(d) = pyarr->dimensions[d];
            }
            break;
      }

      switch (pyarr->descr->type_num) {
         case PyArray_UBYTE:
         case PyArray_USHORT:
         case PyArray_UINT:
            switch (pyarr->descr->elsize) {
               case 1:
                  pyarrobj_to_octvalueNd<uint8NDArray>(octvalue, pyarr, dims);
                  break;
               case 2:
                  pyarrobj_to_octvalueNd<uint16NDArray>(octvalue, pyarr, dims);
                  break;
               case 4:
                  pyarrobj_to_octvalueNd<uint32NDArray>(octvalue, pyarr, dims);
                  break;
               default:
                  throw object_convert_exception("Unknown unsigned integer.");
            }
         case PyArray_SBYTE:
         case PyArray_SHORT:
         case PyArray_INT:
         case PyArray_LONG:
            switch (pyarr->descr->elsize) {
               case 1:
                  pyarrobj_to_octvalueNd<int8NDArray>(octvalue, pyarr, dims);
                  break;
               case 2:
                  pyarrobj_to_octvalueNd<int16NDArray>(octvalue, pyarr, dims);
                  break;
               case 4:
                  pyarrobj_to_octvalueNd<int32NDArray>(octvalue, pyarr, dims);
                  break;
               case 8:
                  pyarrobj_to_octvalueNd<int64NDArray>(octvalue, pyarr, dims);
                  break;
               default:
                  throw object_convert_exception("Unknown integer.");
            }
            break;
         case PyArray_FLOAT:
#ifdef PYTAVE_USE_OCTAVE_FLOATS
            pyarrobj_to_octvalueNd<FloatNDArray>(octvalue, pyarr, dims);
            break;
#else
            /* fallthrough */
#endif
         case PyArray_DOUBLE:
            pyarrobj_to_octvalueNd<NDArray>(octvalue, pyarr, dims);
            break;
         default:
            throw object_convert_exception(
               PyEval_GetFuncDesc((PyObject*)(pyarr)) + string(" ")
               + PyEval_GetFuncName((PyObject*)(pyarr))
               + ": Encountered unsupported Python array");
            break;
      }
   }

   void pyobj_to_octvalue(octave_value &oct_value,
                          const boost::python::object &py_object) {
      extract<int> intx(py_object);
      extract<double> doublex(py_object);
      extract<string> stringx(py_object);
      extract<numeric::array> arrayx(py_object);
      if (intx.check()) {
         oct_value = intx();
      } else if (doublex.check()) {
         oct_value = doublex();
      } else if (arrayx.check()) {
         pyarr_to_octvalue(oct_value, (PyArrayObject*)py_object.ptr());
      } else if (stringx.check()) {
         oct_value = stringx();
      } else {
         throw object_convert_exception(
            PyEval_GetFuncName(py_object.ptr())
            + (PyEval_GetFuncDesc(py_object.ptr())
               + string(": Unsupported Python object type, "
                        "cannot convert to Octave value")));
      }
   }

   void pytuple_to_octlist(octave_value_list &octave_list,
                           const boost::python::tuple &python_tuple) {
      int length = extract<int>(python_tuple.attr("__len__")());

      for (int i = 0; i < length; i++) {
         pyobj_to_octvalue(octave_list(i), python_tuple[i]);
      }
   }


}

/* Emacs
 * Local Variables:
 * fill-column:79
 * coding:utf-8
 * indent-tabs-mode:nil
 * c-basic-offset:3
 * End:
 * vim: set textwidth=79 expandtab shiftwidth=3 :
 */
