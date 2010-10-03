link_directories(
    ${GTKMM_LIBRARY_DIRS} ${PLOTMM_LIBRARY_DIRS})

include_directories(
    ${GTKMM_INCLUDE_DIRS} ${PLOTMM_INCLUDE_DIRS})

add_definitions (-Wall -W)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -funroll-loops")

set (SRC spikedb.cpp gui.cpp spikedata.cpp)
add_executable (spikedb ${SRC})
target_link_libraries(spikedb ${GTKMM_LIBRARIES} ${PLOTMM_LIBRARIES})
