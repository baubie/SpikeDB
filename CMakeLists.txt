CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT(SpikeDB CXX)

FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(Boost COMPONENTS python REQUIRED)
FIND_PACKAGE (PythonLibs)
pkg_check_modules(GTKMM gtkmm-2.4)
pkg_check_modules(CAIRO cairo)
pkg_check_modules(SQLITE3 sqlite3)

IF(APPLE)
	SET(PYTHON_INCLUDE_PATH "/opt/local/Library/Frameworks/Python.framework/Headers")
	SET(PYTHON_LIBRARIES "/opt/local/lib/libpython2.7.dylib")
	FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
ELSE(APPLE)
	SET(COREFOUNDATION_LIBRARY "")
ENDIF(APPLE)


ADD_SUBDIRECTORY(src)

