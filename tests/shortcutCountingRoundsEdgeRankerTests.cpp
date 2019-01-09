/*******************************************************************************
 * tests/shortcutCountingRoundsEdgeRankerTests.cpp
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <utility>
#include <unordered_set>

#include <gtest/gtest.h>

#include "edgeHierarchyGraph.h"
#include "edgeRanking/shortcutCountingRoundsEdgeRanker.h"

struct edgeHash {
    std::size_t operator()(const std::pair<NODE_T, NODE_T> &p) const {
        NODE_T u = p.first;
        NODE_T v = p.second;
        return std::hash<NODE_T>()((1/2) * (u + v) * (u + v + 1) + v);
    }
};

TEST(ShortcutCountingRoundsEdgeRankerTest, SimpleTest) {
    EdgeHierarchyGraph g(7);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 4, 1);
    g.addEdge(4, 5, 1);
    g.addEdge(6, 3, 1);


    ShortcutCountingRoundsEdgeRanker ranker(g);
    g.addEdge(2, 4, 3);
    ranker.addEdge(2, 4);


    std::unordered_set<std::pair<NODE_T, NODE_T>, edgeHash> firstRoundEdges;
    firstRoundEdges.insert(make_pair(0u, 1u));
    firstRoundEdges.insert(make_pair(2u, 3u));
    firstRoundEdges.insert(make_pair(2u, 4u));
    firstRoundEdges.insert(make_pair(6u, 3u));
    firstRoundEdges.insert(make_pair(4u, 5u));

    EDGELEVEL_T edgeLevel = 0;
    while(!firstRoundEdges.empty()) {
        ASSERT_TRUE(ranker.hasNextEdge());
        std::pair<NODE_T, NODE_T> nextEdge = ranker.getNextEdge();
        ASSERT_TRUE(firstRoundEdges.find(nextEdge) != firstRoundEdges.end());
        firstRoundEdges.erase(nextEdge);
        g.setEdgeLevel(nextEdge.first, nextEdge.second, edgeLevel++);
    }

    std::unordered_set<std::pair<NODE_T, NODE_T>, edgeHash> secondRoundEdges;
    secondRoundEdges.insert(make_pair(1u, 2u));
    secondRoundEdges.insert(make_pair(3u, 4u));

    while(!secondRoundEdges.empty()) {
        ASSERT_TRUE(ranker.hasNextEdge());
        std::pair<NODE_T, NODE_T> nextEdge = ranker.getNextEdge();
        ASSERT_TRUE(secondRoundEdges.find(nextEdge) != secondRoundEdges.end());
        secondRoundEdges.erase(nextEdge);
        g.setEdgeLevel(nextEdge.first, nextEdge.second, edgeLevel++);
    }

    EXPECT_FALSE(ranker.hasNextEdge());
}
