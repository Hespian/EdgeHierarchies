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
        std::cout << "Shortcut counting rounds edge ranker" << std::endl;
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
            // needsUpdate.resize(needsUpdate.size() * 2, false);
            assert(numShortcutEdges.size() == edgesInGraph.capacity());
        }
        edgesInGraph.insert(edgeId);

        // g.forAllNeighborsOutWithHighRank(v, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
        //         EDGEID_T neighborEdgeId = edgeIdCreator.getEdgeId(v, x);
        //         needsUpdate[neighborEdgeId] = true;
        //     });
        // g.forAllNeighborsInWithHighRank(u, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
        //         EDGEID_T neighborEdgeId = edgeIdCreator.getEdgeId(x, u);
        //         needsUpdate[neighborEdgeId] = true;
        //     });

        // needsUpdate[edgeId] = true;
    }

    void updateEdge(NODE_T u, NODE_T v) {
        // EDGEID_T edgeId = edgeIdCreator.getEdgeId(u, v);

        // g.forAllNeighborsOutWithHighRank(v, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
        //         EDGEID_T neighborEdgeId = edgeIdCreator.getEdgeId(v, x);
        //         needsUpdate[neighborEdgeId] = true;
        //     });
        // g.forAllNeighborsInWithHighRank(u, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
        //         EDGEID_T neighborEdgeId = edgeIdCreator.getEdgeId(x, u);
        //         needsUpdate[neighborEdgeId] = true;
        //     });

        // needsUpdate[edgeId] = true;
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
        }

        // std::cout << "Updated " << numUpdates << " out of " << edgesInGraph.size() << " Edges. (" << (100.0 * numUpdates) / edgesInGraph.size() << "%)" << std::endl;

        for(EDGEID_T edgeId : edgesInGraph) {
            pair<NODE_T, NODE_T> edge = edgeIdCreator.getEdgeFromId(edgeId);
            NODE_T u = edge.first;
            NODE_T v = edge.second;
            EDGEID_T numShortcutEdgesCurrentEdge = numShortcutEdges[edgeId];
            bool isMinimum = true;
            g.forAllNeighborsOutWithHighRank(v, EDGERANK_INFINIY,
                                             [&](NODE_T neighbor, EDGERANK_T level, EDGEWEIGHT_T weight) {
                                                 EDGEID_T incidentEdgeId = edgeIdCreator.getEdgeId(v, neighbor);
                                                 assert(edgesInGraph.contains(incidentEdgeId));
                                                 if (numShortcutEdges[incidentEdgeId] < numShortcutEdgesCurrentEdge) {
                                                     isMinimum = false;
                                                 }
                                             });

            if(isMinimum) {
                g.forAllNeighborsInWithHighRank(u, EDGERANK_INFINIY,
                                                [&](NODE_T neighbor, EDGERANK_T level, EDGEWEIGHT_T weight) {
                                                    EDGEID_T incidentEdgeId = edgeIdCreator.getEdgeId(neighbor, u);
                                                    assert(edgesInGraph.contains(incidentEdgeId));
                                                    if (numShortcutEdges[incidentEdgeId] <
                                                        numShortcutEdgesCurrentEdge) {
                                                        isMinimum = false;
                                                    }
                                                });
            }

            if(isMinimum) {
                currentRoundEdges.push_back(edgeId);
            }
        }
        std::cout << "Queued " << currentRoundEdges.size() << " out of " << edgesInGraph.size() <<std::endl;
    }

    EdgeHierarchyGraph &g;
    EdgeHierarchyQuery query;
    EdgeIdCreator edgeIdCreator;
    BipartiteMinimumVertexCover mvc;
    vector<EDGEID_T> numShortcutEdges;
    ArraySet<EDGEID_T> edgesInGraph;
    vector<EDGEID_T> currentRoundEdges;
    // vector<bool> needsUpdate;
};
