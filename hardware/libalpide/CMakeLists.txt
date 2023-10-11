set(BASE_OBJS
  ../src/AlpideConfig.cpp
  ../src/AlpideDebug.cpp
  ../src/AlpideDecoder.cpp
  ../src/BoardDecoder.cpp
  ../src/Common.cpp
  ../src/SetupHelpers.cpp
  ../src/TAlpide.cpp
  ../src/TApplyMask.cpp
  ../src/TBoardConfig.cpp
  ../src/TBoardConfigDAQ.cpp
  ../src/TBoardConfigMOSAIC.cpp
  ../src/TChipConfig.cpp
  ../src/TConfig.cpp
  ../src/TCycleAnalysis.cpp
  ../src/TDACScan.cpp
  ../src/TDCTRLAnalysis.cpp
  ../src/TDCTRLMeasurement.cpp
  ../src/TDataTaking.cpp
  ../src/TDigitalAnalysis.cpp
  ../src/TDigitalScan.cpp
  ../src/TDigitalWFAnalysis.cpp
  ../src/TEnduranceCycle.cpp
  ../src/TEyeMeasurement.cpp
  ../src/TFastPowerAnalysis.cpp
  ../src/TFastPowerTest.cpp
  ../src/TFifoAnalysis.cpp
  ../src/TFifoTest.cpp
  ../src/THIC.cpp
  ../src/THicConfig.cpp
  ../src/THisto.cpp
  ../src/TLocalBusAnalysis.cpp
  ../src/TLocalBusTest.cpp
  ../src/TNoiseAnalysis.cpp
  ../src/TNoiseOccupancy.cpp
  ../src/TPowerAnalysis.cpp
  ../src/TPowerBoard.cpp
  ../src/TPowerBoardConfig.cpp
  ../src/TPowerTest.cpp
  ../src/TReadoutAnalysis.cpp
  ../src/TReadoutBoard.cpp
  ../src/TReadoutBoardDAQ.cpp
  ../src/TReadoutBoardMOSAIC.cpp
  ../src/TReadoutTest.cpp
  ../src/TSCurveScan.cpp
  ../src/TScan.cpp
  ../src/TScanAnalysis.cpp
  ../src/TScanConfig.cpp
  ../src/TScanFactory.cpp
  ../src/TScanManager.cpp
  ../src/TestBeamTools.cpp
  ../src/USB.cpp
  ../src/USBHelpers.cpp
)

set(MOSAIC_OBJS
  ../MosaicSrc/TAlpideDataParser.cpp
  ../MosaicSrc/alpidercv.cpp
  ../MosaicSrc/controlinterface.cpp
  ../MosaicSrc/pexception.cpp
  ../MosaicSrc/trgrecorderparser.cpp
)

set(OBJS_ROOT
  ../src/TSCurveAnalysis.cpp
  ../src/TApplyTuning.cpp
  ../src/TEyeAnalysis.cpp
)

add_library(alpide SHARED ${BASE_OBJS} ${MOSAIC_OBJS})
target_include_directories(alpide
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
  PUBLIC ../inc ../MosaicSrc .. # .. is for ReadoutUnitSrc
)
target_link_libraries(alpide mosaic powerboard scopecontrol usb-1.0 tinyxml pthread)

target_compile_definitions(alpide PUBLIC "-DHAS_ROOT")
target_sources(alpide PRIVATE ${OBJS_ROOT})
target_include_directories(alpide PUBLIC ${ROOT_INCLUDE_DIRS})
target_link_libraries(alpide ${ROOT_LIBRARIES} pthread)

install(TARGETS alpide
        LIBRARY DESTINATION lib
)