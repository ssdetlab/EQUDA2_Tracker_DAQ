set(BASE_OBJS
  ../src/AlpideConfig.cpp
  ../src/AlpideDebug.cpp
  ../src/AlpideDecoder.cpp
  ../src/BoardDecoder.cpp
  ../src/TAlpide.cpp
  ../src/TBoardConfig.cpp
  ../src/TBoardConfigMOSAIC.cpp
  ../src/TChipConfig.cpp
  ../src/TConfig.cpp
  ../src/TReadoutBoard.cpp
  ../src/TReadoutBoardMOSAIC.cpp
)

set(MOSAIC_OBJS
  ../MosaicSrc/TAlpideDataParser.cpp
  ../MosaicSrc/alpidercv.cpp
  ../MosaicSrc/controlinterface.cpp
  ../MosaicSrc/pexception.cpp
  ../MosaicSrc/trgrecorderparser.cpp
)

add_library(alpide SHARED ${BASE_OBJS} ${MOSAIC_OBJS})
target_include_directories(alpide
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ../inc ../MosaicSrc .. # .. is for ReadoutUnitSrc
)
target_link_libraries(alpide mosaic powerboard scopecontrol usb-1.0 tinyxml pthread)

target_compile_definitions(alpide PUBLIC "-DHAS_ROOT")
target_include_directories(alpide PUBLIC ${ROOT_INCLUDE_DIRS})
target_link_libraries(alpide ${ROOT_LIBRARIES} pthread)

install(TARGETS alpide
    LIBRARY DESTINATION lib
)