/*******************************************************************************
 * tests/edgeHierarchyQueryTests.cpp
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <cmath>

#include <gtest/gtest.h>

#include "edgeHierarchyGraph.h"
#include "edgeHierarchyQuery.h"

TEST(EdgeHierarchyQueryTests, GetDistanceNoLevels) {
    //          ---3---
    //         |       |
    //         v       |
    // o -1--> o --1-> o
    // |               ^
    // |               |
    //  -------3-------

    EdgeHierarchyGraph g(3);
    EdgeHierarchyQuery query(g);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 1, 3);
    g.addEdge(0, 2, 3);

    // Simple test
    EDGEWEIGHT_T distance = query.getDistance(0, 2);
    EXPECT_EQ(distance, 2);

    // Second query
    distance = query.getDistance(2, 1);
    EXPECT_EQ(distance, 3);

    // No path
    distance = query.getDistance(2, 0);
    EXPECT_EQ(distance, EDGEWEIGHT_INFINITY);

    // s = t
    distance = query.getDistance(2, 2);
    EXPECT_EQ(distance, 0);
}

TEST(EdgeHierarchyQueryTests, OnlyUpwards) {
    EdgeHierarchyGraph g(5);
    EdgeHierarchyQuery query(g);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 4, 1);
    g.setEdgeRank(0, 1, 1);
    g.setEdgeRank(1, 2, 2);
    g.setEdgeRank(2, 3, 3);
    g.setEdgeRank(3, 4, 4);

    EDGEWEIGHT_T distance = query.getDistance(0, 4);
    EXPECT_EQ(distance, 4);
}

TEST(EdgeHierarchyQueryTests, OnlyDownwards) {
    EdgeHierarchyGraph g(5);
    EdgeHierarchyQuery query(g);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 4, 1);
    g.setEdgeRank(0, 1, 4);
    g.setEdgeRank(1, 2, 3);
    g.setEdgeRank(2, 3, 2);
    g.setEdgeRank(3, 4, 1);

    EDGEWEIGHT_T distance = query.getDistance(0, 4);
    EXPECT_EQ(distance, 4);
}

TEST(EdgeHierarchyQueryTests, SimpleUpDown) {
    EdgeHierarchyGraph g(5);
    EdgeHierarchyQuery query(g);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 4, 1);
    g.setEdgeRank(0, 1, 0);
    g.setEdgeRank(1, 2, 2);
    g.setEdgeRank(2, 3, 3);
    g.setEdgeRank(3, 4, 1);

    EDGEWEIGHT_T distance = query.getDistance(0, 4);
    EXPECT_EQ(distance, 4);
}

TEST(EdgeHierarchyQueryTests, ImpossibleLevels) {
    EdgeHierarchyGraph g(5);
    EdgeHierarchyQuery query(g);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 4, 1);
    g.setEdgeRank(0, 1, 2);
    g.setEdgeRank(1, 2, 0);
    g.setEdgeRank(2, 3, 1);
    g.setEdgeRank(3, 4, 3);

    EDGEWEIGHT_T distance = query.getDistance(0, 4);
    EXPECT_EQ(distance, EDGERANK_INFINIY);
}

TEST(EdgeHierarchyQueryTests, PartialHierarchy) {
    EdgeHierarchyGraph g(5);
    EdgeHierarchyQuery query(g);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 4, 1);
    g.setEdgeRank(0, 1, 0);
    g.setEdgeRank(1, 2, 1);

    EDGEWEIGHT_T distance = query.getDistance(0, 4);
    EXPECT_EQ(distance, 4);
}

TEST(EdgeHierarchyQueryTests, OnePossibleOneImpossible) {
    EdgeHierarchyGraph g(5);
    EdgeHierarchyQuery query(g);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 4, 1);
    g.addEdge(0, 2, 3);
    g.addEdge(2, 4, 3);
    g.setEdgeRank(0, 1, 2);
    g.setEdgeRank(1, 2, 0);
    g.setEdgeRank(2, 3, 1);
    g.setEdgeRank(3, 4, 3);
    g.setEdgeRank(0, 2, 4);
    g.setEdgeRank(2, 4, 5);

    EDGEWEIGHT_T distance = query.getDistance(0, 4);
    EXPECT_EQ(distance, 6);
}

TEST(EdgeHierarchyGraphTest, LongBidirectedPath) {
    EdgeHierarchyGraph g(10);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 4, 1);
    g.addEdge(4, 5, 1);
    g.addEdge(5, 6, 1);
    g.addEdge(6, 7, 1);
    g.addEdge(7, 8, 1);
    g.addEdge(8, 9, 1);


    g.addEdge(1, 0, 1);
    g.addEdge(2, 1, 1);
    g.addEdge(3, 2, 1);
    g.addEdge(4, 3, 1);
    g.addEdge(5, 4, 1);
    g.addEdge(6, 5, 1);
    g.addEdge(7, 6, 1);
    g.addEdge(8, 7, 1);
    g.addEdge(9, 8, 1);

    EdgeHierarchyQuery query(g);

    for(int u = 0; u < 10; ++u){
        for(int v = 0; v < 10; ++v){
            EDGEWEIGHT_T distance = std::abs(u - v);
            EXPECT_EQ(query.getDistance(u, v), distance);
        }
    }
}

TEST(EdgeHierarchyQueryTests, MaximumDistanceExact) {
    EdgeHierarchyGraph g(2);
    g.addEdge(0, 1, 1);
    EdgeHierarchyQuery query(g);

    EXPECT_EQ(query.getDistance(0, 1, 1), 1);
}

//TEST(EdgeHierarchyQueryTests, DirectPathNotShortestPath) {
//    EdgeHierarchyGraph g(5);
//    g.addEdge(0, 4, 1);
//    g.addEdge(4, 1, 2824);
//    g.addEdge(0, 1, 2826);
//    g.addEdge(1, 2, 583);
//    g.addEdge(2, 3, 3209);
//    g.addEdge(0, 3, 8281);
//    EdgeHierarchyQuery query(g);
//
//    g.setEdgeRank(0, 1, 406939);
//    g.setEdgeRank(1, 2, EDGERANK_INFINIY - 1);
//    g.setEdgeRank(0, 4, EDGERANK_INFINIY);
//    g.setEdgeRank(4, 1, EDGERANK_INFINIY);
//    g.setEdgeRank(2, 3, EDGERANK_INFINIY);
//    g.setEdgeRank(0, 3, EDGERANK_INFINIY);
//
//    EXPECT_EQ(query.getDistance(0, 3, 6618), 6618);
//}
