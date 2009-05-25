/*
 *  Copyright 2008 David Grundberg, Håkan Fors Nilsson
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

#ifndef PYTAVE_EXCEPTIONS_H
#define PYTAVE_EXCEPTIONS_H

namespace pytave {

   class pytave_exception {
      public:
         static void translate_exception(pytave_exception const &py_ex) {
            PyErr_SetString(PyExc_Exception, py_ex.error.c_str());
         }

         pytave_exception(std::string err) { error = err; };

      private:
         std::string error;

   };

   class octave_error_exception {
      public:
         static bool init() {
            excclass = PyErr_NewException(
               const_cast<char*>("pytave.OctaveError"),
               PyExc_RuntimeError, NULL);
            return excclass != NULL;
         };
         static void translate_exception(octave_error_exception const &py_ex) {
            PyErr_SetString(excclass, py_ex.error.c_str());
         }
         static PyObject *excclass;

         octave_error_exception(std::string err) { error = err; };

      private:
         std::string error;

   };

   class octave_parse_exception {
      public:
         static bool init() {
            excclass = PyErr_NewException(
               const_cast<char*>("pytave.ParseError"),
               PyExc_RuntimeError, NULL);
            return excclass != NULL;
         };
         static void translate_exception(octave_parse_exception const &py_ex) {
            PyErr_SetString(excclass, py_ex.error.c_str());
         }
         static PyObject *excclass;

         octave_parse_exception(std::string err) { error = err; };

      private:
         std::string error;
   };

   class value_convert_exception {
      public:
         static bool init() {
            excclass = PyErr_NewException(
               const_cast<char*>("pytave.ValueConvertError"),
               PyExc_TypeError, NULL);
            return excclass != NULL;
         };
         static void translate_exception(value_convert_exception const &py_ex) {
            PyErr_SetString(excclass, py_ex.error.c_str());
         }
         static PyObject *excclass;

         value_convert_exception(std::string err) { error = err; };

      private:
         std::string error;
   };

   class object_convert_exception {
      public:
         static bool init() {
            excclass = PyErr_NewException(
               const_cast<char*>("pytave.ObjectConvertError"),
               PyExc_TypeError, NULL);
            return excclass != NULL;
         };
         static void translate_exception(
               object_convert_exception const &py_ex) {
            PyErr_SetString(excclass, py_ex.error.c_str());
         }
         static PyObject *excclass;

         object_convert_exception(std::string err) { error = err; };

      private:
         std::string error;
   };

   class variable_name_exception {
      public:
         static bool init() {
            excclass = PyErr_NewException(
               const_cast<char*>("pytave.VarNameError"),
               PyExc_RuntimeError, NULL);
            return excclass != NULL;
         };
         static void translate_exception(variable_name_exception const &py_ex) {
            PyErr_SetString(excclass, py_ex.error.c_str());
         }
         static PyObject *excclass;

         variable_name_exception(std::string err) { error = err; };

      private:
         std::string error;

   };

}

#endif /* PYTAVE_EXCEPTIONS_H */

/* Emacs
 * Local Variables:
 * fill-column:79
 * coding:utf-8
 * indent-tabs-mode:nil
 * c-basic-offset:3
 * End:
 * vim: set textwidth=79 expandtab shiftwidth=3 :
 */
