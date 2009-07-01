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

/* If your extension does not reside in a single file, there is an
 * additional step that is necessary. Be sure to define the symbol
 * PY_ARRAY_UNIQUE_SYMBOL to some name (the same name in all the files
 * comprising the extension), upstream from the include of
 * arrayobject.h. Typically this would be in some header file that is
 * included before arrayobject.h.
 */
#ifndef PYTAVE_DO_DECLARE_SYMBOL
#define NO_IMPORT_ARRAY
#endif
#define PY_ARRAY_UNIQUE_SYMBOL pytave_array_symbol
#include <Python.h>
#ifdef HAVE_NUMPY
#include <numpy/oldnumeric.h>
#include <numpy/old_defines.h>
// Avoid deprecation warnings from NumPy
#undef PyArray_FromDims 
#define PyArray_FromDims PyArray_SimpleNew
#else
#include <Numeric/arrayobject.h>
typedef int npy_intp;
#endif

/* Emacs
 * Local Variables:
 * fill-column:79
 * coding:utf-8
 * indent-tabs-mode:nil
 * c-basic-offset:3
 * End:
 * vim: set textwidth=79 expandtab shiftwidth=3 :
 */
