/*******************************************************************************
 * tests/arraySetTests.cpp
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <vector>
#include <algorithm>

#include <gtest/gtest.h>

#include "arraySet.h"

TEST(ArraySetTest, SimpleTest) {
    ArraySet<unsigned> as(10);
    EXPECT_EQ(as.capacity(), 10);
    EXPECT_EQ(as.size(), 0);

    as.resize(20);
    EXPECT_EQ(as.capacity(), 20);
    EXPECT_EQ(as.size(), 0);

    EXPECT_FALSE(as.contains(5));
    as.insert(5);
    EXPECT_TRUE(as.contains(5));

    EXPECT_FALSE(as.contains(19));

    EXPECT_FALSE(as.contains(1));
    as.insert(1);
    EXPECT_TRUE(as.contains(5));
    EXPECT_TRUE(as.contains(1));

    as.remove(5);
    EXPECT_FALSE(as.contains(5));
    EXPECT_TRUE(as.contains(1));

    as.insert(3);
    EXPECT_FALSE(as.contains(5));
    EXPECT_TRUE(as.contains(1));
    EXPECT_TRUE(as.contains(3));

    std::vector<unsigned> elements;
    for(int element : as) {
        elements.push_back(element);
    }

    std::sort(elements.begin(), elements.end());

    ASSERT_EQ(elements.size(), 2);
    EXPECT_EQ(elements[0], 1);
    EXPECT_EQ(elements[1], 3);
}
