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


template <bool stallForward, bool stallBackward, bool logVerticesSettled>
class EdgeHierarchyQueryOnly {
public:
    uint64_t numVerticesSettled;
    uint64_t numEdgesRelaxed;
    uint64_t numEdgesLookedAtForStalling;
    uint64_t popCount;
    uint64_t avgSearchSpace;
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
                                                             rankBackward(g.getNumberOfNodes()),

                                                             actualDistanceForward(stallForward ? g.getNumberOfNodes() : 0),
                                                             actualDistanceBackward(stallForward ? g.getNumberOfNodes() : 0),
                                                             actualDistanceSetForward(stallForward ? g.getNumberOfNodes() : 0),
                                                             actualDistanceSetBackward(stallForward ? g.getNumberOfNodes() : 0)
    {
        numVerticesSettled = 0;
        numEdgesRelaxed = 0;
        numEdgesLookedAtForStalling = 0;
    };

    void resetCounters() {
        numVerticesSettled = 0;
        numEdgesRelaxed = 0;
        numEdgesLookedAtForStalling = 0;
    }

    EDGEWEIGHT_T getDistance(NODE_T externalS, NODE_T externalT, float stallingPercent) {
        NODE_T s = g.getInternalNodeNumber(externalS);
        NODE_T t = g.getInternalNodeNumber(externalT);
        wasPushedForward.reset_all();
        wasPushedBackward.reset_all();
        if constexpr(stallForward) {
                actualDistanceSetForward.reset_all();
                actualDistanceSetBackward.reset_all();
            }

        popCount = 0;

        if constexpr(logVerticesSettled) {
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
                makeStep<true>(shortestPathMeetingNode, shortestPathLength, stallingPercent);
            }
            else {
                makeStep<false>(shortestPathMeetingNode, shortestPathLength, stallingPercent);
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
    bool canStallAtNodeBackward(const NODE_T v, int percent) {
        const RoutingKit::TimestampFlags &wasPushedCurrent = forward ? wasPushedForward : wasPushedBackward;
        const vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;

        bool result = false;

        auto stallCheckFunc = [&] (const NODE_T u, const EDGEWEIGHT_T weight) {
            ++numEdgesLookedAtForStalling;
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
            g.forAllNeighborsInAndStop(v, stallCheckFunc, percent);
        }
        else {
            g.forAllNeighborsOutAndStop(v, stallCheckFunc, percent);
        }
        return result;
    }

    template<bool forward>
    bool canStallAtNodeForward(NODE_T v) {
        vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;
        vector<EDGEWEIGHT_T> &actualDistanceCurrent = forward ? actualDistanceForward : actualDistanceBackward;
        RoutingKit::TimestampFlags &actualDistanceSetCurrent = forward ? actualDistanceSetForward : actualDistanceSetBackward;
        if(actualDistanceSetCurrent.is_set(v)) {
            return actualDistanceCurrent[v] < tentativeDistanceCurrent[v];
        } else {
            return false;
        }
    }

    template<bool forward>
    void makeStep(NODE_T &shortestPathMeetingNode, EDGEWEIGHT_T &shortestPathLength, int stallingPercent) {
        RoutingKit::MinIDQueue &PQCurrent = forward ? PQForward : PQBackward;
        RoutingKit::TimestampFlags &wasPushedCurrent = forward ? wasPushedForward : wasPushedBackward;
        RoutingKit::TimestampFlags &wasPushedOther = forward ? wasPushedBackward : wasPushedForward;
        vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;
        vector<EDGEWEIGHT_T> &tentativeDistanceOther = forward ? tentativeDistanceBackward : tentativeDistanceForward;
        vector<EDGERANK_T> &rankCurrent = forward ? rankForward : rankBackward;
        vector<EDGEWEIGHT_T> &actualDistanceCurrent = forward ? actualDistanceForward : actualDistanceBackward;
        RoutingKit::TimestampFlags &actualDistanceSetCurrent = forward ? actualDistanceSetForward : actualDistanceSetBackward;

        const auto popped = PQCurrent.pop();

        const NODE_T u = popped.id;
        const EDGERANK_T distanceU = popped.key;
        assert(distanceU == tentativeDistanceCurrent[u]);

        numVerticesSettled++;

        if constexpr(stallForward){
                if(canStallAtNodeForward<forward>(u)) {
                return;
            }
        }

        if constexpr(stallBackward){
                int stallingPercentThisIteration = (1.0 * numVerticesSettled)/avgSearchSpace * stallingPercent;
                stallingPercentThisIteration = std::clamp(stallingPercentThisIteration, 0, 100);
                if(canStallAtNodeBackward<forward>(u, stallingPercent)) {
                    return;
                }
            }
        // if constexpr(stallBackward){
        //         if(canStallAtNodeBackward<forward>(u, stallingPercent)) {
        //             return;
        //         }
        //     }


        if constexpr(logVerticesSettled) {
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

        auto relaxFunc = [&] (const NODE_T v, const EDGERANK_T rank, const EDGEWEIGHT_T weight) {
            ++numEdgesRelaxed;
            const EDGEWEIGHT_T distanceV = distanceU + weight;
            if(wasPushedCurrent.is_set(v)) {
                if(distanceV < tentativeDistanceCurrent[v]) {
                    if constexpr(stallForward){
                        if(!actualDistanceSetCurrent.is_set(v) || distanceV < actualDistanceCurrent[v]) {
                            PQCurrent.decrease_key({v, distanceV});
                            tentativeDistanceCurrent[v] = distanceV;
                            rankCurrent[v] = rank;
                        }
                    }
                    else {
                        PQCurrent.decrease_key({v, distanceV});
                        tentativeDistanceCurrent[v] = distanceV;
                        rankCurrent[v] = rank;
                    }

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

        if constexpr(stallForward) {
            auto stallFunc = [&] (const NODE_T v, const EDGERANK_T rank, const EDGEWEIGHT_T weight) {
                ++numEdgesLookedAtForStalling;
                EDGEWEIGHT_T const distanceV = distanceU + weight;
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

            if constexpr(forward) {
                g.forAllNeighborsOutWithRank(u, combinedFunc);
            }
            else {
                g.forAllNeighborsInWithRank(u, combinedFunc);
            }
        }
        else {
            if constexpr(forward) {
                    g.forAllNeighborsOutWithHighRank(u, rankCurrent[u], relaxFunc);
            }
            else {
                g.forAllNeighborsInWithHighRank(u, rankCurrent[u], relaxFunc);
            }
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
    vector<EDGEWEIGHT_T> actualDistanceForward;
    vector<EDGEWEIGHT_T> actualDistanceBackward;
    RoutingKit::TimestampFlags actualDistanceSetForward;
    RoutingKit::TimestampFlags actualDistanceSetBackward;
};
