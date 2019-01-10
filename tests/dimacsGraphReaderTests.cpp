/*******************************************************************************
 * tests/dimacsGraphReaderTests.cpp
 *
 * Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <gtest/gtest.h>

#include "dimacsGraphReader.h"

TEST(DimacsGraphReaderTests, SimpleTest) {
    EdgeHierarchyGraph g = readGraphDimacs("exampleGraph.dimacs");

    EXPECT_EQ(g.getNumberOfNodes(), 6);
    EXPECT_EQ(g.getNumberOfEdges(), 5);

    EXPECT_TRUE(g.hasEdge(0, 1));
    EXPECT_EQ(g.getEdgeWeight(0, 1), 3);

    EXPECT_TRUE(g.hasEdge(1, 0));
    EXPECT_EQ(g.getEdgeWeight(1, 0), 3);

    EXPECT_TRUE(g.hasEdge(1, 2));
    EXPECT_EQ(g.getEdgeWeight(1, 2), 2);

    EXPECT_TRUE(g.hasEdge(2, 1));
    EXPECT_EQ(g.getEdgeWeight(2, 1), 1);

    EXPECT_TRUE(g.hasEdge(2, 3));
    EXPECT_EQ(g.getEdgeWeight(2, 3), 4);

    EXPECT_EQ(g.getInDegree(4), 0);
    EXPECT_EQ(g.getOutDegree(4), 0);
}
