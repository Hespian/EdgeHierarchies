/*******************************************************************************
 * lib/edgeRanking/shortcutsHopsRoundsEdgeRanker.h
 *
 * Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
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

class ShortcutsHopsRoundsEdgeRanker {

public:
    ShortcutsHopsRoundsEdgeRanker(EdgeHierarchyGraph &g) : g(g), query(g), mvc(g.getNumberOfNodes()), edgeScore(g.getNumberOfEdges()), edgesInGraph(g.getNumberOfEdges()), numHops(g.getNumberOfEdges(), 1), lastEdgeRemovedId(EDGEID_EMPTY_KEY) {
        std::cout << "Shortcut hops rounds edge ranker" << std::endl;
        g.forAllNodes( [&] (NODE_T u) {
                g.forAllNeighborsOut(u, [&] (NODE_T v, EDGEWEIGHT_T weight) {
                        addEdgeInitial(u, v);
                    });
            });
    }

    void addEdgeInitial(NODE_T u, NODE_T v) {
        EDGEID_T edgeId = edgeIdCreator.getEdgeId(u, v);
        if(edgesInGraph.capacity() <= edgeId) {
            edgesInGraph.resize(edgesInGraph.capacity() * 2);
            edgeScore.resize(edgeScore.size() * 2);
            numHops.resize(edgeScore.size() * 2, 1);
            assert(edgeScore.size() == edgesInGraph.capacity());
        }
        edgesInGraph.insert(edgeId);
    }

    void addEdge(NODE_T u, NODE_T v) {
        addEdgeInitial(u, v);
        updateHops(u, v);
    }

    void updateEdge(NODE_T u, NODE_T v) {
        updateHops(u, v);
    }

    void updateHops(NODE_T u, NODE_T v) {
        auto edgeRemoved = edgeIdCreator.getEdgeFromId(lastEdgeRemovedId);
        EDGEID_T edgeIdUpdated =  edgeIdCreator.getEdgeId(u, v);
        if(u == edgeRemoved.first) {
            assert(g.hasEdge(edgeRemoved.second, v));
            EDGEID_T otherEdgeId = edgeIdCreator.getEdgeId(edgeRemoved.second, v);
            numHops[edgeIdUpdated] = numHops[lastEdgeRemovedId] + numHops[otherEdgeId];
        } else if(v == edgeRemoved.second) {
            assert(g.hasEdge(u, edgeRemoved.first));
            EDGEID_T otherEdgeId = edgeIdCreator.getEdgeId(u, edgeRemoved.first);
            numHops[edgeIdUpdated] = numHops[lastEdgeRemovedId] + numHops[otherEdgeId];
        } else {
            assert(false);
        }
    }

    pair<NODE_T, NODE_T> getNextEdge() {
        if(currentRoundEdges.size() == 0) {
            getNextRoundEdges();
        }

        EDGEID_T nextEdgeId = currentRoundEdges.back();
        currentRoundEdges.pop_back();
        edgesInGraph.remove(nextEdgeId);
        auto edge = edgeIdCreator.getEdgeFromId(nextEdgeId);
        lastEdgeRemovedId = nextEdgeId;
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
            auto shortestPathsLost = getShortestPathsLost<true>(edge.first, edge.second, g.getEdgeWeight(u, v), g, query);
            g.setEdgeRank(u, v, EDGERANK_INFINIY);
            auto shortcutsToAdd = mvc.getMinimumVertexCover(shortestPathsLost.first);
            edgeScore[edgeId] = shortcutsToAdd.first.size() + shortcutsToAdd.second.size();

            int numHopsUV = numHops[edgeId];
            int numHopsAdded = 0;
            for(const auto &uPrime: shortcutsToAdd.first) {
                auto neighborEdgeId = edgeIdCreator.getEdgeId(uPrime, u);
                numHopsAdded += numHops[neighborEdgeId] + numHopsUV;
            }
            for(const auto &vPrime: shortcutsToAdd.second){
                auto neighborEdgeId = edgeIdCreator.getEdgeId(v, vPrime);
                numHopsAdded += numHops[neighborEdgeId] + numHopsUV;
            }
            edgeScore[edgeId] *= 1000;
            edgeScore[edgeId] += (100 * numHopsAdded) / numHopsUV;
        }

        // std::cout << "Updated " << numUpdates << " out of " << edgesInGraph.size() << " Edges. (" << (100.0 * numUpdates) / edgesInGraph.size() << "%)" << std::endl;

        for(EDGEID_T edgeId : edgesInGraph) {
            pair<NODE_T, NODE_T> edge = edgeIdCreator.getEdgeFromId(edgeId);
            NODE_T u = edge.first;
            NODE_T v = edge.second;
            int edgeScoreCurrentEdge = edgeScore[edgeId];
            bool isMinimum = true;
            g.forAllNeighborsOutWithHighRank(v, EDGERANK_INFINIY,
                                             [&](NODE_T neighbor, EDGERANK_T level, EDGEWEIGHT_T weight) {
                                                 EDGEID_T incidentEdgeId = edgeIdCreator.getEdgeId(v, neighbor);
                                                 assert(edgesInGraph.contains(incidentEdgeId));
                                                 if (edgeScore[incidentEdgeId] < edgeScoreCurrentEdge) {
                                                     isMinimum = false;
                                                 }
                                             });

            if(isMinimum) {
                g.forAllNeighborsInWithHighRank(u, EDGERANK_INFINIY,
                                                [&](NODE_T neighbor, EDGERANK_T level, EDGEWEIGHT_T weight) {
                                                    EDGEID_T incidentEdgeId = edgeIdCreator.getEdgeId(neighbor, u);
                                                    assert(edgesInGraph.contains(incidentEdgeId));
                                                    if (edgeScore[incidentEdgeId] <
                                                        edgeScoreCurrentEdge) {
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
    vector<int> edgeScore;
    ArraySet<EDGEID_T> edgesInGraph;
    vector<EDGEID_T> currentRoundEdges;
    vector<int> numHops;
    EDGEID_T lastEdgeRemovedId;
};
