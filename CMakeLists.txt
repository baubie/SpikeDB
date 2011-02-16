cmake_minimum_required (VERSION 2.8 FATAL_ERROR)
project (SPIKEDB CXX)
find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-2.4)
pkg_check_modules(CAIRO cairo)
pkg_check_modules(SQLITE3 sqlite3)
pkg_check_modules(PYTHON python-2.7)

add_subdirectory(src)
