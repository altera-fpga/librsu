# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

if(NOT DEFINED FREERTOS_TOOLCHAIN)
    message(FATAL_ERROR "not building inside inside FreeRTOS")
endif()
set(SHARED_LIB OFF)

