################################################################################
# tests/CMakeLists.txt
#
# Copyright (C) 2018-2019 Demian Hespe <hespe@kit.edu>
#
# All rights reserved.
################################################################################

include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# add_custom_target(
#    RoutingKit
#    COMMAND make
#    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/extern/RoutingKit/
# )

function(buildAndAddTest TESTFILE)
  string(REPLACE ".cpp" "" TESTNAME "${TESTFILE}")
  add_executable(${TESTNAME} ${TESTFILE})
  target_compile_options(${TESTNAME} PRIVATE -Wall)
  target_link_libraries(${TESTNAME} gtest gtest_main ${PROJECT_SOURCE_DIR}/extern/RoutingKit/lib/libroutingkit.so)
  add_dependencies(${TESTNAME} RoutingKit)
  add_test(${TESTNAME} ${TESTNAME})
endfunction()

buildAndAddTest("edgeHierarchyGraphTests.cpp")
buildAndAddTest("edgeHierarchyQueryTests.cpp")
buildAndAddTest("edgeHierarchyConstructionTests.cpp")
buildAndAddTest("edgeIdCreatorTests.cpp")
buildAndAddTest("bipartiteMinimumVertexCoverTests.cpp")
buildAndAddTest("arraySetTests.cpp")
buildAndAddTest("shortcutHelperTests.cpp")
buildAndAddTest("shortcutCountingRoundsEdgeRankerTests.cpp")
buildAndAddTest("dimacsGraphReaderTests.cpp")
configure_file(exampleGraph.dimacs exampleGraph.dimacs COPYONLY)
