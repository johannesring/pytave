/*
 *  Copyright 2008 David Grundberg, Håkan Fors Nilsson
 *  Copyright 2009 VZLU Prague
 *
 *  This file is part of Pytave.
 *
 *  Pytave is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
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
#include <boost/type_traits/integral_constant.hpp>
#undef HAVE_STAT /* both boost.python and octave defines HAVE_STAT... */
#include <octave/oct.h>
#include <octave/oct-map.h>
#include <octave/Cell.h>
#include <octave/Matrix.h>
#include <octave/ov.h>

#include "pytavedefs.h"
#include "exceptions.h"

using namespace std;
using namespace boost::python;

namespace pytave {

   void pyobj_to_octvalue(octave_value &oct_value,
                          const boost::python::object &py_object);

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

   template <class PythonPrimitive, class OctaveBase>
   static void copy_pyarrobj_to_octarray_dispatch(OctaveBase &matrix,
                                       const PyArrayObject* const pyarr,
                                       const boost::true_type&) {
      copy_pyarrobj_to_octarray<PythonPrimitive, OctaveBase>
         (matrix, pyarr, 0, 1, 0, 0);
   }

   template <class PythonPrimitive, class OctaveBase>
   static void copy_pyarrobj_to_octarray_dispatch(OctaveBase &matrix,
                                       const PyArrayObject* const pyarr,
                                       const boost::false_type&) {
      assert(0);
   }

   template <class X, class Y> class matching_type : public boost::false_type { };
   template <class X> class matching_type<X, X> : public boost::true_type { };
   template <class X> class matching_type<X, octave_int<X> > : public boost::true_type { };
   template <> class matching_type<float, double> : public boost::true_type { };
   template <> class matching_type<FloatComplex, Complex> : public boost::true_type { };

   template <class PythonPrimitive, class OctaveBase>
   static void copy_pyarrobj_to_octarray_dispatch(OctaveBase &matrix,
                                       const PyArrayObject* const pyarr) {
      matching_type<PythonPrimitive, typename OctaveBase::element_type> inst;
      copy_pyarrobj_to_octarray_dispatch<PythonPrimitive, OctaveBase> (matrix, pyarr, inst);
   }

   template <class OctaveBase>
   static void copy_pyarrobj_to_octarray_boot(OctaveBase &matrix,
                                       const PyArrayObject* const pyarr) {

#define ARRAYCASE(AC_pyarrtype, AC_primitive) case AC_pyarrtype: \
         copy_pyarrobj_to_octarray_dispatch<AC_primitive, OctaveBase>\
         (matrix, pyarr); \
         break; \

      // Prefer int to other types of the same size.
      // E.g. on 32-bit x86 architectures: sizeof(long) == sizeof(int).
      int type_num = pyarr->descr->type_num;
      switch (type_num) {
         case PyArray_LONG:
            if (sizeof(long) == sizeof(int)) {
               type_num = PyArray_INT;
            }
            break;
         case PyArray_SHORT:
            if (sizeof(short) == sizeof(int)) {
               type_num = PyArray_INT;
            }
            break;
         case PyArray_USHORT:
            if (sizeof(unsigned short) == sizeof(unsigned int)) {
               type_num = PyArray_UINT;
            }
            break;
      }

      switch (type_num) {
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

         /* Commonly Numeric.array(..., Numeric.Complex32) */
         ARRAYCASE(PyArray_CFLOAT, FloatComplex)

         /* Commonly Numeric.array(..., Numeric.Complex) */
         ARRAYCASE(PyArray_CDOUBLE, Complex)
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
            pyarrobj_to_octvalueNd<FloatNDArray>(octvalue, pyarr, dims);
            break;
         case PyArray_DOUBLE:
            pyarrobj_to_octvalueNd<NDArray>(octvalue, pyarr, dims);
            break;
         case PyArray_CFLOAT:
            pyarrobj_to_octvalueNd<FloatComplexNDArray>(octvalue, pyarr, dims);
            break;
         case PyArray_CDOUBLE:
            pyarrobj_to_octvalueNd<ComplexNDArray>(octvalue, pyarr, dims);
            break;
         default:
            throw object_convert_exception(
               PyEval_GetFuncDesc((PyObject*)(pyarr)) + string(" ")
               + PyEval_GetFuncName((PyObject*)(pyarr))
               + ": Encountered unsupported Python array");
            break;
      }
   }

   static void pylist_to_cellarray(octave_value &oct_value,
                                   const boost::python::list &list) {

      octave_idx_type length = boost::python::extract<octave_idx_type>(
         list.attr("__len__")());
      octave_value_list values;

      for(octave_idx_type i = 0; i < length; i++) {
         octave_value val;

         pyobj_to_octvalue(val, list[i]);
         values.append(val);

      }

      oct_value = Cell(values);
   }

   static void pydict_to_octmap(octave_value &oct_value,
                                const boost::python::dict &dict) {

      boost::python::list list = dict.items();
      octave_idx_type length = boost::python::extract<octave_idx_type>(
         list.attr("__len__")());

      dim_vector dims = dim_vector(1, 1);

      bool has_dimensions = false;

      Array<octave_value> vals (length);
      Array<std::string> keys (length);

      for(octave_idx_type i = 0; i < length; i++) {

         std::string& key = keys(i);

         boost::python::tuple tuple =
            boost::python::extract<boost::python::tuple>(list[i])();

         boost::python::extract<std::string> str(tuple[0]);
         if(!str.check()) {
            throw object_convert_exception(
               string("Can not convert key of type ")
               + PyEval_GetFuncName(boost::python::object(tuple[0]).ptr())
               + PyEval_GetFuncDesc(boost::python::object(tuple[0]).ptr())
               + " to a structure field name. Field names must be strings.");
         }

         key = str();

         if (!valid_identifier(key)) {
            throw object_convert_exception(
               string("Can not convert key `") + key + "' to a structure "
               "field name. Field names must be valid Octave identifiers.");
         }

         octave_value& val = vals(i);

         pyobj_to_octvalue(val, tuple[1]);

         if(val.is_cell()) {
            const Cell c(val.cell_value());
            if (error_state)
               throw object_convert_exception("Octave error");

            // Some things are assumed since we have converted a Python list to
            // a cell.
            assert(c.dims().length() == 2);
            assert(c.dim1() == 1);

            // We do not bother measuring 1x1 values, since they are replicated
            // to fill up the necessary dimensions.
            if(!(c.dims().length() == 2 && c.dims()(0) == 1 && c.dims()(1) == 1)) {

               if(!has_dimensions) {
                  dims = c.dims();
                  has_dimensions = true;
               } else if(c.dims() != dims) {
                  throw object_convert_exception(
                     "Dimensions of the struct fields do not match");
               }
            }
         }
      }

      Octave_map map = Octave_map(dims);

      for(octave_idx_type i = 0; i < length; i++) {

         std::string& key = keys(i);
         octave_value val = vals(i);

         if(!val.is_cell()) {
            map.assign(key, Cell(dims, val));
         } else {
            const Cell c(val.cell_value());

            if (error_state)
               throw object_convert_exception("Octave error");

            if(c.dims().length() == 2 && c.dims()(0) == 1 && c.dims()(1) == 1) {
               map.assign(key, Cell(dims, c(0)));
            }
            else {
               map.assign(key, c);
            }
         }
         if (error_state) {
            throw object_convert_exception("Octave error");
         }
      }
      oct_value = map;
    }

   void pyobj_to_octvalue(octave_value &oct_value,
                          const boost::python::object &py_object) {
      extract<int> intx(py_object);
      extract<double> doublex(py_object);
      extract<Complex> complexx(py_object);
      extract<string> stringx(py_object);
      extract<numeric::array> arrayx(py_object);
      extract<boost::python::list> listx(py_object);
      extract<boost::python::dict> dictx(py_object);
      if (intx.check()) {
         oct_value = intx();
      } else if (doublex.check()) {
         oct_value = doublex();
      } else if (complexx.check()) {
         oct_value = complexx();
      } else if (arrayx.check()) {
         pyarr_to_octvalue(oct_value, (PyArrayObject*)py_object.ptr());
      } else if (stringx.check()) {
         oct_value = stringx();
      } else if (listx.check()) {
         pylist_to_cellarray(oct_value, (boost::python::list&)py_object);
      } else if (dictx.check()) {
         pydict_to_octmap(oct_value, (boost::python::dict&)py_object);
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
