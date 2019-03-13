/*******************************************************************************
 * tests/shortcutHelperTests.cpp
 *
 * Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <vector>
#include <algorithm>

#include <gtest/gtest.h>

#include "definitions.h"
#include "edgeHierarchyGraph.h"
#include "edgeHierarchyQuery.h"
#include "shortcutHelper.h"

TEST(ShortcutHelperTest, SimpleTest) {
    EdgeHierarchyGraph g(5);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(2, 4, 1);
    g.addEdge(1, 4, 3);
    g.addEdge(0, 4, 4);
    g.addEdge(0, 3, 4);

    EdgeHierarchyQuery query(g);

    g.setEdgeRank(1, 2, 1);

    pair<vector<pair<NODE_T, NODE_T>>, vector<tuple<NODE_T, NODE_T, EDGEWEIGHT_T>>> shortestPathsLost = getShortestPathsLost<true>(1, 2, 1, g, query);

    ASSERT_EQ(shortestPathsLost.first.size(), 1);
    EXPECT_EQ(shortestPathsLost.first[0], make_pair(0u, 3u));

    ASSERT_EQ(shortestPathsLost.second.size(), 1);
    EXPECT_EQ(shortestPathsLost.second[0], make_tuple(1u, 4u, 2u));
}
