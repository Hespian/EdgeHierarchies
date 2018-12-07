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
#define EDGEWEIGHT_T uint32_t
#define EDGEWEIGHT_INFINITY numeric_limits<EDGEWEIGHT_T>::max()
#define EDGELEVEL_T uint32_t
#define EDGELEVEL_INFINIY std::numeric_limits<EDGELEVEL_T>::max()
