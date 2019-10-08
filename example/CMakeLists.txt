PROJECT(CronTimerTest)
cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g")
ENDIF()

#设置启动项目
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CronTimerTest)

add_executable(CronTimerTest
	main.cpp
	)

