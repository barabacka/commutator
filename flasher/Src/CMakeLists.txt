cmake_minimum_required(VERSION 3.0)

project(flasher
    	LANGUAGES C
)

set ( VERSION_MAJOR  0 )
set ( VERSION_MINOR 11 )
set ( VERSION_PATCH  0 )

if ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ver.h )
	file ( READ ${CMAKE_CURRENT_SOURCE_DIR}/ver.h ver )
	string ( REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ ${ver} )
	set ( VERSION_MAJOR_OLD ${CMAKE_MATCH_1} )
	set ( VERSION_MINOR_OLD ${CMAKE_MATCH_2} )
	set ( VERSION_PATCH_OLD ${CMAKE_MATCH_3} )

	if ( DEFINED VERSION_PATCH_OLD )
		math ( EXPR VERSION_PATCH "${VERSION_PATCH_OLD} + 1" )
	endif ()
endif ()

set ( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/ver.h.in "../Src/ver.h" )

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
		bootloader_def.h
		bootloader_opt.h
		stm32.h
	)

add_executable(${PROJECT_NAME} ${SOURCE})
