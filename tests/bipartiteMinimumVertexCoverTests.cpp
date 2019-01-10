/*******************************************************************************
 * tests/bipartiteMinimumVertexCoverTests.cpp
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <vector>
#include <utility>
#include <algorithm>

#include <gtest/gtest.h>

#include "definitions.h"
#include "bipartiteMinimumVertexCover.h"

TEST(bipartiteMinimumVertexCoverTest, SimpleTest) {
    BipartiteMinimumVertexCover bipartiteMVC(10);
    vector<pair<NODE_T, NODE_T>> edges;
    edges.push_back(make_pair(1u, 5u));
    edges.push_back(make_pair(2u, 5u));
    edges.push_back(make_pair(3u, 4u));
    edges.push_back(make_pair(3u, 5u));

    EXPECT_EQ(bipartiteMVC.getMinimumVertexCoverSize(edges), 2);

    auto mvc = bipartiteMVC.getMinimumVertexCover(edges);
    EXPECT_EQ(bipartiteMVC.getMinimumVertexCoverSize(edges), mvc.first.size() + mvc.second.size());

    vector<NODE_T> mvcTotal;
    for(NODE_T v : mvc.first) {
        mvcTotal.push_back(v);
    }
    for(NODE_T v : mvc.second) {
        mvcTotal.push_back(v);
    }
    std::sort(mvcTotal.begin(), mvcTotal.end());

    EXPECT_TRUE(mvcTotal[0] == 3 || mvcTotal[0] == 4);

    EXPECT_EQ(mvcTotal[1], 5);

    if(mvc.second.size() == 1) {
        EXPECT_TRUE(mvc.second[0] == 5);
    }
    else {
        EXPECT_TRUE(mvc.second[0] == 5 || mvc.second[1] == 5);
    }


    edges.clear();
    edges.push_back(make_pair(3u, 7u));
    edges.push_back(make_pair(4u, 7u));
    edges.push_back(make_pair(5u, 6u));
    edges.push_back(make_pair(5u, 7u));

    EXPECT_EQ(bipartiteMVC.getMinimumVertexCoverSize(edges), 2);

    mvc = bipartiteMVC.getMinimumVertexCover(edges);
    EXPECT_EQ(bipartiteMVC.getMinimumVertexCoverSize(edges), mvc.first.size() + mvc.second.size());
}
