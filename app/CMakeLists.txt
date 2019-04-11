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
target_link_libraries(benchmark tlx ${PROJECT_SOURCE_DIR}/extern/RoutingKit/lib/libroutingkit.so)

add_executable(calculateMinimalSearchSpace calculateMinimalSearchSpace.cpp)
target_compile_options(calculateMinimalSearchSpace PRIVATE -Wall)
target_compile_options(calculateMinimalSearchSpace PRIVATE -g)
target_link_libraries(calculateMinimalSearchSpace tlx ${PROJECT_SOURCE_DIR}/extern/RoutingKit/lib/libroutingkit.so)

add_executable(CHEHCombination CHEHCombination.cpp)
target_compile_options(CHEHCombination PRIVATE -Wall)
target_compile_options(CHEHCombination PRIVATE -g)
target_link_libraries(CHEHCombination tlx ${PROJECT_SOURCE_DIR}/extern/RoutingKit/lib/libroutingkit.so)

add_executable(benchmarkTurncosts benchmarkTurncosts.cpp)
target_compile_options(benchmarkTurncosts PRIVATE -Wall)
target_compile_options(benchmarkTurncosts PRIVATE -g)
target_link_libraries(benchmarkTurncosts tlx ${PROJECT_SOURCE_DIR}/extern/RoutingKit/lib/libroutingkit.so)
