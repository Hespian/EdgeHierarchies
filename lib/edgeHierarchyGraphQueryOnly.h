/*******************************************************************************
 * lib/edgeHierarchyGraphQueryOnly.h
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

#define GROUP_EDGES true

using namespace std;


class EdgeHierarchyGraphQueryOnly {
public:
    EdgeHierarchyGraphQueryOnly(NODE_T n) : n(n), m(0), neighborsOut(n), neighborsIn(n), edgesSorted(false), nodeMap(n), reverseNodeMap(n) {
        std::iota(std::begin(nodeMap), std::end(nodeMap), 0);
        std::iota(std::begin(reverseNodeMap), std::end(reverseNodeMap), 0);
    }

    NODE_T getNumberOfNodes() {
        return n;
    }

    EDGECOUNT_T getNumberOfEdges() {
        return m;
    }

    void setNodeMap(std::vector<NODE_T> &newMap) {
        nodeMap.swap(newMap);

        for(NODE_T i = 0; i < n; ++i) {
            reverseNodeMap[nodeMap[i]] = i;
        }
    }

    NODE_T getInternalNodeNumber(NODE_T externalNumber) {
        return nodeMap[externalNumber];
    }

    NODE_T getExternalNodeNumber(NODE_T internalNumber) {
        return reverseNodeMap[internalNumber];
    }

    void addEdge(NODE_T u, NODE_T v, EDGEWEIGHT_T weight) {
        if(edgesSorted) {
            std::cout << "Error. Trying add edge after sorting edges" <<std::endl;
            exit(1);
        }
        ++m;
        neighborsOut[u].push_back({v, weight, EDGERANK_INFINIY});
        neighborsIn[v].push_back({u, weight, EDGERANK_INFINIY});
    }


    void setEdgeRank(NODE_T u, NODE_T v, EDGERANK_T rank) {
        if(edgesSorted) {
            std::cout << "Error. Trying change edge rank after sorting edges" <<std::endl;
            exit(1);
        }
        for(size_t i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i].neighbor == v) {
                neighborsOut[u][i].rank = rank;
                break;
            }
        }

        for(size_t i = 0; i < neighborsIn[v].size(); ++i) {
            if(neighborsIn[v][i].neighbor == u) {
                neighborsIn[v][i].rank = rank;
                break;
            }
        }
    }


    template<typename F>
    void forAllNeighborsInAndStop(NODE_T v, F &&callback, int percent) {
        const size_t end = inBegin[v] + (((inBegin[v + 1] - inBegin[v]) * percent)/100);
        for(size_t i = inBegin[v]; i < end; ++i) {
#if GROUP_EDGES
            bool stop = callback(inEdges[i].neighbor, inEdges[i].weight);
#else
            bool stop = callback(inNeighbor[i], inWeight[i]);
#endif
            if(stop) {
                return;
            }
        }
    }

    template<typename F>
    void forAllNeighborsOutAndStop(NODE_T v, F &&callback, int percent) {
        const size_t end = outBegin[v] + (((outBegin[v + 1] - outBegin[v]) * percent)/100);
        for(size_t i = outBegin[v]; i < end; ++i) {
#if GROUP_EDGES
            bool stop = callback(outEdges[i].neighbor, outEdges[i].weight);
#else
            bool stop = callback(outNeighbor[i], outWeight[i]);
#endif
            if(stop) {
                return;
            }
        }
    }

    template<typename F>
    void forAllNeighborsInWithRank(NODE_T v, F &&callback) {
        for(size_t i = inBegin[v]; i < inBegin[v + 1]; ++i) {
#if GROUP_EDGES
            callback(inEdges[i].neighbor, inEdges[i].rank, inEdges[i].weight);
#else
            callback(inNeighbor[i], inRank[i], inWeight[i]);
#endif
        }
    }

    template<typename F>
    void forAllNeighborsOutWithRank(NODE_T v, F &&callback) {
        for(size_t i = outBegin[v]; i < outBegin[v + 1]; ++i) {
#if GROUP_EDGES
            callback(outEdges[i].neighbor, outEdges[i].rank, outEdges[i].weight);
#else
            callback(outNeighbor[i], outRank[i], outWeight[i]);
#endif
        }
    }

    template<typename F>
    void forAllNeighborsInWithHighRank(const NODE_T v, const EDGERANK_T _rankThreshold, F &&callback) {
        const EDGERANK_T rankThreshold = _rankThreshold;
        size_t i = inBegin[v];
        const size_t end = inBegin[v + 1];
        while(i < end &&
#if GROUP_EDGES
              inEdges[i].rank >= rankThreshold
#else
              inRank[i] >= rankThreshold
#endif
              ){
#if GROUP_EDGES
            callback(inEdges[i].neighbor, inEdges[i].rank, inEdges[i].weight);
#else
            callback(inNeighbor[i], inRank[i], inWeight[i]);
#endif
            ++i;
        }
    }

    template<typename F>
    void forAllNeighborsOutWithHighRank(const NODE_T v, const EDGERANK_T _rankThreshold, F &&callback) {
        const EDGERANK_T rankThreshold = _rankThreshold;
        size_t i = outBegin[v];
        const size_t end = outBegin[v + 1];
        while(i < end &&
#if GROUP_EDGES
              outEdges[i].rank >= rankThreshold
#else
              outRank[i] >= rankThreshold
#endif
              ){
#if GROUP_EDGES
            callback(outEdges[i].neighbor, outEdges[i].rank, outEdges[i].weight);
#else
            callback(outNeighbor[i], outRank[i], outWeight[i]);
#endif
            ++i;
        }
    }

    template<typename F>
    void forAllNodes(F &&callback) {
        for(NODE_T v = 0; v < n; ++v) {
            callback(v);
        }
    }

    void sortEdges() {
        if(edgesSorted) {
            return;
        }
        for(NODE_T v = 0; v < n; ++v) {
            sort(neighborsOut[v].begin(), neighborsOut[v].end(), [&] (edgeInfo i, edgeInfo j) {
                    return i.rank > j.rank;
                });
            sort(neighborsIn[v].begin(), neighborsIn[v].end(), [&] (edgeInfo i, edgeInfo j) {
                    return i.rank > j.rank;
                });
        }

        edgesSorted = true;
    }

    void makeConsecutive() {
        sortEdges();
        outBegin.clear();
        inBegin.clear();
#if GROUP_EDGES
        outEdges.clear();
        inEdges.clear();
#else
        outNeighbor.clear();
        inNeighbor.clear();
        outWeight.clear();
        inWeight.clear();
        outRank.clear();
        inRank.clear();
#endif
        forAllNodes([&] (NODE_T v) {
#if GROUP_EDGES
                outBegin.push_back(outEdges.size());
                inBegin.push_back(inEdges.size());
#else
                outBegin.push_back(outNeighbor.size());
                inBegin.push_back(inNeighbor.size());
#endif
                for(const auto &edge: neighborsOut[v]) {
#if GROUP_EDGES
                    outEdges.push_back(edge);
#else
                    outNeighbor.push_back(edge.neighbor);
                    outWeight.push_back(edge.weight);
                    outRank.push_back(edge.rank);
#endif
                }

                for(const auto &edge: neighborsIn[v]) {
#if GROUP_EDGES
                    inEdges.push_back(edge);
#else
                    inNeighbor.push_back(edge.neighbor);
                    inWeight.push_back(edge.weight);
                    inRank.push_back(edge.rank);
#endif
                }
            });
#if GROUP_EDGES
        outBegin.push_back(outEdges.size());
        inBegin.push_back(inEdges.size());
#else
        outBegin.push_back(outNeighbor.size());
        inBegin.push_back(inNeighbor.size());
#endif

        neighborsOut.clear();
        neighborsIn.clear();
    }

/******************************************************************************/

protected:
    NODE_T n;
    EDGECOUNT_T m;
    vector<vector<edgeInfo>> neighborsOut;
    vector<vector<edgeInfo>> neighborsIn;
    vector<EDGECOUNT_T> outBegin;
    vector<EDGECOUNT_T> inBegin;
#if GROUP_EDGES
    vector<edgeInfo> outEdges;
    vector<edgeInfo> inEdges;
#else
    vector<NODE_T> outNeighbor;
    vector<NODE_T> inNeighbor;
    vector<EDGEWEIGHT_T> outWeight;
    vector<EDGEWEIGHT_T> inWeight;
    vector<EDGERANK_T> outRank;
    vector<EDGERANK_T> inRank;
#endif
    bool edgesSorted;
    vector<NODE_T> nodeMap;
    vector<NODE_T> reverseNodeMap;
};
