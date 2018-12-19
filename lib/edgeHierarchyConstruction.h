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
#include "shortcutHelper.h"

using namespace std;

template <class EdgeRanker>
class EdgeHierarchyConstruction {
public:
    EdgeHierarchyConstruction(EdgeHierarchyGraph &g, EdgeHierarchyQuery &query) : g(g), query(query), edgeRanker(g), bipartiteMVC(g.getNumberOfNodes()) {}

    void setEdgeLevel(NODE_T u, NODE_T v, EDGELEVEL_T level) {
        assert(g.getEdgeLevel(u,v) == EDGELEVEL_INFINIY);
        g.setEdgeLevel(u, v, level);
        EDGEWEIGHT_T uVWeight = g.getEdgeWeight(u, v);
        pair<vector<pair<NODE_T, NODE_T>>, vector<tuple<NODE_T, NODE_T, EDGEWEIGHT_T>>> shortestPathsLost = getShortestPathsLost<true>(u, v, uVWeight, g, query);

        for(auto edgeToDecrease : shortestPathsLost.second) {
            g.decreaseEdgeWeight(get<0>(edgeToDecrease), get<1>(edgeToDecrease), get<2>(edgeToDecrease));
            edgeRanker.updateEdge(get<0>(edgeToDecrease), get<1>(edgeToDecrease));
        }
        auto shortcutVertices = bipartiteMVC.getMinimumVertexCover(shortestPathsLost.first);

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

