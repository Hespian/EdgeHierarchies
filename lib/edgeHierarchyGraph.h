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
    EdgeHierarchyGraph(NODE_T n) : n(n), m(0), neighborsOut(n), neighborsIn(n), edgesSorted(false), nodeMap(n), reverseNodeMap(n) {
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

    NODE_T getInDegree(NODE_T v) {
        return neighborsIn[v].size();
    }

    NODE_T getOutDegree(NODE_T v) {
        return neighborsOut[v].size();
    }

    void addEdge(NODE_T u, NODE_T v, EDGEWEIGHT_T weight) {
        assert(!hasEdge(u, v));
        ++m;
        neighborsOut[u].push_back({v, weight, EDGERANK_INFINIY});
        neighborsIn[v].push_back({u, weight, EDGERANK_INFINIY});
    }

    void decreaseEdgeWeight(NODE_T u, NODE_T v, EDGEWEIGHT_T weight) {
        assert(hasEdge(u, v));
        if(getEdgeWeight(u,v) < weight){
            return;
        }
        assert(getEdgeWeight(u, v) >= weight);
        for(size_t i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i].neighbor == v) {
                neighborsOut[u][i].weight = weight;
                break;
            }
        }
        for(size_t i = 0; i < neighborsIn[v].size(); ++i) {
            if(neighborsIn[v][i].neighbor == u) {
                neighborsIn[v][i].weight = weight;
                return;
            }
        }
        assert(false);
    }

    void setEdgeRank(NODE_T u, NODE_T v, EDGERANK_T rank) {
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

    EDGERANK_T getEdgeRank(NODE_T u, NODE_T v) {
        for(size_t i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i].neighbor == v) {
                return neighborsOut[u][i].rank;
            }
        }
        assert(false);
        return EDGERANK_INFINIY;
    }

    bool hasEdge(NODE_T u, NODE_T v) {
        for(auto neighbor : neighborsOut[u]) {
            if(neighbor.neighbor == v) {
                return true;
            }
        }
        return false;
    }

    EDGEWEIGHT_T getEdgeWeight(NODE_T u, NODE_T v) {
        for(size_t i = 0; i < neighborsOut[u].size(); ++i) {
            if(neighborsOut[u][i].neighbor == v) {
                return neighborsOut[u][i].weight;
            }
        }
        assert(false);
        return EDGEWEIGHT_INFINITY;
    }

    template<typename F>
    void forAllNeighborsIn(NODE_T v, F &&callback) {
        for(size_t i = 0; i < neighborsIn[v].size(); ++i) {
            callback(neighborsIn[v][i].neighbor, neighborsIn[v][i].weight);
        }
    }

    template<typename F>
    void forAllNeighborsOut(NODE_T v, F &&callback) {
        for(size_t i = 0; i < neighborsOut[v].size(); ++i) {
            callback(neighborsOut[v][i].neighbor, neighborsOut[v][i].weight);
        }
    }

    template<typename F>
    void forAllNeighborsInAndStop(NODE_T v, F &&callback) {
        for(size_t i = 0; i < neighborsIn[v].size(); ++i) {
            bool stop = callback(neighborsIn[v][i].neighbor, neighborsIn[v][i].weight);
            if(stop) {
                return;
            }
        }
    }

    template<typename F>
    void forAllNeighborsOutAndStop(NODE_T v, F &&callback) {
        for(size_t i = 0; i < neighborsOut[v].size(); ++i) {
            bool stop = callback(neighborsOut[v][i].neighbor, neighborsOut[v][i].weight);
            if(stop) {
                return;
            }
        }
    }


    template<typename F>
    void forAllNeighborsInWithHighRank(NODE_T v, EDGERANK_T rankThreshold, F &&callback) {
        for(size_t i = 0; i < neighborsIn[v].size(); ++i) {
            if(neighborsIn[v][i].rank >= rankThreshold) {
                callback(neighborsIn[v][i].neighbor, neighborsIn[v][i].rank, neighborsIn[v][i].weight);
            } else if(edgesSorted) {
                break;
            }
        }
    }

    template<typename F>
    void forAllNeighborsOutWithHighRank(NODE_T v, EDGERANK_T rankThreshold, F &&callback) {
        for(size_t i = 0; i < neighborsOut[v].size(); ++i) {
            if(neighborsOut[v][i].rank >= rankThreshold) {
                callback(neighborsOut[v][i].neighbor, neighborsOut[v][i].rank, neighborsOut[v][i].weight);
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
            sort(neighborsOut[v].begin(), neighborsOut[v].end(), [&] (edgeInfo i, edgeInfo j) {
                    return i.rank > j.rank;
                });
            sort(neighborsIn[v].begin(), neighborsIn[v].end(), [&] (edgeInfo i, edgeInfo j) {
                    return i.rank > j.rank;
                });
        }

        edgesSorted = true;
    }

    EdgeHierarchyGraph getTurnCostGraph() {
        std::vector<EDGEID_T> outDegree(n + 1);
        outDegree[0] = 0;
        forAllNodes([&] (NODE_T v) {
                outDegree[v + 1] = getOutDegree(v);
            });

        std::vector<EDGEID_T> nodeBegin(n + 1);
        std::partial_sum(outDegree.begin(), outDegree.end(), nodeBegin.begin());

        EdgeHierarchyGraph result(nodeBegin.back());

        forAllNodes([&] (NODE_T u) {
                EDGECOUNT_T uNeighborCounter = 0;
                forAllNeighborsOut(u, [&] (NODE_T v, EDGEWEIGHT_T originalWeight) {
                        NODE_T uNew = nodeBegin[u] + uNeighborCounter;
                        EDGECOUNT_T vNeighborCounter = 0;
                        forAllNeighborsOut(v, [&] (NODE_T x, EDGEWEIGHT_T ignoreWeight) {
                                NODE_T vNew = nodeBegin[v] + vNeighborCounter;
                                EDGEWEIGHT_T newWeight = originalWeight;
                                if(x == u) {
                                    newWeight += 1000;
                                }
                                result.addEdge(uNew, vNew, newWeight);
                                ++vNeighborCounter;
                            });
                        ++uNeighborCounter;
                    });
            });

        return result;

    }

    template<typename T>
    T getDFSOrderGraph() {
        auto result = T(n);

        NODE_T dfsCount = 0;
        std::vector<NODE_T> dfsNum(n, NODE_INVALID);
        std::vector<bool> marks(n, false);

        // forAllNodes([&] (NODE_T v) {
        //         DFS(v, marks, dfsNum, dfsCount);
        //     });

        std::vector<NODE_T> stack;

        while(!std::all_of(marks.begin(), marks.end(), [](bool v) { return v; })) {
            for(NODE_T v = 0; v < n; ++v) {
                if(!marks[v]) {
                    stack.push_back(v);
                    break;
                }
            }

            while(!stack.empty()) {
                NODE_T v = stack.back();
                if(marks[v]) {
                    stack.pop_back();
                    if(dfsNum[v] == NODE_INVALID) {
                        dfsNum[v] = dfsCount++;
                    }
                } else {
                    marks[v] = true;
                    for(int i = neighborsOut[v].size() - 1; i >=0; --i) {
                        NODE_T w = neighborsOut[v][i].neighbor;
                        if(!marks[w]) {
                            stack.push_back(w);
                        }
                    }
                }
            }
        }

        assert(dfsCount == n);
        if(dfsCount != n) {
            std::cout << "Error! dfsCount " << dfsCount << " instead of " << n << std::endl;
            exit(1);
        }


        forAllNodes([&] (NODE_T v) {
                forAllNeighborsOut(v, [&] (NODE_T w, EDGEWEIGHT_T weight) {
                        result.addEdge(dfsNum[v], dfsNum[w], weight);
                        result.setEdgeRank(dfsNum[v], dfsNum[w], getEdgeRank(v, w));
                    });
            });
        result.setNodeMap(dfsNum);

        return result;
    }

    void DFS(NODE_T v, std::vector<bool> &marks, std::vector<NODE_T> &dfsNum, NODE_T &dfsCount) {
        if(marks[v])
            return;
        marks[v] = true;

        forAllNeighborsOut(v, [&] (NODE_T w, EDGEWEIGHT_T weight) {
                DFS(w, marks, dfsNum, dfsCount);
            });

        dfsNum[v] = dfsCount++;

    }

/******************************************************************************/

protected:
    NODE_T n;
    EDGECOUNT_T m;
    vector<vector<edgeInfo>> neighborsOut;
    vector<vector<edgeInfo>> neighborsIn;
    bool edgesSorted;
    vector<NODE_T> nodeMap;
    vector<NODE_T> reverseNodeMap;
};
