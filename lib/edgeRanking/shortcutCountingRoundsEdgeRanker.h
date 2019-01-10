/*******************************************************************************
 * lib/edgeRanking/shortcutCountingRoundsEdgeRanker.h
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

class ShortcutCountingRoundsEdgeRanker {

public:
    ShortcutCountingRoundsEdgeRanker(EdgeHierarchyGraph &g) : g(g), query(g), mvc(g.getNumberOfNodes()), numShortcutEdges(g.getNumberOfEdges()), edgesInGraph(g.getNumberOfEdges()) {
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
        return edgeIdCreator.getEdgeFromId(nextEdgeId);
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
            assert(g.getEdgeLevel(u, v) == EDGELEVEL_INFINIY);
            g.setEdgeLevel(u, v, EDGELEVEL_INFINIY - 1);
            auto shortestPathsLost = getShortestPathsLost<false>(edge.first, edge.second, g.getEdgeWeight(u, v), g, query);
            g.setEdgeLevel(u, v, EDGELEVEL_INFINIY);
            numShortcutEdges[edgeId] = mvc.getMinimumVertexCoverSize(shortestPathsLost.first);
        }

        for(EDGEID_T edgeId : edgesInGraph) {
            pair<NODE_T, NODE_T> edge = edgeIdCreator.getEdgeFromId(edgeId);
            NODE_T u = edge.first;
            NODE_T v = edge.second;
            EDGEID_T numShortcutEdgesCurrentEdge = numShortcutEdges[edgeId];
            bool isMinimum = true;
            g.forAllNeighborsOutWithHighLevel(v, EDGELEVEL_INFINIY, [&] (NODE_T neighbor, EDGELEVEL_T level, EDGEWEIGHT_T weight) {
                    EDGEID_T incidentEdgeId = edgeIdCreator.getEdgeId(v, neighbor);
                    assert(edgesInGraph.contains(incidentEdgeId));
                    if(numShortcutEdges[incidentEdgeId] < numShortcutEdgesCurrentEdge) {
                        isMinimum = false;
                    }
                });

            if(isMinimum) {
                g.forAllNeighborsInWithHighLevel(u, EDGELEVEL_INFINIY, [&] (NODE_T neighbor, EDGELEVEL_T level, EDGEWEIGHT_T weight) {
                        EDGEID_T incidentEdgeId = edgeIdCreator.getEdgeId(neighbor, u);
                        assert(edgesInGraph.contains(incidentEdgeId));
                        if(numShortcutEdges[incidentEdgeId] < numShortcutEdgesCurrentEdge) {
                            isMinimum = false;
                        }
                    });
            }

            if(isMinimum) {
                currentRoundEdges.push_back(edgeId);
            }
        }
    }

    EdgeHierarchyGraph &g;
    EdgeHierarchyQuery query;
    EdgeIdCreator edgeIdCreator;
    BipartiteMinimumVertexCover mvc;
    vector<EDGEID_T> numShortcutEdges;
    ArraySet<EDGEID_T> edgesInGraph;
    vector<EDGEID_T> currentRoundEdges;
};
