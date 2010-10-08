link_directories(
    ${GTKMM_LIBRARY_DIRS} ${SQLITE3_LIBRARY_DIRS} ${CAIRO_LIBRARY_DIRS})

include_directories(
    ${GTKMM_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS})

add_definitions (-Wall -W)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -funroll-loops")

set (SRC spikedb.cpp gui.cpp spikedata.cpp easyplotmm/easyplotmm.cpp)
add_executable (spikedb ${SRC})
target_link_libraries(spikedb ${GTKMM_LIBRARIES} ${SQLITE3_LIBRARIES} ${CAIRO_LIBRARIES})
