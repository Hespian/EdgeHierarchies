/*******************************************************************************
 * lib/edgeIdCreator.h
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once
#include <cstdint>
#include <utility>
#include <vector>

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
            edges.push_back(std::make_pair(u, v));
            assert(edges.size() == numEdges);
            assert(edges[edgeIdMapper[singeValueEdge]] == std::make_pair(u, v));
        }
        return edgeIdMapper[singeValueEdge];
    }

    std::pair<NODE_T, NODE_T> getEdgeFromId(EDGEID_T id) {
        assert(id < numEdges);
        return edges[id];
    }

protected:
    EDGEID_T getSingeValueEdge(NODE_T u, NODE_T v) {
        // Cantor pairing function
        return (1.0/2) * (u + v) * (u + v + 1) + v;
    }

    EDGEID_T numEdges;
    google::sparse_hash_map<EDGEID_T, EDGEID_T> edgeIdMapper;
    std::vector<std::pair<NODE_T, NODE_T>> edges;
};
