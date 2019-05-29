################################################################################
# app/CMakeLists.txt
#
# Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
#
# All rights reserved.
################################################################################

# set (CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address ${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
# set (CMAKE_LINKER_FLAGS_DEBUG "-fsanitize=address ${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer")

add_custom_target(
   RoutingKit
   COMMAND make
   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/extern/RoutingKit/
)

find_package(Threads REQUIRED)
set(PTHREADLIBRARY ${CMAKE_THREAD_LIBS_INIT})


add_executable(benchmark benchmark.cpp)
target_compile_options(benchmark PRIVATE -Wall)
target_compile_options(benchmark PRIVATE -g)
target_link_libraries(benchmark tlx ${PROJECT_SOURCE_DIR}/extern/RoutingKit/lib/libroutingkit.so ${PTHREADLIBRARY})
add_dependencies(benchmark RoutingKit)


