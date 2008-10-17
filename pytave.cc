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

#define PYTAVE_DO_DECLARE_SYMBOL
#include "arrayobjectdefs.h"
#include <boost/python.hpp>
#include <boost/python/numeric.hpp>

#undef HAVE_STAT /* Both boost.python and octave define HAVE_STAT... */
#include <octave/oct.h>
#include <octave/parse.h>
#include <octave/load-path.h>
#include <octave/file-io.h>
#include <octave/ops.h>

#include <iostream>
#include <sstream>

#include <sys/types.h>

#include <octave/Matrix.h>
#include <octave/ov.h>
#include <octave/builtins.h>
#include <octave/defaults.h>

#include "pytavedefs.h"

#include "exceptions.h"
#include "python_to_octave.h"
#include "octave_to_python.h"

using namespace boost::python;
using namespace std;

namespace pytave { /* {{{ */ 

   void init() {

      if (!octave_error_exception::init()
          || !value_convert_exception::init()
          || !object_convert_exception::init()) {
         PyErr_SetString(PyExc_ImportError, "_pytave: init failed");
         return;
      }

      set_liboctave_error_handler(error);
      set_liboctave_warning_handler(warning);
      set_liboctave_warning_with_id_handler(warning_with_id);

      // New in Octave 3
      initialize_default_warning_state();

      install_defaults();
      initialize_file_io();
      initialize_symbol_tables();
      install_types();

      install_ops();

      install_builtins();

      // true argument new in Octave 3
      load_path::initialize(true); // or use false to set empty path

      // initialize python numeric array
      import_array();
   }

   boost::python::tuple get_exceptions() {
      return make_tuple(object(handle<PyObject>(
                                  octave_error_exception::excclass)),
                        object(handle<PyObject>(
                                  value_convert_exception::excclass)),
                        object(handle<PyObject>(
                                  object_convert_exception::excclass)));
   }

   boost::python::tuple func_eval(const int nargout,
                                  const std::string &funcname,
                                  const boost::python::tuple &arguments) {

      octave_value_list octave_args, retval;

      pytuple_to_octlist(octave_args, arguments);

      reset_error_handler();
      buffer_error_messages++;
      
      Py_BEGIN_ALLOW_THREADS
      retval = feval(funcname, octave_args, nargout);
      Py_END_ALLOW_THREADS

      if (error_state != 0) {
// error_state values:
// -2 error without traceback
// -1 traceback
//  1 general error
         int parse_status = 0;
         reset_error_handler();
         octave_value_list tmp = eval_string("lasterror.message",
                                             true, parse_status, 1);
         if (!tmp.empty() && tmp(0).is_string())
            throw octave_error_exception(tmp(0).string_value());
         else
            throw octave_error_exception("");
      }

      if (nargout > 0) {
         boost::python::tuple pytuple;
         octlist_to_pytuple(pytuple, retval);
         return pytuple;
      } else {
         // Return () if nargout <= 0.
         return make_tuple();
      }
   }
} /* namespace pytave }}} */

BOOST_PYTHON_MODULE(_pytave) { /* {{{ */
   using namespace boost::python;

   def("init", pytave::init);
   def("feval", pytave::func_eval);
   def("get_exceptions", pytave::get_exceptions);

   register_exception_translator<pytave::pytave_exception>(
      pytave::pytave_exception::translate_exception);

   register_exception_translator<pytave::octave_error_exception>(
      pytave::octave_error_exception::translate_exception);

   register_exception_translator<pytave::object_convert_exception>(
      pytave::object_convert_exception::translate_exception);

   register_exception_translator<pytave::value_convert_exception>(
      pytave::value_convert_exception::translate_exception);

} /* }}} */

/* Emacs
 * Local Variables:
 * fill-column:79
 * coding:utf-8
 * indent-tabs-mode:nil
 * c-basic-offset:3
 * End:
 * vim: set textwidth=79 expandtab shiftwidth=3 :
 */
