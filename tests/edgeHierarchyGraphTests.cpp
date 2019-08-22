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

TEST(EdgeHierarchyGraphTest, AddEdgeWeight) {
    EdgeHierarchyGraph g(4);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 1, 3);
    g.addEdge(2, 3, 2);

    EXPECT_EQ(g.getEdgeWeight(1, 2), 1);
    EXPECT_EQ(g.getEdgeWeight(2, 1), 3);
    EXPECT_EQ(g.getEdgeWeight(2, 3), 2);
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

    g.setEdgeRank(1, 2, 1);
    g.setEdgeRank(1, 0, 2);

    EXPECT_EQ(g.getEdgeRank(0, 1), EDGERANK_INFINIY);
    EXPECT_EQ(g.getEdgeRank(1, 2), 1);
    EXPECT_EQ(g.getEdgeRank(1, 0), 2);

    vector<tuple<NODE_T, EDGERANK_T, EDGEWEIGHT_T>> result;
    auto logEdges = [&] (NODE_T v, EDGERANK_T level, EDGEWEIGHT_T weight) { result.push_back(make_tuple(v, level, weight)); };

    // Outgoing edges
    result.clear();
    g.forAllNeighborsOutWithHighRank(0, 0, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(1u, EDGERANK_INFINIY, 4u));

    result.clear();
    g.forAllNeighborsOutWithHighRank(0, EDGERANK_INFINIY, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(1u, EDGERANK_INFINIY, 4u));

    result.clear();
    g.forAllNeighborsOutWithHighRank(1, 1, logEdges);
    ASSERT_GE(result.size(), 2);
    EXPECT_EQ(result.size(), 2);
    sort(result.begin(), result.end());
    EXPECT_EQ(result[0], make_tuple(0u, 2u, 1u));
    EXPECT_EQ(result[1], make_tuple(2u, 1u, 3u));

    result.clear();
    g.forAllNeighborsOutWithHighRank(1, 2, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(0u, 2u, 1u));

    result.clear();
    g.forAllNeighborsOutWithHighRank(1, 3, logEdges);
    EXPECT_EQ(result.size(), 0);

    // Incoming edges
    result.clear();
    g.forAllNeighborsInWithHighRank(0, 1, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(1u, 2u, 1u));

    result.clear();
    g.forAllNeighborsInWithHighRank(0, 2, logEdges);
    ASSERT_GE(result.size(), 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], make_tuple(1u, 2u, 1u));

    result.clear();
    g.forAllNeighborsInWithHighRank(0, 3, logEdges);
    EXPECT_EQ(result.size(), 0);

    result.clear();
    g.forAllNeighborsInWithHighRank(0, EDGERANK_INFINIY, logEdges);
    EXPECT_EQ(result.size(), 0);
}

TEST(EdgeHierarchyGraphTest, DecreaseEdgeWeightDuplicate) {
    EdgeHierarchyGraph g(2);
    g.addEdge(0, 1, 5);

    g.decreaseEdgeWeight(0, 1, 1);

    EXPECT_EQ(g.getEdgeWeight(0, 1), 1);
}

struct edge {
    NODE_T u;
    NODE_T v;
    EDGEWEIGHT_T weight;
};

TEST(EdgeHierarchyGraphTest, TurnCostSimple) {
    // This test is VERY implementation dependent!
    EdgeHierarchyGraph g(5);


    g.addEdge(0, 1, 1);
    g.addEdge(1, 3, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 1, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 1, 1);
    g.addEdge(3, 4, 1);
    g.addEdge(4, 2, 1);

    auto turnCostGraph = g.getTurnCostGraph(100);

    vector<edge> expectedEdges = {
        {0, 1, 1},
        {0, 2, 1},
        {1, 5, 101},
        {1, 6, 1},
        {2, 3, 101},
        {2, 4, 1},
        {3, 2, 101},
        {3, 1, 1},
        {4, 5, 1},
        {4, 6, 1},
        {5, 1, 101},
        {5, 2, 1},
        {6, 7, 1},
        {7, 3, 1},
        {7, 4, 1},
    };

    EXPECT_EQ(turnCostGraph.getNumberOfNodes(), 8);

    EXPECT_EQ(turnCostGraph.getNumberOfEdges(), expectedEdges.size());

    for(auto e: expectedEdges) {
        // cout << e.u << " " << e.v << endl;
        EXPECT_TRUE(turnCostGraph.hasEdge(e.u, e.v));
        EXPECT_EQ(turnCostGraph.getEdgeWeight(e.u, e.v), e.weight);
    }

}

TEST(EdgeHierarchyGraphTest, dfsOrderSimple) {
    EdgeHierarchyGraph g(4);

    g.addEdge(0, 1, 1);
    g.addEdge(3, 1, 3);
    g.addEdge(1, 2, 4);
    g.addEdge(1, 3, 2);
    g.addEdge(2, 0, 5);

    EdgeHierarchyGraph orderedG = g.getDFSOrderGraph<EdgeHierarchyGraph, false>();

    EXPECT_EQ(orderedG.getNumberOfEdges(), orderedG.getNumberOfEdges());
    EXPECT_EQ(orderedG.getNumberOfNodes(), orderedG.getNumberOfNodes());

    EXPECT_TRUE(orderedG.hasEdge(1, 2));
    EXPECT_EQ(orderedG.getEdgeWeight(1, 2), g.getEdgeWeight(3, 1));
    EXPECT_TRUE(orderedG.hasEdge(2, 1));
    EXPECT_EQ(orderedG.getEdgeWeight(2, 1), g.getEdgeWeight(1, 3));
    EXPECT_TRUE(orderedG.hasEdge(2, 0));
    EXPECT_EQ(orderedG.getEdgeWeight(2, 0), g.getEdgeWeight(1, 2));
    EXPECT_TRUE(orderedG.hasEdge(0, 3));
    EXPECT_EQ(orderedG.getEdgeWeight(0, 3), g.getEdgeWeight(2, 0));
    EXPECT_TRUE(orderedG.hasEdge(3, 2));
    EXPECT_EQ(orderedG.getEdgeWeight(3, 2), g.getEdgeWeight(0, 1));
}
