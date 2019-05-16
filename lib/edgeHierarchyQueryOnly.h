/*******************************************************************************
 * lib/edgeHierarchyQueryOnly.h
 *
 * Copyright (C) 2018-2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include "assert.h"
#include <vector>
#include <utility>

#include "routingkit/id_queue.h"
#include "routingkit/timestamp_flag.h"

#include "definitions.h"
#include "edgeHierarchyGraphQueryOnly.h"

#define LOG_VERTICES_SETTLED false
// #define USE_STALLING

class EdgeHierarchyQueryOnly {
public:
    int numVerticesSettled;
    int numEdgesRelaxed;
    int popCount;
    std::vector<std::pair<NODE_T, EDGEWEIGHT_T>> verticesSettledForward;
    std::vector<std::pair<NODE_T, EDGEWEIGHT_T>> verticesSettledBackward;

    EdgeHierarchyQueryOnly(EdgeHierarchyGraphQueryOnly &g) : g(g),
                                                             PQForward(g.getNumberOfNodes()),
                                                             PQBackward(g.getNumberOfNodes()),
                                                             wasPushedForward(g.getNumberOfNodes()),
                                                             wasPushedBackward(g.getNumberOfNodes()),
                                                             tentativeDistanceForward(g.getNumberOfNodes()),
                                                             tentativeDistanceBackward(g.getNumberOfNodes()),
                                                             rankForward(g.getNumberOfNodes()),
                                                             rankBackward(g.getNumberOfNodes())
#ifdef USE_STALLING
                                                           ,actualDistanceForward(g.getNumberOfNodes()),
                                                             actualDistanceBackward(g.getNumberOfNodes()),
                                                             actualDistanceSetForward(g.getNumberOfNodes()),
                                                             actualDistanceSetBackward(g.getNumberOfNodes())
#endif
    {
        numVerticesSettled = 0;
        numEdgesRelaxed = 0;
    };

    void resetCounters() {
        numVerticesSettled = 0;
        numEdgesRelaxed = 0;
    }

    EDGEWEIGHT_T getDistance(NODE_T externalS, NODE_T externalT) {
        NODE_T s = g.getInternalNodeNumber(externalS);
        NODE_T t = g.getInternalNodeNumber(externalT);
        wasPushedForward.reset_all();
        wasPushedBackward.reset_all();
#ifdef USE_STALLING
        actualDistanceSetForward.reset_all();
        actualDistanceSetBackward.reset_all();
#endif

        popCount = 0;

        if(LOG_VERTICES_SETTLED) {
            verticesSettledForward.clear();
            verticesSettledBackward.clear();
        }

        PQForward.push({s, 0});
        PQBackward.push({t, 0});
        wasPushedForward.set(s);
        wasPushedBackward.set(t);
        tentativeDistanceForward[s] = 0;
        tentativeDistanceBackward[t] = 0;
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
        return shortestPathLength;
    }

protected:

    template<bool forward>
    bool canStallAtNodeOld(NODE_T v) {
        RoutingKit::TimestampFlags &wasPushedCurrent = forward ? wasPushedForward : wasPushedBackward;
        vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;

        bool result = false;

        auto stallCheckFunc = [&] (NODE_T u, EDGEWEIGHT_T weight) {
            ++numEdgesRelaxed;
            if(wasPushedCurrent.is_set(u)) {
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

#ifdef USE_STALLING
    template<bool forward>
    bool canStallAtNode(NODE_T v) {
        vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;
        vector<EDGEWEIGHT_T> &actualDistanceCurrent = forward ? actualDistanceForward : actualDistanceBackward;
        RoutingKit::TimestampFlags &actualDistanceSetCurrent = forward ? actualDistanceSetForward : actualDistanceSetBackward;
        if(actualDistanceSetCurrent.is_set(v)) {
            return actualDistanceCurrent[v] < tentativeDistanceCurrent[v];
        } else {
            return false;
        }
    }
#endif

    template<bool forward>
    void makeStep(NODE_T &shortestPathMeetingNode, EDGEWEIGHT_T &shortestPathLength) {
        RoutingKit::MinIDQueue &PQCurrent = forward ? PQForward : PQBackward;
        RoutingKit::TimestampFlags &wasPushedCurrent = forward ? wasPushedForward : wasPushedBackward;
        RoutingKit::TimestampFlags &wasPushedOther = forward ? wasPushedBackward : wasPushedForward;
        vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;
        vector<EDGEWEIGHT_T> &tentativeDistanceOther = forward ? tentativeDistanceBackward : tentativeDistanceForward;
        vector<EDGERANK_T> &rankCurrent = forward ? rankForward : rankBackward;
#ifdef USE_STALLING
        vector<EDGEWEIGHT_T> &actualDistanceCurrent = forward ? actualDistanceForward : actualDistanceBackward;
        RoutingKit::TimestampFlags &actualDistanceSetCurrent = forward ? actualDistanceSetForward : actualDistanceSetBackward;
#endif

        auto popped = PQCurrent.pop();

        NODE_T u = popped.id;
        EDGERANK_T distanceU = popped.key;
        assert(distanceU == tentativeDistanceCurrent[u]);

        numVerticesSettled++;

#ifdef USE_STALLING
        if(canStallAtNode<forward>(u)) {
            return;
        }
#endif

        if(LOG_VERTICES_SETTLED) {
            if(forward){
                verticesSettledForward.push_back({g.getExternalNodeNumber(u), distanceU});
            } else {
                verticesSettledBackward.push_back({g.getExternalNodeNumber(u), distanceU});
            }
        }

        if(wasPushedOther.is_set(u)){
			if(shortestPathLength > distanceU + tentativeDistanceOther[u]){
				shortestPathLength = distanceU + tentativeDistanceOther[u];
				shortestPathMeetingNode = u;
			}
		}

        auto relaxFunc = [&] (NODE_T v, EDGERANK_T rank, EDGEWEIGHT_T weight) {
            ++numEdgesRelaxed;
            EDGEWEIGHT_T distanceV = distanceU + weight;
            if(wasPushedCurrent.is_set(v)) {
                if(distanceV < tentativeDistanceCurrent[v]) {
#ifdef USE_STALLING
                    if(!actualDistanceSetCurrent.is_set(v) || distanceV < actualDistanceCurrent[v]) {
#endif
                        PQCurrent.decrease_key({v, distanceV});
                        tentativeDistanceCurrent[v] = distanceV;
                        rankCurrent[v] = rank;
#ifdef USE_STALLING
                    }
#endif

                }
               else if(distanceV == tentativeDistanceCurrent[v] && rankCurrent[v] < rank) {
                   rankCurrent[v] = rank;
               }
            }
            else {
                PQCurrent.push({v, distanceV});
                tentativeDistanceCurrent[v] = distanceV;
                wasPushedCurrent.set(v);
                rankCurrent[v] = rank;
            }
        };

#ifdef USE_STALLING
        auto stallFunc = [&] (NODE_T v, EDGERANK_T rank, EDGEWEIGHT_T weight) {
            ++numEdgesRelaxed;
            EDGEWEIGHT_T distanceV = distanceU + weight;
            if(wasPushedCurrent.is_set(v)) {
                if(tentativeDistanceCurrent[v] > distanceV) {
                    if(actualDistanceSetCurrent.is_set(v)) {
                        if(actualDistanceCurrent[v] > distanceV) {
                            actualDistanceCurrent[v] = distanceV;
                        }
                    } else {
                        actualDistanceCurrent[v] = distanceV;
                        actualDistanceSetCurrent.set(v);
                    }
                }
            } else {
                if(actualDistanceSetCurrent.is_set(v)) {
                    if(actualDistanceCurrent[v] > distanceV) {
                        actualDistanceCurrent[v] = distanceV;
                    }
                } else {
                    actualDistanceCurrent[v] = distanceV;
                    actualDistanceSetCurrent.set(v);
                }
            }
        };

        EDGERANK_T rankU = rankCurrent[u];
        auto combinedFunc = [&] (NODE_T v, EDGERANK_T rank, EDGEWEIGHT_T weight) {
            if(rank >= rankU) {
                relaxFunc(v, rank, weight);
            } else {
                stallFunc(v, rank, weight);
            }
        };
#endif

        if(forward) {
#ifdef USE_STALLING
            g.forAllNeighborsOutWithRank(u, combinedFunc);
#else
            g.forAllNeighborsOutWithHighRank(u, rankCurrent[u], relaxFunc);
#endif
        }
        else {
#ifdef USE_STALLING
            g.forAllNeighborsInWithRank(u, combinedFunc);
#else
            g.forAllNeighborsInWithHighRank(u, rankCurrent[u], relaxFunc);
#endif
        }
    }

    EdgeHierarchyGraphQueryOnly &g;
    RoutingKit::MinIDQueue PQForward;
    RoutingKit::MinIDQueue PQBackward;
    RoutingKit::TimestampFlags wasPushedForward;
    RoutingKit::TimestampFlags wasPushedBackward;
    vector<EDGEWEIGHT_T> tentativeDistanceForward;
    vector<EDGEWEIGHT_T> tentativeDistanceBackward;
    vector<EDGERANK_T> rankForward;
    vector<EDGERANK_T> rankBackward;
#ifdef USE_STALLING
    vector<EDGEWEIGHT_T> actualDistanceForward;
    vector<EDGEWEIGHT_T> actualDistanceBackward;
    RoutingKit::TimestampFlags actualDistanceSetForward;
    RoutingKit::TimestampFlags actualDistanceSetBackward;
#endif
};
