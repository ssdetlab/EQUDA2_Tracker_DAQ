include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

# EUDAQ module lib
add_library(
    ${EUDAQ_MODULE} 
    SHARED 
    src/StaveProducer.cc  
    src/StaveROOTMonitor.cc  
    src/StaveThresholdScanner.cc  
    src/StaveFakeHitRateScanner.cc  
    src/StaveTTreeDataCollector.cc  
    src/StaveTTreeDataCollectorTLUSync.cc
    src/DummyProducer.cc
    src/DummyDataCollector.cc
)

target_include_directories(
    ${EUDAQ_MODULE} 
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# jsoncpp dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})

# module includes and ROOT dictionary for detector_event class
add_library(
    det_event_dict 
    SHARED 
    ./inc/DetectorEvent.h 
    ./inc/LinkDef.h)
set(DETECTOR_EVENT_DICT det_event_dict)
ROOT_GENERATE_DICTIONARY(
    detector_dict 
    DetectorEvent.h 
    MODULE 
    ${DETECTOR_EVENT_DICT} 
    LINKDEF 
    LinkDef.h)
target_link_libraries(
    ${DETECTOR_EVENT_DICT} 
    PRIVATE 
    ROOT::RIO)

find_package(OpenMP REQUIRED)

target_link_libraries(
    ${EUDAQ_MODULE} 
    PUBLIC
    ${EUDAQ_CORE_LIBRARY} 
    ${EUDAQ_TTREE_LIBRARY} 
    ${EUDAQ_ROOT_MONITOR_LIBRARY} 
    ${DETECTOR_EVENT_DICT} 
    ${ROOT_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    OpenMP::OpenMP_CXX
    alpide)          

# Threshold scan executable
add_executable(
    StaveThresholdScan 
    src/threshold_scan.cc)

# Fake hit rate scan executable 
add_executable(
    StaveFakeHitRateScan 
    src/fake_hit_rate_scan.cc)

target_link_libraries(
    StaveThresholdScan 
    PRIVATE 
    ${EUDAQ_MODULE})

target_link_libraries(
    StaveFakeHitRateScan 
    PRIVATE 
    ${EUDAQ_MODULE})

install(
    TARGETS ${EUDAQ_MODULE}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

install(
    TARGETS ${DETECTOR_EVENT_DICT}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

install(
    TARGETS StaveThresholdScan
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

install(
    TARGETS StaveFakeHitRateScan
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

# copy ROOT dictionary files for runtime access
set(DETECTOR_EVENT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libdet_event_dict_rdict.pcm)
set(DETECTOR_EVENT_MAP ${CMAKE_CURRENT_BINARY_DIR}/libdet_event_dict.rootmap)
install(
    FILES ${DETECTOR_EVENT_PCM} ${DETECTOR_EVENT_MAP}
    DESTINATION lib)
