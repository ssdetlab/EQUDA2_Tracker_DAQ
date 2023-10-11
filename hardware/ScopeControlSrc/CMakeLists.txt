set(SRCS
  scope_control.cpp
  serial/src/serial.cc
  serial/src/impl/unix.cc
  serial/src/impl/list_ports/list_ports_linux.cc
)

add_library(scopecontrol SHARED ${SRCS})
target_include_directories(scopecontrol PUBLIC . serial/include)
target_link_libraries(scopecontrol rt)
install(TARGETS scopecontrol
  LIBRARY DESTINATION lib)
