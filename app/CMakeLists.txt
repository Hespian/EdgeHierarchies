################################################################################
# app/CMakeLists.txt
#
# Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
#
# All rights reserved.
################################################################################

add_executable(benchmark benchmark.cpp)
target_compile_options(benchmark PRIVATE -Wall)
target_compile_options(benchmark PRIVATE -g)
target_link_libraries(benchmark tlx)
