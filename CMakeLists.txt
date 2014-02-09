# Copyright © 2014 Fabian Schuiki
# This file builds the Auris Radio project.

cmake_minimum_required(VERSION 2.8)
project(AurisRadio)

# Default to debug build.
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions("-DBUILD_DEBUG")
else()
	add_definitions("-DBUILD_RELEASE")
endif()


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")
find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(Auris REQUIRED)
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
if (NOT APPLE)
	find_package(LibUUID REQUIRED)
endif()
include_directories(${Boost_INCLUDE_DIRS} ${LibUUID_INCLUDE_DIRS} ${Auris_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})


add_executable(aurisradio
	source/main.cpp
	source/MainWindow.cpp
	source/Jockey.cpp)
target_link_libraries(aurisradio ${Boost_LIBRARIES} ${LibUUID_LIBRARIES} ${GTKMM_LIBRARIES} vlc)
install(TARGETS aurisradio RUNTIME DESTINATION bin)