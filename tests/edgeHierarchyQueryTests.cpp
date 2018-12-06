/*******************************************************************************
 * tests/edgeHierarchyQueryTests.cpp
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <gtest/gtest.h>

#include "edgeHierarchyGraph.h"
#include "edgeHierarchyQuery.h"

TEST(EdgeHierarchyQueryTests, CanConstruct) {
    EdgeHierarchyGraph g(5);
    EdgeHierarchyQuery query(g);
}
