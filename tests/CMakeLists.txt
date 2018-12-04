################################################################################
# tests/CMakeLists.txt
#
# Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
#
# All rights reserved.
################################################################################

include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(tests "tests.cpp")

target_link_libraries(tests gtest gtest_main)

add_test(tests tests)
