CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT(SpikeDB CXX)

FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(Boost COMPONENTS python REQUIRED)
FIND_PACKAGE ( PythonLibs )
FIND_PACKAGE ( PythonInterp )
pkg_check_modules(GTKMM gtkmm-2.4)
pkg_check_modules(CAIRO cairo)
pkg_check_modules(SQLITE3 sqlite3)

IF(APPLE)
	SET(PYTHON_INCLUDE_PATH "/opt/local/Library/Frameworks/Python.framework/Headers")
	SET(PYTHON_LIBRARIES "/opt/local/lib/libpython2.7.dylib")
ENDIF(APPLE)


if(APPLE)
INCLUDE(InstallRequiredSystemLibraries)
SET(VERSION "1.1.0")
SET(CPACK_BUNDLE_IDENTIFIER ca.aubie.spikedb)
SET(CPACK_PACKAGE_FILE_NAME "SpikeDB")
SET(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/SpikeDB.icns)
SET(CPACK_BUNDLE_ICON ${CMAKE_SOURCE_DIR}/SpikeDB.icns)
SET(CPACK_BUNDLE_NAME "SpikeDB")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SpikeDB")
SET(CPACK_PACKAGE_VENDOR "Brandon Aubie")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_EXECUTABLES "spikedb;spikedb")
SET(CPACK_BUNDLE_PLIST "info.plist")
SET(CPACK_BUNDLE_STARTUP_COMMAND "src/spikedb")
INCLUDE(CPack)

# image generation
execute_process(COMMAND sw_vers -productVersion COMMAND cut -d. -f1,2
    OUTPUT_VARIABLE OSX_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
set(BUNDLE_DMG ${PROJECT_NAME}-mac-${OSX_VERSION}.dmg)
set(BUNDLE_LOCATION ${CMAKE_BINARY_DIR}/src/SpikeDB.app)
set(_dmg_temp ${BUNDLE_DMG}.temp)
add_custom_command(OUTPUT ${BUNDLE_DMG}
    COMMAND rm -rf ${_dmg_temp} ${BUNDLE_DMG}
    COMMAND mkdir ${_dmg_temp}
    COMMAND cp -r ${BUNDLE_LOCATION} ${_dmg_temp}
	COMMAND ln -s /Applications ${_dmg_temp}/Applications
	COMMAND hdiutil create ${BUNDLE_DMG} -srcdir ${_dmg_temp} -fs HFS+ -volname "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}"
    COMMAND rm -rf ${_dmg_temp}
    DEPENDS spikedb)
add_custom_target(package DEPENDS ${BUNDLE_DMG})
endif(APPLE)


ADD_SUBDIRECTORY(src)
