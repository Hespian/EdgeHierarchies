/*******************************************************************************
 * tests/edgeHierarchyGraphTests.cpp
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <gtest/gtest.h>

#include "edgeHierarchyGraph.h"

TEST(EdgeHierarchyGraphTest, hasCorrectSizeNoEdges) {
    EdgeHierarchyGraph g(5);
    EXPECT_EQ(g.getNumberOfNodes(), 5);
    EXPECT_EQ(g.getNumberOfEdges(), 0);
    for(NODE_T i = 0; i< 5; ++i) {
        EXPECT_EQ(g.getInDegree(i), 0);
        EXPECT_EQ(g.getOutDegree(i), 0);
    }
}


TEST(EdgeHierarchyGraphTest, AddEdgeDegree) {
    EdgeHierarchyGraph g(5);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 1, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(4, 3, 1);

    EXPECT_EQ(g.getNumberOfEdges(), 4);

    EXPECT_EQ(g.getInDegree(0), 0);
    EXPECT_EQ(g.getOutDegree(0), 0);

    EXPECT_EQ(g.getInDegree(1), 1);
    EXPECT_EQ(g.getOutDegree(1), 1);

    EXPECT_EQ(g.getInDegree(2), 1);
    EXPECT_EQ(g.getOutDegree(2), 2);

    EXPECT_EQ(g.getInDegree(3), 2);
    EXPECT_EQ(g.getOutDegree(3), 0);

    EXPECT_EQ(g.getInDegree(4), 0);
    EXPECT_EQ(g.getOutDegree(4), 1);
}


TEST(EdgeHierarchyGraphTest, AddEdgeHasEdge) {
    EdgeHierarchyGraph g(5);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 1, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(4, 3, 1);


    EXPECT_TRUE(g.hasEdge(1,2));
    EXPECT_TRUE(g.hasEdge(2,1));
    EXPECT_TRUE(g.hasEdge(2,3));
    EXPECT_TRUE(g.hasEdge(4,3));

    EXPECT_FALSE(g.hasEdge(3,2));
    EXPECT_FALSE(g.hasEdge(0,0));
}

TEST(EdgeHierarchyGraphTest, ForAllNeighbors) {
    EdgeHierarchyGraph g(5);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 2);
    g.addEdge(2, 1, 4);
    g.addEdge(4, 3, 3);

    vector<vector<pair<NODE_T, EDGEWEIGHT_T>>> expectedNeighborsIn(5);
    expectedNeighborsIn[1].push_back(make_pair(2, 4));
    expectedNeighborsIn[2].push_back(make_pair(1, 1));
    expectedNeighborsIn[3].push_back(make_pair(2, 2));
    expectedNeighborsIn[3].push_back(make_pair(4, 3));


    vector<vector<pair<NODE_T, EDGEWEIGHT_T>>> expectedNeighborsOut(5);
    expectedNeighborsOut[1].push_back(make_pair(2, 1));
    expectedNeighborsOut[2].push_back(make_pair(1, 4));
    expectedNeighborsOut[2].push_back(make_pair(3, 2));
    expectedNeighborsOut[4].push_back(make_pair(3, 3));

    vector<vector<pair<NODE_T, EDGEWEIGHT_T>>> actualNeighborsIn(5);
    for(NODE_T u = 0; u < g.getNumberOfNodes(); ++u) {
        g.forAllNeighborsIn(u,
                            [&] (NODE_T v, EDGEWEIGHT_T weight){
                                actualNeighborsIn[u].push_back(make_pair(v, weight));
                            });
    }


    for(NODE_T u = 0; u < g.getNumberOfNodes(); ++u) {
        sort(expectedNeighborsIn[u].begin(), expectedNeighborsIn[u].end());
        sort(actualNeighborsIn[u].begin(), actualNeighborsIn[u].end());
        ASSERT_EQ(actualNeighborsIn[u].size(), expectedNeighborsIn[u].size());
        for(size_t i = 0; i < actualNeighborsIn[u].size(); ++i) {
            EXPECT_EQ(actualNeighborsIn[u][i], expectedNeighborsIn[u][i]);
        }
    }

    vector<vector<pair<NODE_T, EDGEWEIGHT_T>>> actualNeighborsOut(5);
    for(NODE_T u = 0; u < g.getNumberOfNodes(); ++u) {
        g.forAllNeighborsOut(u,
                             [&] (NODE_T v, EDGEWEIGHT_T weight){
                                 actualNeighborsOut[u].push_back(make_pair(v, weight));
                             });
    }


    for(NODE_T u = 0; u < g.getNumberOfNodes(); ++u) {
        sort(expectedNeighborsOut[u].begin(), expectedNeighborsOut[u].end());
        sort(actualNeighborsOut[u].begin(), actualNeighborsOut[u].end());
        ASSERT_EQ(actualNeighborsOut[u].size(), expectedNeighborsOut[u].size());
        for(size_t i = 0; i < actualNeighborsOut[u].size(); ++i) {
            EXPECT_EQ(actualNeighborsOut[u][i], expectedNeighborsOut[u][i]);
        }
    }
}

TEST(EdgeHierarchyGraphTest, ForAllNodes) {
    EdgeHierarchyGraph g(5);

    vector<NODE_T> expected({0, 1, 2, 3, 4});

    vector<NODE_T> actual;

    g.forAllNodes([&] (NODE_T v) {
            actual.push_back(v);
        });

    sort(actual.begin(), actual.end());

    ASSERT_EQ(expected.size(), actual.size());
    for(size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(expected[i], actual[i]);
    }
}

TEST(EdgeHierarchyGraphTest, EdgeLevels) {
    EdgeHierarchyGraph g(3);

    g.addEdge(0, 1, 4);
    g.addEdge(1, 2, 3);
    g.addEdge(1, 0, 1);

    g.setEdgeLevel(1, 2, 1);
    g.setEdgeLevel(1, 0, 2);

    EXPECT_EQ(g.getEdgeLevel(0, 1), EDGELEVEL_INFINIY);
    EXPECT_EQ(g.getEdgeLevel(1, 2), 1);
    EXPECT_EQ(g.getEdgeLevel(1, 0), 2);

    vector<tuple<NODE_T, EDGELEVEL_T, EDGEWEIGHT_T>> result;
    auto logEdges = [&] (NODE_T v, EDGELEVEL_T level, EDGEWEIGHT_T weight) { result.push_back(make_tuple(v, level, weight)); };

    // Outgoing edges
    result.clear();
    g.forAllNeighborsOutWithHighLevel(0, 0, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(1u, EDGELEVEL_INFINIY, 4u));

    result.clear();
    g.forAllNeighborsOutWithHighLevel(0, EDGELEVEL_INFINIY, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(1u, EDGELEVEL_INFINIY, 4u));

    result.clear();
    g.forAllNeighborsOutWithHighLevel(1, 1, logEdges);
    ASSERT_GE(result.size(), 2);
    EXPECT_EQ(result.size(), 2);
    sort(result.begin(), result.end());
    EXPECT_EQ(result[0], make_tuple(0u, 2u, 1u));
    EXPECT_EQ(result[1], make_tuple(2u, 1u, 3u));

    result.clear();
    g.forAllNeighborsOutWithHighLevel(1, 2, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(0u, 2u, 1u));

    result.clear();
    g.forAllNeighborsOutWithHighLevel(1, 3, logEdges);
    EXPECT_EQ(result.size(), 0);

    // Incoming edges
    result.clear();
    g.forAllNeighborsInWithHighLevel(0, 1, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(1u, 2u, 1u));

    result.clear();
    g.forAllNeighborsInWithHighLevel(0, 2, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(1u, 2u, 1u));

    result.clear();
    g.forAllNeighborsInWithHighLevel(0, 3, logEdges);
    EXPECT_EQ(result.size(), 0);

    result.clear();
    g.forAllNeighborsInWithHighLevel(0, EDGELEVEL_INFINIY, logEdges);
    EXPECT_EQ(result.size(), 0);
}
