/*******************************************************************************
 * app/definitions.h
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once
#include <cstdint>
#include <limits>

#define NODE_T uint32_t
#define NODE_INVALID std::numeric_limits<NODE_T>::max()
#define EDGEWEIGHT_T uint32_t
#define EDGEWEIGHT_INFINITY numeric_limits<EDGEWEIGHT_T>::max()
#define EDGERANK_T uint32_t
#define EDGECOUNT_T EDGERANK_T
#define EDGERANK_INFINIY std::numeric_limits<EDGERANK_T>::max()
#define EDGEID_T uint64_t
#define EDGEID_EMPTY_KEY std::numeric_limits<EDGEID_T>::max()
