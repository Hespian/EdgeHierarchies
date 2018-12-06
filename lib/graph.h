/*******************************************************************************
 * lib/graph.h
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include <vector>

#include "definitions.h"

using namespace std;

class Graph {
public:
    Graph(int n) : n(n), neighborsIn(n), neighborsOut(n), edgeLevelsIn(n), edgeLevelsOut(n) {

    }

    NODE_T getNumberOfNodes() {
        return n;
    }

    NODE_T getInDegree(NODE_T v) {
        return neighborsIn[v].size();
    }

    NODE_T getOutDegree(NODE_T v) {
        return neighborsOut[v].size();
    }

    void addEdge(NODE_T u, NODE_T v) {
        neighborsOut[u].push_back(v);
        edgeLevelsOut[u].push_back(EDGELEVEL_INFINIY);
        neighborsIn[v].push_back(u);
        edgeLevelsIn[v].push_back(EDGELEVEL_INFINIY);
    }

    void setEdgeLevel(NODE_T u, NODE_T v, EDGELEVEL_T level) {
        for(int i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i] == v) {
                edgeLevelsOut[u][i] = level;
                break;
            }
        }

        for(int i = 0; i < neighborsIn[v].size(); ++i) {
            if(neighborsIn[v][i] == u) {
                edgeLevelsIn[v][i] = level;
                break;
            }
        }
    }

    EDGELEVEL_T getEdgeLevel(NODE_T u, NODE_T v) {
        for(int i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i] == v) {
                return edgeLevelsOut[u][i];
            }
        }
    }

    bool hasEdge(NODE_T u, NODE_T v) {
        for(auto neighbor : neighborsOut[u]) {
            if(neighbor == v) {
                return true;
            }
        }
        return false;
    }

    template<typename F>
    void forAllNeighborsIn(NODE_T v, F &&callback) {
        for(auto neighbor: neighborsIn[v]) {
            callback(neighbor);
        }
    }

    template<typename F>
    void forAllNeighborsOut(NODE_T v, F &&callback) {
        for(auto neighbor: neighborsOut[v]) {
            callback(neighbor);
        }
    }


    template<typename F>
    void forAllNeighborsInWithHighLevel(NODE_T v, EDGELEVEL_T levelThreshold, F &&callback) {
        for(int i = 0; i < neighborsIn[v].size(); ++i) {
            if(edgeLevelsIn[v][i] >= levelThreshold) {
                callback(neighborsIn[v][i], edgeLevelsIn[v][i]);
            }
        }
    }

    template<typename F>
    void forAllNeighborsOutWithHighLevel(NODE_T v, EDGELEVEL_T levelThreshold, F &&callback) {
        for(int i = 0; i < neighborsOut[v].size(); ++i) {
            if(edgeLevelsOut[v][i] >= levelThreshold) {
                callback(neighborsOut[v][i], edgeLevelsOut[v][i]);
            }
        }
    }

    template<typename F>
    void forAllNodes(F &&callback) {
        for(int v = 0; v < n; ++v) {
            callback(v);
        }
    }

/******************************************************************************/

protected:
    int n;
    vector<vector<NODE_T>> neighborsOut;
    vector<vector<EDGELEVEL_T>> edgeLevelsOut;
    vector<vector<NODE_T>> neighborsIn;
    vector<vector<EDGELEVEL_T>> edgeLevelsIn;
};
