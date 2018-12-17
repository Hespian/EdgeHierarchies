/*******************************************************************************
 * tests/edgeIdCreatorTests.cpp
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <queue>
#include <utility>

#include <gtest/gtest.h>

#include "edgeIdCreator.h"

TEST(EdgeIdCreatorTest, SimpleTest) {
    EdgeIdCreator edgeIdCreator;
    EXPECT_EQ(edgeIdCreator.getEdgeId(2, 3), 0);
    EXPECT_EQ(edgeIdCreator.getEdgeId(2, 3), 0);
    EXPECT_EQ(edgeIdCreator.getEdgeId(3, 2), 1);
    EXPECT_EQ(edgeIdCreator.getEdgeId(2, 3), 0);
    EXPECT_EQ(edgeIdCreator.getEdgeId(3, 2), 1);

    EXPECT_EQ(edgeIdCreator.getEdgeFromId(1), std::make_pair(3u, 2u));
    EXPECT_EQ(edgeIdCreator.getEdgeFromId(0), std::make_pair(2u, 3u));
}
