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

template<bool decreaseEdgeWeights>
vector<pair<NODE_T, NODE_T>> getShortestPathsLost(NODE_T u, NODE_T v, EDGEWEIGHT_T uVWeight, EdgeHierarchyGraph &g, EdgeHierarchyQuery &query) {
    vector<pair<NODE_T, NODE_T>> result;
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
                                                                                   if(g.hasEdge(uPrime, v)) {
                                                                                       if(decreaseEdgeWeights) {
                                                                                           g.decreaseEdgeWeight(uPrime, v, uPrimeVWeight);
                                                                                       }
                                                                                   }
                                                                                   else if (g.hasEdge(u, vPrime)) {
                                                                                       if(decreaseEdgeWeights) {
                                                                                           EDGEWEIGHT_T uVPrimeWeight = uVWeight + vPrimeWeight;
                                                                                           g.decreaseEdgeWeight(u, vPrime, uVPrimeWeight);
                                                                                       }
                                                                                   }
                                                                                   else {
                                                                                       result.push_back(make_pair(uPrime, vPrime));
                                                                                   }
                                                                               }
                                                                           });
                                     });
    return result;
}
