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

#include "arrayobjectdefs.h"
#include <boost/python.hpp>
#include <boost/python/numeric.hpp>
#include <boost/type_traits/integral_constant.hpp>
#undef HAVE_STAT /* both boost::python and octave define HAVE_STAT... */
#include <octave/oct.h>
#include <octave/Matrix.h>
#include <octave/ov.h>
#include <octave/oct-map.h>

#include <iostream>
#include "pytavedefs.h"
#include "exceptions.h"

/* From docs:
 *  Note that the names of the element type constants refer to the C data
 *  types, not the Python data types. A Python int is equivalent to a C long,
 *  and a Python float corresponds to a C double . Many of the element types
 *  listed above do not have corresponding Python scalar types
 *  (e.g. PyArray_INT ).
 */

using namespace std;
using namespace boost::python;

namespace pytave {

   void octvalue_to_pyobj(boost::python::object &py_object,
                          const octave_value& octvalue);

   template <class PythonPrimitive, class OctaveBase>
   static void copy_octarray_to_pyarrobj(
                                  PyArrayObject *pyarr,
                                  const OctaveBase &matrix,
                                  const unsigned int matindex,
                                  const unsigned int matstride,
                                  const int dimension,
                                  const unsigned int offset) {
      unsigned char *ptr = (unsigned char*) pyarr->data;
      if (dimension == pyarr->nd - 1) {
         // Last dimension, base case
         for (int i = 0; i < pyarr->dimensions[dimension]; i++) {
            *(PythonPrimitive *)&ptr[offset + i*pyarr->strides[dimension]]
               = matrix.elem(matindex + i*matstride);
         }
      } else {
         for (int i = 0; i < pyarr->dimensions[dimension]; i++) {
            copy_octarray_to_pyarrobj<PythonPrimitive, OctaveBase>(
               pyarr,
               matrix,
               matindex + i*matstride,
               matstride * pyarr->dimensions[dimension],
               dimension + 1,
               offset + i*pyarr->strides[dimension]);
         }
      }
   }

   static PyArrayObject *createPyArr(const dim_vector &dims,
                                     int pyarrtype) {
      int dimensions[dims.length()];
      for (int i = 0; i < dims.length(); i++) {
         dimensions[i] = dims(i);
      }

      return (PyArrayObject *)PyArray_FromDims(
         dims.length(), dimensions, pyarrtype);
   }

   template <class PythonPrimitive, class OctaveBase>
   static PyArrayObject *create_array(const OctaveBase &octarr,
                                      int pyarraytype) {
      PyArrayObject *pyarr = createPyArr(octarr.dims(), pyarraytype);
      try {
         copy_octarray_to_pyarrobj
            <PythonPrimitive, OctaveBase>(pyarr, octarr, 0, 1, 0, 0);
      } catch (value_convert_exception &pe) {
         Py_DECREF(pyarr);
         throw;
      }
      return pyarr;
   }

   template <class PythonPrimitive, class OctaveBase>
   static PyArrayObject *create_array(const OctaveBase &octarr,
                                      int pyarraytype, boost::true_type) {
      return create_array<PythonPrimitive, OctaveBase> (octarr, pyarraytype);
   }

   template <class PythonPrimitive, class OctaveBase>
   static PyArrayObject *create_array(const OctaveBase &octarr,
                                      int pyarraytype, boost::false_type) {
      assert(0);
      return 0;
   }

   template <class CLASS, size_t bytes>
   inline static PyArrayObject *create_uint_array(CLASS value) {
      if (bytes == sizeof(int)) {
         boost::integral_constant<bool, bytes==sizeof(int)> inst;
         return create_array<unsigned int, CLASS>(value, PyArray_UINT, inst);
      } else if (bytes == sizeof(char)) {
         boost::integral_constant<bool, bytes==sizeof(char)> inst;
         return create_array<unsigned char, CLASS>(value, PyArray_UBYTE, inst);
      } else if (bytes == sizeof(short)) {
         boost::integral_constant<bool, bytes==sizeof(short)> inst;
         return create_array<unsigned short, CLASS>(value, PyArray_USHORT, inst);
      } else {
         ostringstream os;
         os << "Numeric arrays doesn't support unsigned " << (bytes*8)
            << "-bit values on this architecture.";
         throw value_convert_exception(os.str());
      }
   }

   template <class CLASS, size_t bytes>
   inline static PyArrayObject *create_sint_array(CLASS value) {
      if (bytes == sizeof(long)) {
         boost::integral_constant<bool, bytes==sizeof(long)> inst;
         return create_array<long, CLASS>(value, PyArray_LONG, inst);
      } else if (bytes == sizeof(int)) {
         boost::integral_constant<bool, bytes==sizeof(int)> inst;
         return create_array<signed int, CLASS>(value, PyArray_INT, inst);
      } else if (bytes == sizeof(char)) {
         boost::integral_constant<bool, bytes==sizeof(char)> inst;
         return create_array<signed char, CLASS>(value, PyArray_SBYTE, inst);
      } else if (bytes == sizeof(short)) {
         boost::integral_constant<bool, bytes==sizeof(short)> inst;
         return create_array<signed short, CLASS>(value, PyArray_SHORT, inst);
      } else {
         ostringstream os;
         os << "Numeric arrays doesn't support signed " << (bytes*8)
            << "-bit values on this architecture.";
         throw value_convert_exception(os.str());
      }
   }

   static PyArrayObject *octvalue_to_pyarrobj(const octave_value &matrix) {
      if (matrix.is_double_type ()) {
         if (matrix.is_complex_type ()) {
            return create_array<Complex, ComplexNDArray>
               (matrix.complex_array_value(), PyArray_CDOUBLE);
         } else if (matrix.is_real_type()) {
            return create_array<double, NDArray>(matrix.array_value(),
                                                 PyArray_DOUBLE);
         } else
            throw value_convert_exception("Unknown double matrix type");
      }

#ifdef PYTAVE_USE_OCTAVE_FLOATS
      if (matrix.is_single_type ()) {
         if (matrix.is_complex_type ()) {
            return create_array<FloatComplex, FloatComplexNDArray>
               (matrix.float_complex_array_value(), PyArray_CFLOAT);
         } else if (matrix.is_real_type()) {
            return create_array<float, FloatNDArray>(
               matrix.float_array_value(), PyArray_FLOAT);
         } else
            throw value_convert_exception("Unknown float matrix type");
      }
#endif

      if (matrix.is_int64_type()) {
         return create_sint_array<int64NDArray, sizeof(int64_t)>(
            matrix.int64_array_value());
      }
      if (matrix.is_uint32_type()) {
         return create_uint_array<uint32NDArray, sizeof(uint32_t)>(
            matrix.uint32_array_value());
      }
      if (matrix.is_int32_type()) {
         return create_sint_array<int32NDArray, sizeof(int32_t)>(
            matrix.int32_array_value());
      }
      if (matrix.is_uint16_type()) {
         return create_uint_array<uint16NDArray, sizeof(uint16_t)>(
            matrix.uint16_array_value());
      }
      if (matrix.is_int16_type()) {
         return create_sint_array<int16NDArray, sizeof(int16_t)>(
            matrix.int16_array_value());
      }
      if (matrix.is_uint8_type()) {
         return create_uint_array<uint8NDArray, sizeof(uint8_t)>(
            matrix.uint8_array_value());
      }
      if (matrix.is_int8_type()) {
         return create_sint_array<int8NDArray, sizeof(int8_t)>(
            matrix.int8_array_value());
      }

      throw value_convert_exception("Octave matrix type not known, "
                                    "conversion not implemented");
   }

   static void octvalue_to_pyarr(boost::python::object &py_object,
                          const octave_value& octvalue) {
      PyArrayObject *pyarr = octvalue_to_pyarrobj(octvalue);
      py_object = object(handle<PyObject>((PyObject *)pyarr));
   }

   static inline bool is_1xn_or_0x0(const dim_vector& dv) {
         return (dv.length() == 2 && (dv(0) == 1 || (dv(0) == 0 && dv(1) == 0)));
   }

   static void octcell_to_pyobject(boost::python::object &py_object,
                                   const Cell& cell) {
      py_object = boost::python::list();

      if(!is_1xn_or_0x0(cell.dims())) {
         throw value_convert_exception(
            "Only one-dimensional (row mayor) cell arrays can be converted.");
      }

      for(octave_idx_type i = 0 ; i < cell.length(); i++) {
         boost::python::object py_val;

         octvalue_to_pyobj(py_val, cell.elem(i));

         ((boost::python::list&)py_object).insert(i, py_val);
      }
   }

   static void octmap_to_pyobject(boost::python::object &py_object,
                                  const Octave_map& map) {
      py_object = boost::python::dict();
      string_vector keys = map.keys();

      for(octave_idx_type i = 0 ; i < keys.length(); i++) {
         boost::python::object py_val;

         octvalue_to_pyobj(py_val, map.contents(keys[i]));

         py_object[keys[i]] = py_val;
      }
   }

   void octvalue_to_pyobj(boost::python::object &py_object,
                          const octave_value& octvalue) {
      if (octvalue.is_undefined())
         throw value_convert_exception(
            "Octave value `undefined'. Can not convert to a Python object");
      else if (octvalue.is_scalar_type()) {
         if (octvalue.is_bool_type())
            py_object = object(octvalue.bool_value());
         else if (octvalue.is_real_scalar())
            py_object = object(octvalue.double_value());
         else if (octvalue.is_complex_scalar())
            py_object = object(octvalue.complex_value());
         else if (octvalue.is_integer_type())
            py_object = object(octvalue.int_value());
         else
            throw value_convert_exception(
               "Conversion for this scalar not implemented");
      } else if (octvalue.is_string()) {
         if (! is_1xn_or_0x0 (octvalue.dims ()))
            throw value_convert_exception(
               "Multi-row character matrices can not be converted.");
         py_object = str(octvalue.string_value());
      } else if (octvalue.is_matrix_type()) {
         octvalue_to_pyarr(py_object, octvalue);
      } else if (octvalue.is_map()) {
         octmap_to_pyobject(py_object, octvalue.map_value());
      } else if (octvalue.is_cell()) {
         octcell_to_pyobject(py_object, octvalue.cell_value()); 
      } else
         throw value_convert_exception(
            "Conversion from Octave value not implemented");
   }

   void octlist_to_pytuple(boost::python::tuple &python_tuple,
                           const octave_value_list &octave_list) {
      boost::python::list seq;
      int length = octave_list.length();
      for (int i = 0; i < length; i++) {
         boost::python::object py_object;
         octvalue_to_pyobj(py_object, octave_list(i));
         seq.append(py_object);
      }
      python_tuple = tuple(seq);
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
