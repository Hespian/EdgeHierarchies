/*******************************************************************************
 * lib/shortcutHelper.h
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include <vector>
#include <utility>
#include "assert.h"
#include <tuple>

// first: shortest paths lost; second: edges to decrease
template<bool returnEdgesToDecrease>
pair<vector<pair<NODE_T, NODE_T>>, vector<tuple<NODE_T, NODE_T, EDGEWEIGHT_T>>> getShortestPathsLost(NODE_T u, NODE_T v, EDGEWEIGHT_T uVWeight, EdgeHierarchyGraph &g, EdgeHierarchyQuery &query) {
    pair<vector<pair<NODE_T, NODE_T>>, vector<tuple<NODE_T, NODE_T, EDGEWEIGHT_T>>> result;
    g.forAllNeighborsInWithHighLevel(u, EDGELEVEL_INFINIY,
                                     [&] (NODE_T uPrime, EDGELEVEL_T uPrimeLevel, EDGEWEIGHT_T uPrimeWeight) {
                                         assert(uPrimeLevel == EDGELEVEL_INFINIY);
                                         EDGEWEIGHT_T uPrimeVWeight = uVWeight + uPrimeWeight;
                                         g.forAllNeighborsOutWithHighLevel(v, EDGELEVEL_INFINIY,
                                                                           [&] (NODE_T vPrime, EDGELEVEL_T vPrimeLevel, EDGEWEIGHT_T vPrimeWeight) {
                                                                               assert(vPrimeLevel == EDGELEVEL_INFINIY);
                                                                               EDGEWEIGHT_T uPrimeVPrimeWeight = uPrimeVWeight + vPrimeWeight;
                                                                               EDGEWEIGHT_T distanceInQueryGraph = query.getDistance(uPrime, vPrime, uPrimeVPrimeWeight);

                                                                               if(distanceInQueryGraph > uPrimeVPrimeWeight) {
                                                                                   if(g.hasEdge(uPrime, v)) {
                                                                                       assert(g.getEdgeWeight(uPrime, v) > uPrimeVWeight);
                                                                                       if(returnEdgesToDecrease) {
                                                                                           result.second.push_back(make_tuple(uPrime, v, uPrimeVWeight));
                                                                                       }
                                                                                   }
                                                                                   else if (g.hasEdge(u, vPrime)) {
                                                                                       assert(g.getEdgeWeight(u, vPrime) > uVWeight + vPrimeWeight);
                                                                                       if(returnEdgesToDecrease) {
                                                                                           EDGEWEIGHT_T uVPrimeWeight = uVWeight + vPrimeWeight;
                                                                                           result.second.push_back(make_tuple(u, vPrime, uVPrimeWeight));
                                                                                       }
                                                                                   }
                                                                                   else {
                                                                                       result.first.push_back(make_pair(uPrime, vPrime));
                                                                                   }
                                                                               }
                                                                           });
                                     });
    return result;
}
