/*
 *  Copyright 2008 David Grundberg, Håkan Fors Nilsson
 *  Copyright 2009 Jaroslav Hajek, VZLU Prague
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

#include <boost/python.hpp>
#include <boost/python/numeric.hpp>

#undef HAVE_STAT /* Both boost.python and octave define HAVE_STAT... */
#undef HAVE_FSTAT /* Both boost.python and octave define HAVE_FSTAT... */
#include <octave/oct.h>

#include <octave/oct-map.h>
#include <octave/octave.h>
#include <octave/ov.h>
#include <octave/parse.h>
#include <octave/symtab.h>
#include <octave/toplev.h>
#include <octave/utils.h>

#include <iostream>
#ifdef HAVE_USELOCALE
#include <locale.h>
#endif
#include <sstream>
#include <sys/types.h>

#include "pytavedefs.h"

#define PYTAVE_DO_DECLARE_SYMBOL
#include "arrayobjectdefs.h"
#include "exceptions.h"
#include "octave_to_python.h"
#include "python_to_octave.h"

using namespace boost::python;
using namespace std;

namespace pytave { /* {{{ */

#ifdef HAVE_USELOCALE
   locale_t c_locale;
#endif

   void init(bool silent = true) {
#ifdef HAVE_USELOCALE
      c_locale = newlocale(LC_ALL, "C", 0);
#endif

      if (!octave_error_exception::init()
          || !value_convert_exception::init()
          || !object_convert_exception::init()
          || !octave_parse_exception::init()
          || !variable_name_exception::init ()) {
         PyErr_SetString(PyExc_ImportError, "_pytave: init failed");
         return;
      }

      // Initialize Octave.
      // Also print Octave startup message.
      const char* argv[] = {"octave",
                            "--no-line-editing",
                            "--no-history",
                            "--no-init-file",
                            "--silent",
                            NULL};
      int argc = 5;

      if (silent) {
         argc--;
         argv[argc] = 0;
      }

#ifdef HAVE_USELOCALE
      // Set C locale
      locale_t old_locale = uselocale(c_locale);
#endif

      octave_main(argc, const_cast<char**>(argv), 1);

#ifdef HAVE_USELOCALE
      // Reset locale
      uselocale(old_locale);
#endif

      // Initialize Python Numeric Array

      // This is actually a macro that becomes a block expression. If an error
      // occurs, e.g. Numeric Array not installed, an exception is set.
      import_array()
#ifdef HAVE_NUMPY
      // Let boost use numpy
      numeric::array::set_module_and_type ("numpy", "ndarray");
#endif
   }

   string get_module_name () {
      return numeric::array::get_module_name ();
   }

   boost::python::tuple get_exceptions() {
      return make_tuple(object(handle<PyObject>(
                                  octave_error_exception::excclass)),
                        object(handle<PyObject>(
                                  value_convert_exception::excclass)),
                        object(handle<PyObject>(
                                  object_convert_exception::excclass)),
                        object(handle<PyObject>(
                                  octave_parse_exception::excclass)),
                        object(handle<PyObject>(
                                  variable_name_exception::excclass)));
   }

   string make_error_message (const Octave_map& map) {
      ostringstream exceptionmsg;
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

      return exceptionmsg.str ();
   }

   boost::python::tuple func_eval(const int nargout,
                                  const string &funcname,
                                  const boost::python::tuple &arguments) {

      octave_value_list octave_args, retval;

      pytuple_to_octlist(octave_args, arguments);

      reset_error_handler();
      buffer_error_messages++;

      // Updating the timestamp makes Octave reread changed files
      Vlast_prompt_time.stamp();

#ifdef HAVE_USELOCALE
      // Set C locale
      locale_t old_locale = uselocale(c_locale);
#endif

      bool bad_alloc_state = false;
      Py_BEGIN_ALLOW_THREADS
      try {
         retval = feval(funcname, octave_args, (nargout >= 0) ? nargout : 0);
      } catch (bad_alloc) {
         bad_alloc_state = true;
      }
      Py_END_ALLOW_THREADS

#ifdef HAVE_USELOCALE
      // Reset locale
      uselocale(old_locale);
#endif

      if (bad_alloc_state)
         throw bad_alloc (); // Translated to MemoryError by boost::python

      else if (error_state != 0) {
// error_state values:
// -2 error without traceback
// -1 traceback
//  1 general error
         int parse_status = 0;
         reset_error_handler();
         octave_value_list lasterror = eval_string("lasterror",
                                                   true, parse_status, 1);
         if (!lasterror.empty() && lasterror(0).is_map()) {
            string exceptionmsg = make_error_message(lasterror(0).map_value ());
            throw octave_error_exception(exceptionmsg);
         } else
            throw octave_error_exception("No Octave error available");
      }

      if (nargout >= 0) {
         boost::python::tuple pytuple;
         octlist_to_pytuple(pytuple, retval);
         return pytuple;
      } else {
         // Return () if nargout < 0.
         return make_tuple();
      }
   }

   boost::python::tuple str_eval(int nargout,
                                 const string &code,
                                 bool silent) {

      octave_value_list retval;
      int parse_status;

      reset_error_handler();
      buffer_error_messages++;

      // Updating the timestamp makes Octave reread changed files
      Vlast_prompt_time.stamp();

#ifdef HAVE_USELOCALE
      // Set C locale
      locale_t old_locale = uselocale(c_locale);
#endif

      bool bad_alloc_state = false;
      Py_BEGIN_ALLOW_THREADS
      try {
         retval = eval_string(code, silent, parse_status,
            (nargout >= 0) ? nargout : 0);
      } catch (bad_alloc) {
         bad_alloc_state = true;
      }
      Py_END_ALLOW_THREADS

#ifdef HAVE_USELOCALE
      // Reset locale
      uselocale(old_locale);
#endif

      if (bad_alloc_state)
         throw bad_alloc (); // Translated to MemoryError by boost::python

      if (parse_status != 0 || error_state != 0) {
// error_state values:
// -2 error without traceback
// -1 traceback
//  1 general error
         int parse_status1 = 0;
         reset_error_handler();
         octave_value_list lasterror = eval_string("lasterror",
                                                   true, parse_status1, 1);
         if (!lasterror.empty() && lasterror(0).is_map()) {
            string exceptionmsg = make_error_message (lasterror(0).map_value ());

            if (parse_status != 0)
               throw octave_parse_exception(exceptionmsg);
            else
               throw octave_error_exception(exceptionmsg);
         } else
            throw octave_error_exception("No Octave error available");
      }

      if (nargout >= 0) {
         boost::python::tuple pytuple;
         octlist_to_pytuple(pytuple, retval);
         return pytuple;
      } else {
         // Return () if nargout < 0.
         return make_tuple();
      }
   }

   boost::python::object getvar(const string& name,
                                bool global) {
      octave_value val;

      if (global)
         val = symbol_table::global_varval(name);
      else
         val = symbol_table::varval(name);

      if (val.is_undefined()) {
         throw variable_name_exception (name + " not defined in current scope");
      }

      boost::python::object pyobject;
      octvalue_to_pyobj(pyobject, val);

      return pyobject;
   }

   void setvar(const string& name, 
               const boost::python::object& pyobject,
               bool global) {
      octave_value val;

      if (!valid_identifier(name)) {
         throw variable_name_exception (name + " is not a valid identifier");
      }

      pyobj_to_octvalue(val, pyobject);

      if (global)
         symbol_table::global_varref(name) = val;
      else
         symbol_table::varref(name) = val;
   }

   bool isvar(const string& name, bool global) {
      bool retval;

      if (global)
         retval = symbol_table::global_varval (name).is_defined ();
      else
         retval = symbol_table::is_variable (name);

      return retval;
   }

   void delvar(const string& name, bool global) {

      if (global) {

         // FIXME: workaround a bug in Octave 3.2.0.
         if (! symbol_table::is_global (name))
            symbol_table::insert (name).mark_global ();

         symbol_table::clear_global (name);
      } else
         symbol_table::clear_variable (name);
   }

   int push_scope() {
      symbol_table::scope_id local_scope = symbol_table::alloc_scope();
      symbol_table::set_scope(local_scope);
      octave_call_stack::push(local_scope);
      return local_scope;
   }

   void pop_scope () {
      symbol_table::scope_id curr_scope = symbol_table::current_scope();
      if (curr_scope != symbol_table::top_scope())
         {
            symbol_table::erase_scope(curr_scope);
            octave_call_stack::pop();
         }
   }

// Make sure Octave is correctly unloaded. We cannot depend on Octave running
// at the (true) process atexit point, because at that time the Octave library
// might have been unloaded.
//
// At least that is the hypothesis, since Octave (in certain circumstances)
// cause a segmentation fault in do_octave_atexit called from the exit
// function. (One Octave call that causes this is "sleep(0)".)
   void atexit () {
#ifdef HAVE_USELOCALE
      // Set C locale
      locale_t old_locale = uselocale(c_locale);
#endif

      Py_BEGIN_ALLOW_THREADS
      do_octave_atexit();
      Py_END_ALLOW_THREADS

#ifdef HAVE_USELOCALE
      // Reset locale
      uselocale(old_locale);
#endif
   }

} /* namespace pytave }}} */

BOOST_PYTHON_MODULE(_pytave) { /* {{{ */
   using namespace boost::python;

   def("init", pytave::init);
   def("get_module_name", pytave::get_module_name);
   def("feval", pytave::func_eval);
   def("eval", pytave::str_eval);
   def("getvar", pytave::getvar);
   def("setvar", pytave::setvar);
   def("isvar", pytave::isvar);
   def("delvar", pytave::delvar);
   def("push_scope", pytave::push_scope);
   def("pop_scope", pytave::pop_scope);
   def("atexit", pytave::atexit);
   def("get_exceptions", pytave::get_exceptions);

   register_exception_translator<pytave::pytave_exception>(
      pytave::pytave_exception::translate_exception);

   register_exception_translator<pytave::octave_error_exception>(
      pytave::octave_error_exception::translate_exception);

   register_exception_translator<pytave::octave_parse_exception>(
      pytave::octave_parse_exception::translate_exception);

   register_exception_translator<pytave::object_convert_exception>(
      pytave::object_convert_exception::translate_exception);

   register_exception_translator<pytave::value_convert_exception>(
      pytave::value_convert_exception::translate_exception);

   register_exception_translator<pytave::variable_name_exception>(
      pytave::variable_name_exception::translate_exception);

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
