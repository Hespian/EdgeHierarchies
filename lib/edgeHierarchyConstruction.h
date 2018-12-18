/*******************************************************************************
 * lib/edgeHierarchyConstruction.h
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include <vector>
#include <utility>

#include "definitions.h"
#include "edgeHierarchyGraph.h"
#include "edgeHierarchyQuery.h"
#include "bipartiteMinimumVertexCover.h"

using namespace std;

template <typename EdgeRanker>
class EdgeHierarchyConstruction {
public:
    EdgeHierarchyConstruction(EdgeHierarchyGraph &g, EdgeHierarchyQuery &query) : g(g), query(query), edgeRanker(g), bipartiteMVC(g.getNumberOfNodes()) {}

    vector<pair<NODE_T, NODE_T>> getShortestPathsLost(NODE_T u, NODE_T v, EDGEWEIGHT_T uVWeight) {
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
                                                                                       result.push_back(make_pair(uPrime, vPrime));
                                                                                   }
                                                                               });
                                         });
        return result;
    }

    void setEdgeLevel(NODE_T u, NODE_T v, EDGELEVEL_T level) {
        assert(g.getEdgeLevel(u,v) == EDGELEVEL_INFINIY);
        g.setEdgeLevel(u, v, level);
        EDGEWEIGHT_T uVWeight = g.getEdgeWeight(u, v);
        vector<pair<NODE_T, NODE_T>> shortestPathsLost = getShortestPathsLost(u, v, uVWeight);
        auto shortcutVertices = bipartiteMVC.getMinimumVertexCover(shortestPathsLost);

        for(auto uPrime : shortcutVertices.first) {
            EDGEWEIGHT_T uPrimeVWeight = g.getEdgeWeight(uPrime, u) + uVWeight;
            if(!g.hasEdge(uPrime, v)) {
                g.addEdge(uPrime, v, uPrimeVWeight);
                edgeRanker.addEdge(uPrime, v);
            }
            else {
                g.decreaseEdgeWeight(uPrime, v, uPrimeVWeight);
                edgeRanker.updateEdge(uPrime, v);
            }
        }
        for(auto vPrime : shortcutVertices.second) {
            EDGEWEIGHT_T uVPrimeWeight = uVWeight + g.getEdgeWeight(v, vPrime);
            if(!g.hasEdge(u, vPrime)) {
                g.addEdge(u, vPrime, uVPrimeWeight);
                edgeRanker.addEdge(u, vPrime);
            }
            else {
                g.decreaseEdgeWeight(u, vPrime, uVPrimeWeight);
                edgeRanker.updateEdge(u, vPrime);
            }
        }
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
    BipartiteMinimumVertexCover bipartiteMVC;
};

