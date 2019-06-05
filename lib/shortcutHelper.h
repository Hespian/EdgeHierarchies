/*******************************************************************************
 * lib/shortcutHelper.h
 *
 * Copyright (C) 2018-2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include <vector>
#include <utility>
#include <cassert>
#include <tuple>

#include <routingkit/contraction_hierarchy.h>

int numEquals = 0;

RoutingKit::ContractionHierarchyQuery shortcutHelperChQuery;

// first: shortest paths lost; second: edges to decrease
template<bool returnEdgesToDecrease>
pair<vector<pair<NODE_T, NODE_T>>, vector<tuple<NODE_T, NODE_T, EDGEWEIGHT_T>>> getShortestPathsLost(NODE_T u, NODE_T v, EDGEWEIGHT_T uVWeight, EdgeHierarchyGraph &g, EdgeHierarchyQuery &query) {
    pair<vector<pair<NODE_T, NODE_T>>, vector<tuple<NODE_T, NODE_T, EDGEWEIGHT_T>>> result;
    g.forAllNeighborsInWithHighRank(u, EDGERANK_INFINIY,
                                    [&](NODE_T uPrime, EDGERANK_T uPrimeLevel, EDGEWEIGHT_T uPrimeWeight) {
                                        assert(uPrimeLevel == EDGERANK_INFINIY);
                                        EDGEWEIGHT_T uPrimeVWeight = uVWeight + uPrimeWeight;
                                        g.forAllNeighborsOutWithHighRank(v, EDGERANK_INFINIY,
                                                                         [&](NODE_T vPrime, EDGERANK_T vPrimeLevel,
                                                                             EDGEWEIGHT_T vPrimeWeight) {
                                                                             assert(vPrimeLevel == EDGERANK_INFINIY);
                                                                             EDGEWEIGHT_T uPrimeVPrimeWeight =
                                                                                     uPrimeVWeight + vPrimeWeight;
                                                                             shortcutHelperChQuery.reset().add_source(uPrime).add_target(vPrime).run<true, false>();
                                                                             auto distanceInQueryGraph = shortcutHelperChQuery.get_distance();
                                                                             // EDGEWEIGHT_T distanceInQueryGraph = query.getDistance(
                                                                             //         uPrime, vPrime,
                                                                             //         uPrimeVPrimeWeight);

                                                                             if(g.getEdgeRank(u, v) != EDGERANK_INFINIY - 1 && distanceInQueryGraph == uPrimeVPrimeWeight) {
                                                                                 ++numEquals;
                                                                             }
                                                                             if (distanceInQueryGraph >=
                                                                                 uPrimeVPrimeWeight) {
                                                                                 if (g.hasEdge(uPrime, v)) {
                                                                                     if (returnEdgesToDecrease) {
                                                                                         result.second.emplace_back(
                                                                                                 uPrime, v,
                                                                                                 uPrimeVWeight);
                                                                                     }
                                                                                 } else if (g.hasEdge(u, vPrime)) {
                                                                                     if (returnEdgesToDecrease) {
                                                                                         EDGEWEIGHT_T uVPrimeWeight =
                                                                                                 uVWeight +
                                                                                                 vPrimeWeight;
                                                                                         result.second.emplace_back(u,
                                                                                                                    vPrime,
                                                                                                                    uVPrimeWeight);
                                                                                     }
                                                                                 } else {
                                                                                     result.first.emplace_back(uPrime,
                                                                                                               vPrime);
                                                                                 }
                                                                             }
                                                                             // if (distanceInQueryGraph ==
                                                                             //     uPrimeVPrimeWeight) {
                                                                             //     // if (g.hasEdge(uPrime, v) && g.getEdgeRank(uPrime, v) != EDGERANK_INFINIY && g.getEdgeWeight(uPrime,v) == uPrimeWeight + uVWeight) {
                                                                             //     if (g.hasEdge(uPrime, v)) {
                                                                             //         if (returnEdgesToDecrease) {
                                                                             //             result.second.emplace_back(
                                                                             //                     uPrime, v,
                                                                             //                     uPrimeVWeight);
                                                                             //         }
                                                                             //     // } else if (g.hasEdge(u, vPrime) && g.getEdgeRank(u, vPrime) != EDGERANK_INFINIY && g.getEdgeWeight(u,vPrime) == uVWeight + vPrimeWeight) {
                                                                             //     } else if (g.hasEdge(u, vPrime)) {
                                                                             //         if (returnEdgesToDecrease) {
                                                                             //             EDGEWEIGHT_T uVPrimeWeight =
                                                                             //                     uVWeight +
                                                                             //                     vPrimeWeight;
                                                                             //             result.second.emplace_back(u,
                                                                             //                                        vPrime,
                                                                             //                                        uVPrimeWeight);
                                                                             //         }
                                                                             //     // } else if(g.hasEdge(uPrime, vPrime) && g.getEdgeRank(uPrime, vPrime) != EDGERANK_INFINIY && g.getEdgeWeight(uPrime, vPrime) == uPrimeVPrimeWeight) {
                                                                             //     } else if(g.hasEdge(uPrime, vPrime) && g.getEdgeRank(uPrime, vPrime) != EDGERANK_INFINIY) {
                                                                             //         result.first.emplace_back(uPrime, vPrime);
                                                                             //     }
                                                                             // }
                                                                         });
                                    });
    return result;
}
