include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src MODULE_SRC)

add_library(${EUDAQ_MODULE} SHARED ${MODULE_SRC})

# jsoncpp dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})
link_libraries(${JSONCPP_LIBRARIES})

# module includes and ROOT dictionary for detector_event class
include_directories(./inc)
add_library(det_event_dict SHARED ./inc/DetectorEvent.h ./inc/LinkDef.h)
set(DETECTOR_EVENT_DICT det_event_dict)
ROOT_GENERATE_DICTIONARY(detector_dict DetectorEvent.h MODULE ${DETECTOR_EVENT_DICT} LINKDEF LinkDef.h)
target_link_libraries(${DETECTOR_EVENT_DICT} PRIVATE ROOT::RIO)

find_package(OpenMP REQUIRED)

target_link_libraries(${EUDAQ_MODULE} 
                      ${EUDAQ_CORE_LIBRARY} 
                      ${EUDAQ_TTREE_LIBRARY} 
                      ${EUDAQ_ROOT_MONITOR_LIBRARY} 
                      ${DETECTOR_EVENT_DICT} 
                      ${ROOT_LIBRARIES}
                      ${JSONCPP_LIBRARIES}
                      OpenMP::OpenMP_CXX
                      alpide)          

install(TARGETS ${EUDAQ_MODULE}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(TARGETS ${DETECTOR_EVENT_DICT}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

# copy ROOT dictionary files for runtime access
set(DETECTOR_EVENT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libdet_event_dict_rdict.pcm)
set(DETECTOR_EVENT_MAP ${CMAKE_CURRENT_BINARY_DIR}/libdet_event_dict.rootmap)
install(FILES ${DETECTOR_EVENT_PCM} ${DETECTOR_EVENT_MAP}
        DESTINATION lib)
