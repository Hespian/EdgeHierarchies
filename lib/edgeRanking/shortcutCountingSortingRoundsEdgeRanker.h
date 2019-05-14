/*******************************************************************************
 * lib/edgeRanking/shortcutCountingSortingRoundsEdgeRanker.h
 *
 * Copyright (C) 2018-2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include <vector>
#include <utility>
#include "assert.h"

#include "definitions.h"
#include "edgeIdCreator.h"
#include "edgeHierarchyGraph.h"
#include "edgeHierarchyQuery.h"
#include "bipartiteMinimumVertexCover.h"
#include "arraySet.h"
#include "shortcutHelper.h"

using namespace std;

class ShortcutCountingSortingRoundsEdgeRanker {

public:
    ShortcutCountingSortingRoundsEdgeRanker(EdgeHierarchyGraph &g) : g(g), query(g), mvc(g.getNumberOfNodes()), numShortcutEdges(g.getNumberOfEdges()), edgesInGraph(g.getNumberOfEdges()) {
        std::cout << "Shortcut counting sorting rounds edge ranker" << std::endl;
        g.forAllNodes( [&] (NODE_T u) {
                g.forAllNeighborsOut(u, [&] (NODE_T v, EDGEWEIGHT_T weight) {
                        addEdge(u, v);
                    });
            });
    }

    void addEdge(NODE_T u, NODE_T v) {
        EDGEID_T edgeId = edgeIdCreator.getEdgeId(u, v);
        if(edgesInGraph.capacity() <= edgeId) {
            edgesInGraph.resize(edgesInGraph.capacity() * 2);
            numShortcutEdges.resize(numShortcutEdges.size() * 2);
            assert(numShortcutEdges.size() == edgesInGraph.capacity());
        }
        edgesInGraph.insert(edgeId);
    }

    void updateEdge(NODE_T u, NODE_T v) {
    }

    pair<NODE_T, NODE_T> getNextEdge() {
        if(currentRoundEdges.size() == 0) {
            getNextRoundEdges();
        }

        EDGEID_T nextEdgeId = currentRoundEdges.back();
        currentRoundEdges.pop_back();
        edgesInGraph.remove(nextEdgeId);
        auto edge = edgeIdCreator.getEdgeFromId(nextEdgeId);
        updateEdge(edge.first, edge.second);
        return edge;
    }

    bool hasNextEdge() {
        return edgesInGraph.size() > 0;
    }

protected:
    void getNextRoundEdges() {
        for(EDGEID_T edgeId : edgesInGraph) {
            pair<NODE_T, NODE_T> edge = edgeIdCreator.getEdgeFromId(edgeId);
            NODE_T u = edge.first;
            NODE_T v = edge.second;
            assert(g.getEdgeRank(u, v) == EDGERANK_INFINIY);
            g.setEdgeRank(u, v, EDGERANK_INFINIY - 1);
            auto shortestPathsLost = getShortestPathsLost<false>(edge.first, edge.second, g.getEdgeWeight(u, v), g, query);
            g.setEdgeRank(u, v, EDGERANK_INFINIY);
            numShortcutEdges[edgeId] = mvc.getMinimumVertexCoverSize(shortestPathsLost.first);
            currentRoundEdges.push_back(edgeId);
        }

        std::sort(currentRoundEdges.begin(), currentRoundEdges.end(), [&] (EDGEID_T i, EDGEID_T j) {
                return numShortcutEdges[i] < numShortcutEdges[j];
            });

        std::cout << "Queued up " << currentRoundEdges.size() << " edges with " << numShortcutEdges[currentRoundEdges.front()] << " to " << numShortcutEdges[currentRoundEdges.back()] << " shortcuts" << std::endl;

    }

    EdgeHierarchyGraph &g;
    EdgeHierarchyQuery query;
    EdgeIdCreator edgeIdCreator;
    BipartiteMinimumVertexCover mvc;
    vector<EDGEID_T> numShortcutEdges;
    ArraySet<EDGEID_T> edgesInGraph;
    vector<EDGEID_T> currentRoundEdges;
};
