/*******************************************************************************
 * tests/edgeHierarchyConstructionTests.cpp
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <queue>
#include <utility>

#include <gtest/gtest.h>

#include "edgeHierarchyGraph.h"
#include "edgeHierarchyQuery.h"
#include "edgeHierarchyConstruction.h"


class ArbitraryOrderEdgeRanker {
public:
    ArbitraryOrderEdgeRanker(EdgeHierarchyGraph &g) : g(g) {
        g.forAllNodes( [&] (NODE_T u) {
                g.forAllNeighborsOut(u, [&] (NODE_T v, EDGEWEIGHT_T weight) {
                        edgeQueue.push(make_pair(u, v));
                    });
            });
    }

    void addEdge(NODE_T u, NODE_T v) {
        edgeQueue.push(make_pair(u, v));
    }

    pair<NODE_T, NODE_T> getNextEdge() {
        auto nextEdge(edgeQueue.front());
        edgeQueue.pop();
        return nextEdge;
    }

    bool hasNextEdge() {
        return !edgeQueue.empty();
    }

private:
    EdgeHierarchyGraph &g;
    queue<pair<NODE_T, NODE_T>> edgeQueue;
};


TEST(EdgeHierarchyConstructionTest, SetEdgeLevelSimple) {
    EdgeHierarchyGraph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    EdgeHierarchyQuery query(g);

    EdgeHierarchyConstruction<ArbitraryOrderEdgeRanker> construction(g, query);

    construction.setEdgeLevel(1, 2, 1);

    EXPECT_EQ(g.getEdgeLevel(1, 2), 1);

    EXPECT_TRUE(g.hasEdge(0, 2) || g.hasEdge(1,3));

    if(g.hasEdge(0, 2)) {
        EXPECT_EQ(g.getEdgeLevel(0, 2), EDGELEVEL_INFINIY);
        EXPECT_EQ(g.getEdgeWeight(0, 2), 2);
    }
    else {
        EXPECT_EQ(g.getEdgeLevel(1, 3), EDGELEVEL_INFINIY);
        EXPECT_EQ(g.getEdgeWeight(1, 3), 2);
    }

    EXPECT_EQ(query.getDistance(0, 3), 3);
}

TEST(EdgeHierarchyConstructionTest, SetEdgeLevelNoNewEdge) {
    EdgeHierarchyGraph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(0, 3, 3);

    EdgeHierarchyQuery query(g);

    EdgeHierarchyConstruction<ArbitraryOrderEdgeRanker> construction(g, query);

    construction.setEdgeLevel(1, 2, 1);

    EXPECT_EQ(g.getEdgeLevel(1, 2), 1);

    EXPECT_FALSE(g.hasEdge(0, 2) || g.hasEdge(1,3));

    EXPECT_EQ(query.getDistance(0, 3), 3);
}


TEST(EdgeHierarchyConstructionTest, RunSimple) {
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

    EdgeHierarchyConstruction<ArbitraryOrderEdgeRanker> construction(g, query);


    EdgeHierarchyGraph originalGraph(g);
    EdgeHierarchyQuery originalGraphQuery(originalGraph);


    construction.run();

    g.forAllNodes( [&] (NODE_T u) {
            g.forAllNeighborsOut(u, [&] (NODE_T v, EDGEWEIGHT_T weight) {
                    EXPECT_LT(g.getEdgeLevel(u, v), EDGEWEIGHT_INFINITY);
                });
        });

    // Sanity check
    EXPECT_EQ(originalGraph.getNumberOfEdges(), 18);

    for(NODE_T u = 0; u < 10; ++u){
        for(NODE_T v = 0; v < 10; ++v){
            EXPECT_EQ(query.getDistance(u, v), originalGraphQuery.getDistance(u,v));
        }
    }
}
