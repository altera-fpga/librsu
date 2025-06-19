# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

set_target_properties(uniLibRSU PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/bin"
)

message(${PROJECT_SOURCE_DIR})
target_include_directories(uniLibRSU
		PRIVATE
		"../platform_include"
		PUBLIC
        ${PROJECT_SOURCE_DIR}/../FreeRTOS/Fat-Fs/portable/common/
        ${PROJECT_SOURCE_DIR}/../FreeRTOS/Fat-Fs/include/
        ${PROJECT_SOURCE_DIR}/../osal/freertos/
		${FREERTOS_CONFIG_HEADER_PATH}
        ${driver_include_dirs}
        ${RTOS_INCLUDES}
    )
target_compile_options(uniLibRSU PRIVATE
  -march=armv8-a+crc
  -Wno-format
  -Wno-format-security
)

add_subdirectory(dependency)
target_link_libraries(uniLibRSU socfpga_drivers)
