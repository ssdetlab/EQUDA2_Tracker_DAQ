set(SRCS
  src/genconsumer.cpp
  src/i2cbus.cpp
  src/i2cslave.cpp
  src/i2csyspll.cpp
  src/ipbus.cpp
  src/ipbusudp.cpp
  src/mboard.cpp
  src/mcoordinator.cpp
  src/mdatagenerator.cpp
  src/mdatareceiver.cpp
  src/mdatasave.cpp
  src/mexception.cpp
  src/mruncontrol.cpp
  src/mservice.cpp
  src/mtriggercontrol.cpp
  src/mwbbslave.cpp
  src/pulser.cpp
  src/trgrecorder.cpp
)

add_library(mosaic SHARED ${SRCS})
target_include_directories(mosaic PUBLIC include)
install(TARGETS mosaic
  LIBRARY DESTINATION lib)
