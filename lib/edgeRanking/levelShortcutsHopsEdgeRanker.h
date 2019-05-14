/*******************************************************************************
 * lib/edgeRanking/levelShortcutsHopsEdgeRanker.h
 *
 * Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include "assert.h"
#include <vector>

#include "routingkit/id_queue.h"

#include "definitions.h"
#include "edgeIdCreator.h"
#include "edgeHierarchyGraph.h"
#include "bipartiteMinimumVertexCover.h"
#include "shortcutHelper.h"


class LevelShortcutsHopsEdgeRanker {

public:
    LevelShortcutsHopsEdgeRanker(EdgeHierarchyGraph &g) : g(g), PQ(g.getNumberOfEdges() * 2), numHops(g.getNumberOfEdges() * 2), level(g.getNumberOfEdges() * 2), mvc(g.getNumberOfNodes()), lastEdgeReturned(EDGEID_EMPTY_KEY), query(g), poppedCounter(0) {
        std::cout << "Level shortcuts edge lazy ranker" <<std::endl;
        g.forAllNodes( [&] (NODE_T u) {
                g.forAllNeighborsOut(u, [&] (NODE_T v, EDGEWEIGHT_T weight) {
                        addEdgeFresh(u, v);
                    });
            });
    }


    void addEdge(NODE_T u, NODE_T v) {
        EDGEID_T edgeId = edgeIdCreator.getEdgeId(u, v);
        if(edgeId > PQ.id_count()) {
            increaseCapacity(2 * edgeId);
        }

        // numHops[edgeId] = 1;
        unsigned maxLevel = 1;
        g.forAllNeighborsOutWithHighRank(v, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
                EDGEID_T neighborEdgeId = edgeIdCreator.getEdgeId(v, x);
                if(level[neighborEdgeId] > maxLevel) {
                    maxLevel = level[neighborEdgeId];
                }
                // updateImportance(v, x);
            });
        g.forAllNeighborsInWithHighRank(u, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
                EDGEID_T neighborEdgeId = edgeIdCreator.getEdgeId(x, u);
                if(level[neighborEdgeId] > maxLevel) {
                    maxLevel = level[neighborEdgeId];
                }
                // updateImportance(x, u);
            });
        level[edgeId] = maxLevel + 1;

        updateImportance(u, v);
    }

    void updateEdge(NODE_T u, NODE_T v) {
        // g.forAllNeighborsOutWithHighRank(v, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
        //         updateImportance(v, x);
        //     });
        // g.forAllNeighborsInWithHighRank(u, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
        //         updateImportance(x, u);
        //     });

        // updateImportance(u, v);
    }

    pair<NODE_T, NODE_T> getNextEdge() {
        // updateNeighborEdges();
        auto popped = PQ.pop();
        EDGEID_T edgeId = popped.id;
        EDGEID_T edgeIdOld = EDGEID_EMPTY_KEY;
        pair<NODE_T, NODE_T> edge = edgeIdCreator.getEdgeFromId(edgeId);
        while(edgeId != edgeIdOld){
            edgeIdOld = edgeId;
            edge = edgeIdCreator.getEdgeFromId(edgeId);
            auto newImportance = getEdgeImportance(edge.first, edge.second);

            if(PQ.peek().key < newImportance) {
                // cout << "LARGER!" << endl;
                PQ.push({(unsigned)edgeId, newImportance});
                popped = PQ.pop();
                edgeId = popped.id;
            }
        }
        lastEdgeReturned = edgeId;
        // poppedCounter++;
        // if(poppedCounter % 1000 == 0) {
        //     std::cout << poppedCounter << " out of " << PQ.size() << std::endl;
        // }
        if(g.getEdgeRank(edge.first, edge.second) != EDGERANK_INFINIY) {
            std::cout << "returned invalid edge!" << std::endl;
        }
        return edge;
    }

    bool hasNextEdge() {
        return !PQ.empty();
    }
protected:

    void updateNeighborEdges() {

        if(lastEdgeReturned == EDGEID_EMPTY_KEY) {
            return;
        }
        pair<NODE_T, NODE_T> edge = edgeIdCreator.getEdgeFromId(lastEdgeReturned);
        NODE_T u = edge.first;
        NODE_T v = edge.second;

        unsigned newLevel = level[lastEdgeReturned] + 1;

        g.forAllNeighborsOutWithHighRank(v, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
                EDGEID_T neighborEdgeId = edgeIdCreator.getEdgeId(v, x);
                if(level[neighborEdgeId] < newLevel) {
                    level[neighborEdgeId] = newLevel;
                }
                updateImportance(v, x);
            });
        g.forAllNeighborsInWithHighRank(u, g.getEdgeRank(u, v), [&] (NODE_T x, EDGEWEIGHT_T weight, EDGERANK_T rank) {
                EDGEID_T neighborEdgeId = edgeIdCreator.getEdgeId(x, u);
                if(level[neighborEdgeId] < newLevel) {
                    level[neighborEdgeId] = newLevel;
                }
                updateImportance(x, u);
            });
    }

    unsigned getEdgeImportance(NODE_T u, NODE_T v) {
        if(g.getEdgeRank(u, v) != EDGERANK_INFINIY) {
            std::cout << "Updating importance of removed edge!" <<std::endl;
        }
        EDGEID_T edgeId = edgeIdCreator.getEdgeId(u, v);
        g.setEdgeRank(u, v, EDGERANK_INFINIY - 1);
        auto shortestPathsLost = getShortestPathsLost<false>(u, v, g.getEdgeWeight(u, v), g, query);
        g.setEdgeRank(u, v, EDGERANK_INFINIY);
        auto numShortcutEdges = mvc.getMinimumVertexCoverSize(shortestPathsLost.first);

        return 1 + 1000*level[edgeId] + (1000*numShortcutEdges);
    }

    void updateImportance(NODE_T u, NODE_T v) {
        EDGEID_T edgeId = edgeIdCreator.getEdgeId(u, v);

        auto importance = getEdgeImportance(u, v);


        if(!PQ.contains_id(edgeId)) {
            PQ.push({(unsigned)edgeId, importance});
        } else {
            unsigned currentKey = PQ.get_key(edgeId);
            if(importance < currentKey) {
                PQ.decrease_key({(unsigned)edgeId, importance});
            }
            if(importance > currentKey) {
                PQ.increase_key({(unsigned)edgeId, importance});
            }
        }
    }

    void addEdgeFresh(NODE_T u, NODE_T v) {
        EDGEID_T edgeId = edgeIdCreator.getEdgeId(u, v);
        numHops[edgeId] = 1;
        level[edgeId] = 1;

        updateImportance(u, v);
    }

    void increaseCapacity(size_t size) {
		PQ.id_pos.resize(size, RoutingKit::invalid_id);
        PQ.heap.resize(size);
        numHops.resize(size);
        level.resize(size);
    }

    EdgeHierarchyGraph &g;
    EdgeIdCreator edgeIdCreator;
    RoutingKit::MinIDQueue PQ;
    std::vector<unsigned> numHops;
    std::vector<unsigned> level;
    BipartiteMinimumVertexCover mvc;
    EDGEID_T lastEdgeReturned;
    EdgeHierarchyQuery query;
    unsigned poppedCounter = 0;

};
