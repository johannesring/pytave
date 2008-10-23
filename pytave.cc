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

#define PYTAVE_DO_DECLARE_SYMBOL
#include "arrayobjectdefs.h"
#include <boost/python.hpp>
#include <boost/python/numeric.hpp>

#undef HAVE_STAT /* Both boost.python and octave define HAVE_STAT... */
#include <octave/oct.h>
#include <octave/octave.h>
#include <octave/ov.h>
#include <octave/parse.h>

#include <iostream>
#include <sstream>
#include <sys/types.h>

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

      // Initialize Octave.
      if (Py_FdIsInteractive(stdin, NULL)) {
         // Print Octave interactive message.
         char* argv[] = {"octave", "--no-line-editing", "--no-history", NULL};
         octave_main(3, argv, 1);
      } else {
         // One may use -q to surpress the startup greeting.
         char* argv[] = {"octave", "-q", "--no-line-editing",
                         "--no-history", NULL};
         octave_main(4, argv, 1);
      }

      // Initialize Python Numeric Array

      // This is actually a macro that becomes a block expression. If an error
      // occurs, e.g. Numeric Array not installed, an exception is set.
      import_array()
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
