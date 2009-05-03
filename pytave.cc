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
#include <octave/oct-map.h>
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
      // Also print Octave startup message.
      const char* argv[] = {"octave",
                            "--no-line-editing",
                            "--no-history",
                            NULL};
      octave_main(3, const_cast<char**>(argv), 1);

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
         octave_value_list lasterror = eval_string("lasterror",
                                                   true, parse_status, 1);
         if (!lasterror.empty() && lasterror(0).is_map()) {
            ostringstream exceptionmsg;
            Octave_map map = lasterror(0).map_value();
            string message = map.stringfield("message", "");
            string identifier = map.stringfield("identifier", "");
            Cell stackCell = map.contents("stack");

            // Trim trailing new lines
            message = message.substr(0, message.find_last_not_of("\r\n") + 1);

            if (!stackCell.is_empty() && stackCell(0).is_map()) {
               // The struct element is called "stack" but only contain
               // info about the top frame.
               Octave_map stack = stackCell(0).map_value();
               string file = stack.stringfield("file", "");
               string name = stack.stringfield("name", "");
               int line = stack.intfield("line", 1);
               int column = stack.intfield("column", 2);

               exceptionmsg << file << ":" << line << ":" << column << ": ";
               if (!name.empty())
                  exceptionmsg << "in '" << name << "': ";
            }

            if (!identifier.empty()) {
               exceptionmsg << "(identifier: " << identifier << ") ";
            }
            exceptionmsg << message;

            throw octave_error_exception(exceptionmsg.str());
         } else
            throw octave_error_exception("No Octave error available");
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
