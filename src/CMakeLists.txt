# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

target_include_directories(uniLibRSU PRIVATE "priv_include")

target_sources(uniLibRSU PRIVATE "libRSU_utils.c")
target_sources(uniLibRSU PRIVATE "libRSU_cfg.c")
target_sources(uniLibRSU PRIVATE "libRSU.c")
target_sources(uniLibRSU PRIVATE "libRSU_ops.c")
target_sources(uniLibRSU PRIVATE "libRSU_misc.c")
target_sources(uniLibRSU PRIVATE "libRSU_cb.c")
target_sources(uniLibRSU PRIVATE "libRSU_image.c")

target_compile_options(uniLibRSU PRIVATE -Wformat -Wformat-signedness)
