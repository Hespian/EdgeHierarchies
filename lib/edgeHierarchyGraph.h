/*******************************************************************************
 * lib/edgeHierarchyGraph.h
 *
 * Copyright (C) 2018-2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include <vector>
#include <cassert>
#include <numeric>


#include "definitions.h"

using namespace std;

class EdgeHierarchyGraph {
public:
    EdgeHierarchyGraph(NODE_T n) : n(n), m(0), neighborsOut(n), edgeWeightsOut(n), edgeLevelsOut(n), neighborsIn(n), edgeWeightsIn(n), edgeLevelsIn(n), edgesSorted(false) {

    }

    NODE_T getNumberOfNodes() {
        return n;
    }

    EDGECOUNT_T getNumberOfEdges() {
        return m;
    }

    NODE_T getInDegree(NODE_T v) {
        return neighborsIn[v].size();
    }

    NODE_T getOutDegree(NODE_T v) {
        return neighborsOut[v].size();
    }

    void addEdge(NODE_T u, NODE_T v, EDGEWEIGHT_T weight) {
        assert(!hasEdge(u, v));
        ++m;
        neighborsOut[u].push_back(v);
        edgeWeightsOut[u].push_back(weight);
        edgeLevelsOut[u].push_back(EDGELEVEL_INFINIY);
        neighborsIn[v].push_back(u);
        edgeWeightsIn[v].push_back(weight);
        edgeLevelsIn[v].push_back(EDGELEVEL_INFINIY);
    }

    void decreaseEdgeWeight(NODE_T u, NODE_T v, EDGEWEIGHT_T weight) {
        assert(hasEdge(u, v));
        if(getEdgeWeight(u,v) < weight){
            return;
        }
        assert(getEdgeWeight(u, v) >= weight);
        for(size_t i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i] == v) {
                edgeWeightsOut[u][i] = weight;
                break;
            }
        }
        for(size_t i = 0; i < neighborsIn[v].size(); ++i) {
            if(neighborsIn[v][i] == u) {
                edgeWeightsIn[v][i] = weight;
                return;
            }
        }
        assert(false);
    }

    void setEdgeLevel(NODE_T u, NODE_T v, EDGELEVEL_T level) {
        for(size_t i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i] == v) {
                edgeLevelsOut[u][i] = level;
                break;
            }
        }

        for(size_t i = 0; i < neighborsIn[v].size(); ++i) {
            if(neighborsIn[v][i] == u) {
                edgeLevelsIn[v][i] = level;
                break;
            }
        }
    }

    EDGELEVEL_T getEdgeLevel(NODE_T u, NODE_T v) {
        for(size_t i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i] == v) {
                return edgeLevelsOut[u][i];
            }
        }
        assert(false);
    }

    bool hasEdge(NODE_T u, NODE_T v) {
        for(auto neighbor : neighborsOut[u]) {
            if(neighbor == v) {
                return true;
            }
        }
        return false;
    }

    EDGEWEIGHT_T getEdgeWeight(NODE_T u, NODE_T v) {
        for(size_t i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i] == v) {
                return edgeWeightsOut[u][i];
            }
        }
        assert(false);
    }

    template<typename F>
    void forAllNeighborsIn(NODE_T v, F &&callback) {
        for(size_t i = 0; i < neighborsIn[v].size(); ++i) {
            callback(neighborsIn[v][i], edgeWeightsIn[v][i]);
        }
    }

    template<typename F>
    void forAllNeighborsOut(NODE_T v, F &&callback) {
        for(size_t i = 0; i < neighborsOut[v].size(); ++i) {
            callback(neighborsOut[v][i], edgeWeightsOut[v][i]);
        }
    }


    template<typename F>
    void forAllNeighborsInWithHighLevel(NODE_T v, EDGELEVEL_T levelThreshold, F &&callback) {
        for(size_t i = 0; i < neighborsIn[v].size(); ++i) {
            if(edgeLevelsIn[v][i] >= levelThreshold) {
                callback(neighborsIn[v][i], edgeLevelsIn[v][i], edgeWeightsIn[v][i]);
            } else if(edgesSorted) {
                break;
            }
        }
    }

    template<typename F>
    void forAllNeighborsOutWithHighLevel(NODE_T v, EDGELEVEL_T levelThreshold, F &&callback) {
        for(size_t i = 0; i < neighborsOut[v].size(); ++i) {
            if(edgeLevelsOut[v][i] >= levelThreshold) {
                callback(neighborsOut[v][i], edgeLevelsOut[v][i], edgeWeightsOut[v][i]);
            } else if(edgesSorted) {
                break;
            }
        }
    }

    template<typename F>
    void forAllNodes(F &&callback) {
        for(NODE_T v = 0; v < n; ++v) {
            callback(v);
        }
    }

    void sortEdges() {
        for(NODE_T v = 0; v < n; ++v) {
            vector<NODE_T> order(edgeLevelsOut[v].size());
            iota(begin(order), end(order), 0);

            sort(order.begin(), order.end(), [&] (NODE_T i, NODE_T j) {
                return edgeLevelsOut[v][i] > edgeLevelsOut[v][j];
            });

            applyPermutation(neighborsOut[v], order);
            applyPermutation(edgeWeightsOut[v], order);
            applyPermutation(edgeLevelsOut[v], order);
        }

        for(NODE_T v = 0; v < n; ++v) {
            vector<NODE_T> order(edgeLevelsIn[v].size());
            iota(begin(order), end(order), 0);

            sort(order.begin(), order.end(), [&] (NODE_T i, NODE_T j) {
                return edgeLevelsIn[v][i] > edgeLevelsIn[v][j];
            });

            applyPermutation(neighborsIn[v], order);
            applyPermutation(edgeWeightsIn[v], order);
            applyPermutation(edgeLevelsIn[v], order);
        }

        edgesSorted = true;
    }

/******************************************************************************/

protected:
    NODE_T n;
    EDGECOUNT_T m;
    vector<vector<NODE_T>> neighborsOut;
    vector<vector<EDGEWEIGHT_T>> edgeWeightsOut;
    vector<vector<EDGELEVEL_T>> edgeLevelsOut;
    vector<vector<NODE_T>> neighborsIn;
    vector<vector<EDGEWEIGHT_T>> edgeWeightsIn;
    vector<vector<EDGELEVEL_T>> edgeLevelsIn;
    bool edgesSorted;

    template<typename T>
    void applyPermutation(
            vector<T> &vec,
            vector<NODE_T> &perm)
    {
        vector<T> vecTemp(vec.size());
        for (size_t i = 0; i < vec.size(); i++) {
            vecTemp[i] = vec[perm[i]];
        }
        vec.swap(vecTemp);
    }
};
