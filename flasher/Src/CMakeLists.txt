cmake_minimum_required(VERSION 3.0)

project(flasher
		VERSION 0.3
    	LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
enable_language(C)

set ( SOURCE
		flasher.c
		common.c
		bootloader.c
		uart.c
	)

set ( HEADER
		common.h
		uart.h
		bootloader_opt.h
	)

add_executable(${PROJECT_NAME} ${SOURCE})
