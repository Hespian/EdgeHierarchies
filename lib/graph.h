/*******************************************************************************
 * app/graph.h
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
    Graph(int n) : n(n), neighborsIn(n), neighborsOut(n) {

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
        neighborsIn[v].push_back(u);
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
    void forAllNodes(F &&callback) {
        for(int v = 0; v < n; ++v) {
            callback(v);
        }
    }

/******************************************************************************/

protected:
    int n;
    vector<vector<NODE_T>> neighborsOut;
    vector<vector<NODE_T>> neighborsIn;
};
