cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME CronTimerExample)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(CMAKE_CXX_FLAGS "-g -Wall")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819 /wd4267")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif ()

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	)

set(SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	)

set(CRON_TIMER_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/../include/cron_timer_mgr.h
	)

source_group(source FILES ${SOURCE_FILES})
source_group(source\\CronTimer FILES ${CRON_TIMER_SRC})

add_executable(${PROJECT_NAME}
	${SOURCE_FILES}
	${CRON_TIMER_SRC}
	)
