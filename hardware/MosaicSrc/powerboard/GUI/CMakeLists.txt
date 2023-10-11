if(NOT Qt5_FOUND)
  return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRCS
  src/main.cpp
  src/pbMainWindow.cpp
  src/setValidator.cpp
  src/optionsDialog.cpp
  ../pbif.cpp
)

add_executable(pbGUI ${SRCS})
target_include_directories(pbGUI PRIVATE ..)
target_link_libraries(pbGUI Qt5::Gui Qt5::Core Qt5::Xml Qt5::Widgets mosaic powerboard)

if (PB_NEW)
	target_compile_definitions(pbGUI PRIVATE "-DPB_NEW")
endif()

install(TARGETS pbGUI
  RUNTIME DESTINATION bin)
