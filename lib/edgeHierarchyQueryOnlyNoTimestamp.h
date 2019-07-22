/*******************************************************************************
 * lib/edgeHierarchyQueryOnlyNoTimestamp.h
 *
 * Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include "assert.h"
#include <vector>
#include <utility>

#include "routingkit/id_queue.h"

#include "definitions.h"
#include "edgeHierarchyGraphQueryOnly.h"


template <bool stallForward, bool stallBackward, bool logVerticesSettled>
class EdgeHierarchyQueryOnlyNoTimestamp {
public:
    uint64_t numVerticesSettled;
    uint64_t numEdgesRelaxed;
    uint64_t numEdgesLookedAtForStalling;
    uint64_t popCount;
    std::vector<std::pair<NODE_T, EDGEWEIGHT_T>> verticesSettledForward;
    std::vector<std::pair<NODE_T, EDGEWEIGHT_T>> verticesSettledBackward;

    EdgeHierarchyQueryOnlyNoTimestamp(EdgeHierarchyGraphQueryOnly &g) : g(g),
                                                             PQForward(g.getNumberOfNodes()),
                                                             PQBackward(g.getNumberOfNodes()),
                                                             tentativeDistanceForward(g.getNumberOfNodes(), EDGEWEIGHT_INFINITY),
                                                             tentativeDistanceBackward(g.getNumberOfNodes(), EDGEWEIGHT_INFINITY),
                                                             rankForward(g.getNumberOfNodes()),
                                                             rankBackward(g.getNumberOfNodes())
    {
        numVerticesSettled = 0;
        numEdgesRelaxed = 0;
        numEdgesLookedAtForStalling = 0;
        visitedBackward.reserve(g.getNumberOfNodes());
        visitedForward.reserve(g.getNumberOfNodes());
        if(stallForward) {
            std::cout << "Forward stalling not supported" <<std::endl;
            exit(1);
        }
    };

    void resetCounters() {
        numVerticesSettled = 0;
        numEdgesRelaxed = 0;
        numEdgesLookedAtForStalling = 0;
    }

    EDGEWEIGHT_T getDistance(NODE_T externalS, NODE_T externalT) {
        NODE_T s = g.getInternalNodeNumber(externalS);
        NODE_T t = g.getInternalNodeNumber(externalT);

        popCount = 0;

        if constexpr(logVerticesSettled) {
            verticesSettledForward.clear();
            verticesSettledBackward.clear();
        }

        PQForward.push({s, 0});
        PQBackward.push({t, 0});
        tentativeDistanceForward[s] = 0;
        visitedForward.push_back(s);
        tentativeDistanceBackward[t] = 0;
        visitedBackward.push_back(t);
        rankForward[s] = 0;
        rankBackward[t] = 0;

        bool forward = true;
        bool finished = false;

        EDGEWEIGHT_T shortestPathLength = EDGEWEIGHT_INFINITY;
        NODE_T shortestPathMeetingNode = numeric_limits<NODE_T>::max();

        while(!finished) {
            bool forwardFinished = false;
            if(PQForward.empty()) {
                forwardFinished = true;
            }
            else if(PQForward.peek().key >= shortestPathLength) {
                forwardFinished = true;
            }

            bool backwardFinished = false;
            if(PQBackward.empty()) {
                backwardFinished = true;
            }
            else if(PQBackward.peek().key >= shortestPathLength) {
                backwardFinished = true;
            }

            if(forwardFinished && backwardFinished) {
                break;
            }

            if(forwardFinished) {
                forward = false;
            }
            if(backwardFinished) {
                forward = true;
            }

            if(forward) {
                makeStep<true>(shortestPathMeetingNode, shortestPathLength);
            }
            else {
                makeStep<false>(shortestPathMeetingNode, shortestPathLength);
            }
            forward = !forward;
            // TODO: Why is this wrong?
            // if(maximumDistance != EDGEWEIGHT_INFINITY && shortestPathLength <= maximumDistance) {
            //     finished = true;
            // }
        }
        PQForward.clear();
        PQBackward.clear();
        resetDistances();
        return shortestPathLength;
    }

    void resetDistances() {
        for(auto v : visitedForward) {
            tentativeDistanceForward[v] = EDGEWEIGHT_INFINITY;
        }
        visitedForward.clear();
        for(auto v : visitedBackward) {
            tentativeDistanceBackward[v] = EDGEWEIGHT_INFINITY;
        }
        visitedBackward.clear();
    }

protected:

    template<bool forward>
    bool canStallAtNodeBackward(const NODE_T v) {
        const vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;

        bool result = false;

        auto stallCheckFunc = [&] (const NODE_T u, const EDGEWEIGHT_T weight) {
            ++numEdgesLookedAtForStalling;
            if(tentativeDistanceCurrent[u] < EDGEWEIGHT_INFINITY) {
                EDGEWEIGHT_T distanceV = tentativeDistanceCurrent[u] + weight;
                if(distanceV < tentativeDistanceCurrent[v]) {
                    result = true;
                    return true;
                }
            }
            return false;
        };

        if(forward) {
            g.forAllNeighborsInAndStop(v, stallCheckFunc);
        }
        else {
            g.forAllNeighborsOutAndStop(v, stallCheckFunc);
        }
        return result;
    }


    template<bool forward>
    void makeStep(NODE_T &shortestPathMeetingNode, EDGEWEIGHT_T &shortestPathLength) {
        RoutingKit::MinIDQueue &PQCurrent = forward ? PQForward : PQBackward;
        vector<NODE_T> &visitedCurrent = forward ? visitedForward : visitedBackward;
        vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;
        vector<EDGEWEIGHT_T> &tentativeDistanceOther = forward ? tentativeDistanceBackward : tentativeDistanceForward;
        vector<EDGERANK_T> &rankCurrent = forward ? rankForward : rankBackward;

        const auto popped = PQCurrent.pop();

        const NODE_T u = popped.id;
        const EDGERANK_T distanceU = popped.key;
        assert(distanceU == tentativeDistanceCurrent[u]);

        numVerticesSettled++;

        if constexpr(stallBackward){
                if(canStallAtNodeBackward<forward>(u)) {
                    return;
                }
            }


        if constexpr(logVerticesSettled) {
            if(forward){
                verticesSettledForward.push_back({g.getExternalNodeNumber(u), distanceU});
            } else {
                verticesSettledBackward.push_back({g.getExternalNodeNumber(u), distanceU});
            }
        }

        if(tentativeDistanceOther[u] < EDGEWEIGHT_INFINITY){
			if(shortestPathLength > distanceU + tentativeDistanceOther[u]){
				shortestPathLength = distanceU + tentativeDistanceOther[u];
				shortestPathMeetingNode = u;
			}
		}

        auto relaxFunc = [&] (const NODE_T v, const EDGERANK_T rank, const EDGEWEIGHT_T weight) {
            ++numEdgesRelaxed;
            const EDGEWEIGHT_T distanceV = distanceU + weight;
            if(tentativeDistanceCurrent[v] < EDGEWEIGHT_INFINITY) {
                if(distanceV < tentativeDistanceCurrent[v]) {
                    PQCurrent.decrease_key({v, distanceV});
                    tentativeDistanceCurrent[v] = distanceV;
                    rankCurrent[v] = rank;
                }
               else if(distanceV == tentativeDistanceCurrent[v] && rankCurrent[v] < rank) {
                   rankCurrent[v] = rank;
               }
            }
            else {
                PQCurrent.push({v, distanceV});
                tentativeDistanceCurrent[v] = distanceV;
                visitedCurrent.push_back(v);
                rankCurrent[v] = rank;
            }
        };

        if constexpr(forward) {
                g.forAllNeighborsOutWithHighRank(u, rankCurrent[u], relaxFunc);
            }
        else {
            g.forAllNeighborsInWithHighRank(u, rankCurrent[u], relaxFunc);
        }
    }

    EdgeHierarchyGraphQueryOnly &g;
    RoutingKit::MinIDQueue PQForward;
    RoutingKit::MinIDQueue PQBackward;
    vector<EDGEWEIGHT_T> tentativeDistanceForward;
    vector<EDGEWEIGHT_T> tentativeDistanceBackward;
    vector<EDGERANK_T> rankForward;
    vector<EDGERANK_T> rankBackward;
    std::vector<NODE_T> visitedForward;
    std::vector<NODE_T>visitedBackward;
};
