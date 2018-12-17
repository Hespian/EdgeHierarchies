/*******************************************************************************
 * lib/edgeIdCreator.h
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once
#include <cstdint>

#include <sparsehash/sparse_hash_map>

#include "definitions.h"

class EdgeIdCreator {
public:
    EdgeIdCreator() : numEdges(0) {
    }

    EDGEID_T getEdgeId(NODE_T u, NODE_T v) {
        EDGEID_T singeValueEdge = getSingeValueEdge(u, v);
        if(edgeIdMapper.find(singeValueEdge) == edgeIdMapper.end()) {
            edgeIdMapper[singeValueEdge] = numEdges++;
        }
        return edgeIdMapper[singeValueEdge];
    }


protected:
    EDGEID_T getSingeValueEdge(NODE_T u, NODE_T v) {
        return (1/2) * (u + v) * (u + v + 1) + v;
    }

    EDGEID_T numEdges;
    google::sparse_hash_map<EDGEID_T, EDGEID_T> edgeIdMapper;
};
