# Copyright (c) 2015, Pierre-Andre Saulais <pasaulais@free.fr>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.8)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/teensy-arm.toolchain.cmake")

project(TeensyCMake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


set(TARGET_NAME test-cmake)
set(TY_EXECUTABLE "/home/xoza/bin/teensy_loader_cli" CACHE FILEPATH "Path to the 'teensy_loader_cli' executable that can upload programs to the Teensy")

set(TEENSY_C_CORE_FILES
	    ${TEENSY_ROOT}/analog.c
	    ${TEENSY_ROOT}/avr_emulation.cpp
	    ${TEENSY_ROOT}/Print.cpp
	)


# FreeRTOS	
AUX_SOURCE_DIRECTORY(FreeRTOS FREERTOS_SOURCES) 
AUX_SOURCE_DIRECTORY(FreeRTOS/portable/GCC/ARM_CM3 FREERTOS_SOURCES_PORTABLE) 
include_directories(SYSTEM FreeRTOS/include FreeRTOS/portable/GCC/ARM_CM3/)

AUX_SOURCE_DIRECTORY(/home/xoza/src/ILI9341_t3/ TFT_SOURCES) 
include_directories( /home/xoza/src/ILI9341_t3/ ) 
# SPI
AUX_SOURCE_DIRECTORY(${ARDUINO_LIB_ROOT}/SPI/ SPI_LIB_SOURCES) 
include_directories(SYSTEM ${ARDUINO_LIB_ROOT}/SPI/ ) 

# project
AUX_SOURCE_DIRECTORY(src PROJECT_SOURCES) 
AUX_SOURCE_DIRECTORY(audio AUDIO_SOURCES) 
AUX_SOURCE_DIRECTORY(instruments INSTR_SOURCES) 
include_directories(.)
ADD_EXECUTABLE(${TARGET_NAME} ${PROJECT_SOURCES} ${AUDIO_SOURCES} ${TEENSY_C_CORE_FILES} ${FREERTOS_SOURCES} ${FREERTOS_SOURCES_PORTABLE} ${SPI_LIB_SOURCES} ${TFT_SOURCES} ${INSTR_SOURCES})
set_target_properties(${TARGET_NAME} PROPERTIES
	OUTPUT_NAME ${TARGET_NAME}
	SUFFIX ".elf"
)
set(TARGET_ELF "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}.elf")

# firmware build/upload
# Generate the hex firmware files that can be flashed to the MCU.
set(EEPROM_OPTS -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0)
set(HEX_OPTS -O ihex -R .eeprom)
add_custom_command(OUTPUT ${TARGET_ELF}.eep
				   COMMAND ${CMAKE_OBJCOPY} ${EEPROM_OPTS} ${TARGET_ELF} ${TARGET_ELF}.eep
				   DEPENDS ${TARGET_ELF})
add_custom_command(OUTPUT ${TARGET_ELF}.hex
				   COMMAND ${CMAKE_OBJCOPY} ${HEX_OPTS} ${TARGET_ELF} ${TARGET_ELF}.hex
				   DEPENDS ${TARGET_ELF})
add_custom_target(${TARGET_NAME}_Firmware ALL
				  DEPENDS ${TARGET_ELF}.eep ${TARGET_ELF}.hex)
add_dependencies(${TARGET_NAME}_Firmware ${TARGET_NAME})

if(EXISTS "${TY_EXECUTABLE}")
	add_custom_target(upload
					  DEPENDS ${TY_EXECUTABLE} ${TARGET_ELF}.hex
					  COMMAND "${TY_EXECUTABLE}" -mmcu=mk20dx256 -s -w -v ${TARGET_ELF}.hex)
	add_dependencies(upload ${TARGET_NAME}_Firmware)
endif(EXISTS "${TY_EXECUTABLE}")

#subdirs(sketches)
