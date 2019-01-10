/*******************************************************************************
 * lib/bipartiteMinimumVertexCover.h
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include <vector>
#include <utility>
#include <algorithm>
#include "assert.h"

#include "routingkit/timestamp_flag.h"

#include "definitions.h"

using namespace std;

class BipartiteMinimumVertexCover {
public:
    BipartiteMinimumVertexCover(NODE_T maxNumberOfNodes) :
        nLhs(0),
        nRhs(0),
        neighborsLhs(maxNumberOfNodes),
        neighborsRhs(maxNumberOfNodes),
        nodesLhs(maxNumberOfNodes, NODE_INVALID),
        nodesRhs(maxNumberOfNodes, NODE_INVALID),
        nodesInverseLhs(maxNumberOfNodes, NODE_INVALID),
        nodesInverseRhs(maxNumberOfNodes, NODE_INVALID),
        matchingPartnerLhs(maxNumberOfNodes, NODE_INVALID),
        matchingPartnerRhs(maxNumberOfNodes, NODE_INVALID),
        visited(maxNumberOfNodes),
        markedLhs(maxNumberOfNodes, false),
        markedRhs(maxNumberOfNodes, false) {
    }

    pair<vector<NODE_T>, vector<NODE_T>> getMinimumVertexCover(vector<pair<NODE_T, NODE_T>> &edges) {
        buildLocalGraph(edges);

        findMaximumMatching();

        markVertices();

        pair<vector<NODE_T>, vector<NODE_T>> result;
        for(NODE_T v = 0; v < nLhs; ++v) {
            if(!markedLhs[v]) {
                result.first.push_back(nodesInverseLhs[v]);
            }
        }
        for(NODE_T v = 0; v < nRhs; ++v) {
            if(markedRhs[v]) {
                result.second.push_back(nodesInverseRhs[v]);
            }
        }

        assert(NODE_T(count_if(matchingPartnerLhs.begin(), matchingPartnerLhs.begin() + nLhs, [] (NODE_T partner) { return partner != NODE_INVALID; })) == result.first.size() + result.second.size());

        cleanUp();

        return result;
    }


    NODE_T getMinimumVertexCoverSize(vector<pair<NODE_T, NODE_T>> &edges) {
        buildLocalGraph(edges);

        findMaximumMatching();

        NODE_T size = count_if(matchingPartnerLhs.begin(), matchingPartnerLhs.begin() + nLhs, [] (NODE_T partner) { return partner != NODE_INVALID; });

        cleanUp();
        return size;
    }


protected:
    void markVertices() {
        visited.reset_all();
        for(NODE_T u = 0; u < nLhs; ++u) {
            if(matchingPartnerLhs[u] == NODE_INVALID) {
                markedLhs[u] = true;
                markVerticesStep(u);
            }
        }
    }

    void markVerticesStep(NODE_T u) {
        markedLhs[u] = true;
        for(NODE_T v : neighborsLhs[u]) {
            if(markedRhs[v]) {
                continue;
            }
            markedRhs[v] = true;
            // This would be an augmenting path
            assert(matchingPartnerRhs[v] != NODE_INVALID);
            markVerticesStep(matchingPartnerRhs[v]);
        }
    }

    bool augmentingPathStep(NODE_T u) {
        bool foundPath = false;
        for(NODE_T v : neighborsLhs[u]) {
            if(visited.is_set(v)) {
                continue;
            }
            visited.set(v);
            if(matchingPartnerRhs[v] == NODE_INVALID) {
                foundPath = true;
            }
            else {
                foundPath = augmentingPathStep(matchingPartnerRhs[v]);
            }
            if(foundPath) {
                matchingPartnerRhs[v] = u;
                matchingPartnerLhs[u] = v;
                break;
            }
        }
        return foundPath;
    }

    void findMaximumMatching() {
        bool foundPath = false;
        do {
            visited.reset_all();
            foundPath = false;
            for(NODE_T u = 0; u < nLhs; ++u) {
                if(matchingPartnerLhs[u] == NODE_INVALID) {
                    foundPath = augmentingPathStep(u);
                    if(foundPath) {
                        break;
                    }
                }
            }
        } while (foundPath);
    }

    void buildLocalGraph(vector<pair<NODE_T, NODE_T>> &edges) {
        for(auto edge : edges) {
            NODE_T uGlobal = edge.first;
            NODE_T vGlobal = edge.second;

            NODE_T uLocal = getLocalNodeId<true>(uGlobal);
            NODE_T vLocal = getLocalNodeId<false>(vGlobal);

            neighborsLhs[uLocal].push_back(vLocal);
            neighborsRhs[vLocal].push_back(uLocal);
        }
    }

    void cleanUp() {
        for(NODE_T vLocal = 0; vLocal < nLhs; ++vLocal) {
            NODE_T vGlobal = nodesInverseLhs[vLocal];
            neighborsLhs[vLocal].clear();
            nodesLhs[vGlobal] = NODE_INVALID;
            nodesInverseLhs[vLocal] = NODE_INVALID;
            matchingPartnerLhs[vLocal] = NODE_INVALID;
            markedLhs[vLocal] = false;
        }

        for(NODE_T vLocal = 0; vLocal < nRhs; ++vLocal) {
            NODE_T vGlobal = nodesInverseRhs[vLocal];
            neighborsRhs[vLocal].clear();
            nodesRhs[vGlobal] = NODE_INVALID;
            nodesInverseRhs[vLocal] = NODE_INVALID;
            matchingPartnerRhs[vLocal] = NODE_INVALID;
            markedRhs[vLocal] = false;
        }

        nLhs = 0;
        nRhs = 0;
    }

    template<bool lhs>
    NODE_T getLocalNodeId(NODE_T globalNodeId) {
        NODE_T &n = lhs ? nLhs : nRhs;
        vector<NODE_T> &nodes = lhs ? nodesLhs : nodesRhs;
        vector<NODE_T> &nodesOther = lhs ? nodesRhs : nodesLhs;
        vector<NODE_T> &nodesInverse = lhs ? nodesInverseLhs : nodesInverseRhs;

        if(nodes[globalNodeId] == NODE_INVALID) {
            assert(nodesOther[globalNodeId] == NODE_INVALID);
            nodes[globalNodeId] = n;
            nodesInverse[n] = globalNodeId;
            ++n;
        }
        return nodes[globalNodeId];
    }

    NODE_T nLhs;
    NODE_T nRhs;
    vector<vector<NODE_T>> neighborsLhs;
    vector<vector<NODE_T>> neighborsRhs;
    vector<NODE_T> nodesLhs;
    vector<NODE_T> nodesRhs;
    vector<NODE_T> nodesInverseLhs;
    vector<NODE_T> nodesInverseRhs;
    vector<NODE_T> matchingPartnerLhs;
    vector<NODE_T> matchingPartnerRhs;
    vector<NODE_T> augmentingPath;
    RoutingKit::TimestampFlags visited;
    vector<NODE_T> markedLhs;
    vector<NODE_T> markedRhs;
};
