cmake_minimum_required(VERSION 3.14)

project(QTlayout2 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets )
include_directories(${X11_INCLUDE_DIRS})

add_executable(QTlayout2
  main.cpp
)
target_link_libraries(QTlayout2 Qt5::Widgets)

IF (NOT WIN32)
	find_package(Qt5 REQUIRED X11Extras)
	target_link_libraries(QTlayout2 ${X11_LIBRARIES} Qt5::X11Extras)
ENDIF()
