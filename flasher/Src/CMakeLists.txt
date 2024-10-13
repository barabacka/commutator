cmake_minimum_required(VERSION 3.0)

project(flasher)

set ( SOURCE
		flasher.c
		bootloader.c
		uart.c
	)

set ( HEADER
		uart.h
		bootloader.h
		bootloader_opt.h
	)

add_executable(${PROJECT_NAME} ${SOURCE})
