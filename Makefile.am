ACLOCAL_AMFLAGS = -I m4
EXTRA_lib_LTLIBRARIES = _pytave.la
EXTRA_DIST = test/test.py test/exceptions.py project-root.jam \
package/__init__.py package/pytave.py
EXTRA_libdir = @PYTAVE_MODULE_INSTALL_PATH@

_pytave_la_CPPFLAGS = @OCTAVE_CPPFLAGS@ @PYTHON_CPPFLAGS@ -I@NUMPY_INCLUDEDIR@

# Having -module in LDFLAGS lets us use a non-standard library name 
# (e.g. _pytave instead of libpytave)
_pytave_la_LDFLAGS = -module -L@OCTAVE_LIBRARYDIR@ -loctinterp -loctave  \
-lcruft -l@BOOST_PYTHON_LIB@ -R @PYTAVE_OCTAVE_RPATH@

_pytave_la_SOURCES = octave_to_python.cc pytave.cc python_to_octave.cc \
arrayobjectdefs.h octave_to_python.h pytavedefs.h exceptions.h   \
exceptions.cc python_to_octave.h config.h
