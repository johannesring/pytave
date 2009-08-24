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

#include <boost/python.hpp>
#include "exceptions.h"

namespace pytave {

	PyObject *octave_error_exception::excclass = NULL;
	PyObject *value_convert_exception::excclass = NULL;
	PyObject *object_convert_exception::excclass = NULL;
	PyObject *octave_parse_exception::excclass = NULL;
	PyObject *variable_name_exception::excclass = NULL;

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
