CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT(SpikeDB)

IF(APPLE)
	SET(BOOST_ROOT "/opt/local/")
	SET(CMAKE_LIBRARY_PATH /opt/local/lib ${CMAKE_LIBRARY_PATH} )
	SET(CMAKE_MODULE_PATH /opt/local/lib)
	SET(PYTHON_INCLUDE_PATH "/opt/local/Library/Frameworks/Python.framework/Headers")
	SET(PYTHON_LIBRARIES "/opt/local/lib/libpython2.7.dylib")
	SET(PYTHON_LIBRARY_DIRS "/opt/local/lib/")
	FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
ELSE(APPLE)
	SET(COREFOUNDATION_LIBRARY "")
ENDIF(APPLE)

FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(PythonLibs)
FIND_PACKAGE(Boost COMPONENTS python REQUIRED)
pkg_check_modules(GTKMM gtkmm-2.4)
pkg_check_modules(CAIRO cairo)
pkg_check_modules(SQLITE3 sqlite3)

ADD_SUBDIRECTORY(src)

