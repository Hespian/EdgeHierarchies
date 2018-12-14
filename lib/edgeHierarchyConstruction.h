/*******************************************************************************
 * lib/edgeHierarchyConstruction.h
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include "definitions.h"
#include "edgeHierarchyGraph.h"
#include "edgeHierarchyQuery.h"

using namespace std;

template <typename EdgeRanker>
class EdgeHierarchyConstruction {
public:
    EdgeHierarchyConstruction(EdgeHierarchyGraph &g, EdgeHierarchyQuery &query) : g(g), query(query), edgeRanker(g) {}

    void setEdgeLevel(NODE_T u, NODE_T v, EDGELEVEL_T level) {
        assert(g.getEdgeLevel(u,v) == EDGELEVEL_INFINIY);
        g.setEdgeLevel(u, v, level);
        EDGEWEIGHT_T uVWeight = g.getEdgeWeight(u, v);
        g.forAllNeighborsInWithHighLevel(u, EDGELEVEL_INFINIY,
                                         [&] (NODE_T uPrime, EDGELEVEL_T uPrimeLevel, EDGEWEIGHT_T uPrimeWeight) {
                                             assert(uPrimeLevel == EDGELEVEL_INFINIY);
                                             EDGEWEIGHT_T uPrimeVWeight = uVWeight + uPrimeWeight;
                                             g.forAllNeighborsOutWithHighLevel(v, EDGELEVEL_INFINIY,
                                                                              [&] (NODE_T vPrime, EDGELEVEL_T vPrimeLevel, EDGEWEIGHT_T vPrimeWeight) {
                                                                                   assert(vPrimeLevel == EDGELEVEL_INFINIY);
                                                                                   EDGEWEIGHT_T uPrimeVPrimeWeight = uPrimeVWeight + vPrimeWeight;
                                                                                   EDGEWEIGHT_T distanceInQueryGraph = query.getDistance(uPrime, vPrime);

                                                                                   if(distanceInQueryGraph > uPrimeVPrimeWeight) {
                                                                                       // TODO Pick the better edge here
                                                                                       g.addEdge(uPrime, v, uPrimeVWeight);
                                                                                       edgeRanker.addEdge(uPrime, v);
                                                                                   }
                                                                               });
                                         });
    }

    void run() {
        EDGECOUNT_T currentLevel = 1;
        while(edgeRanker.hasNextEdge()) {
            auto nextEdge = edgeRanker.getNextEdge();
            setEdgeLevel(nextEdge.first, nextEdge.second, currentLevel++);
        }
    }

protected:
    EdgeHierarchyGraph &g;
    EdgeHierarchyQuery &query;
    EdgeRanker edgeRanker;
};

