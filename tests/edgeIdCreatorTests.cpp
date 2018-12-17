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
}
