/*******************************************************************************
 * lib/edgeHierarchyQuery.h
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
#include "edgeHierarchyGraph.h"

class EdgeHierarchyQuery {
public:
    int numVerticesSettled;
    int numEdgesRelaxed;

    EdgeHierarchyQuery(EdgeHierarchyGraph &g) : g(g),
                                                PQForward(g.getNumberOfNodes()),
                                                PQBackward(g.getNumberOfNodes()),
                                                wasPushedForward(g.getNumberOfNodes()),
                                                wasPushedBackward(g.getNumberOfNodes()),
                                                tentativeDistanceForward(g.getNumberOfNodes()),
                                                tentativeDistanceBackward(g.getNumberOfNodes()),
                                                rankForward(g.getNumberOfNodes()),
                                                rankBackward(g.getNumberOfNodes()) {
        numVerticesSettled = 0;
        numEdgesRelaxed = 0;
    };

    void resetCounters() {
        numVerticesSettled = 0;
        numEdgesRelaxed = 0;
    }
    EDGEWEIGHT_T getDistance(NODE_T s, NODE_T t) {
        return getDistance(s, t, EDGEWEIGHT_INFINITY);
    }

    EDGEWEIGHT_T getDistance(NODE_T s, NODE_T t, EDGEWEIGHT_T maximumDistance) {
        wasPushedForward.reset_all();
        wasPushedBackward.reset_all();

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
            else if(PQForward.peek().key >= shortestPathLength || PQForward.peek().key >= maximumDistance) {
                forwardFinished = true;
            }

            bool backwardFinished = false;
            if(PQBackward.empty()) {
                backwardFinished = true;
            }
            else if(PQBackward.peek().key >= shortestPathLength || PQBackward.peek().key >= maximumDistance) {
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
    bool canStallAtNode(NODE_T v) {
        RoutingKit::TimestampFlags &wasPushedCurrent = forward ? wasPushedForward : wasPushedBackward;
        vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;

        bool result = false;

        auto stallCheckFunc = [&] (NODE_T u, EDGEWEIGHT_T weight) {
            if(wasPushedCurrent.is_set(u)) {
                EDGEWEIGHT_T distanceV = tentativeDistanceCurrent[u] + weight;
                if(distanceV < tentativeDistanceCurrent[v]) {
                    result = true;
                }
            }
        };

        if(forward) {
            g.forAllNeighborsIn(v, stallCheckFunc);
        }
        else {
            g.forAllNeighborsOut(v, stallCheckFunc);
        }
        return result;
    }

    template<bool forward>
    void makeStep(NODE_T &shortestPathMeetingNode, EDGEWEIGHT_T &shortestPathLength) {
        RoutingKit::MinIDQueue &PQCurrent = forward ? PQForward : PQBackward;
        RoutingKit::TimestampFlags &wasPushedCurrent = forward ? wasPushedForward : wasPushedBackward;
        RoutingKit::TimestampFlags &wasPushedOther = forward ? wasPushedBackward : wasPushedForward;
        vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;
        vector<EDGEWEIGHT_T> &tentativeDistanceOther = forward ? tentativeDistanceBackward : tentativeDistanceForward;
        vector<EDGERANK_T> &rankCurrent = forward ? rankForward : rankBackward;

        auto popped = PQCurrent.pop();
        numVerticesSettled++;

        NODE_T u = popped.id;
        EDGERANK_T distanceU = popped.key;
        assert(distanceU == tentativeDistanceCurrent[u]);

        if(canStallAtNode<forward>(u)) {
            return;
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
                wasPushedCurrent.set(v);
                rankCurrent[v] = rank;
            }
        };

        if(forward) {
            g.forAllNeighborsOutWithHighRank(u, rankCurrent[u], relaxFunc);
        }
        else {
            g.forAllNeighborsInWithHighRank(u, rankCurrent[u], relaxFunc);
        }
    }

    EdgeHierarchyGraph &g;
    RoutingKit::MinIDQueue PQForward;
    RoutingKit::MinIDQueue PQBackward;
    RoutingKit::TimestampFlags wasPushedForward;
    RoutingKit::TimestampFlags wasPushedBackward;
    vector<EDGEWEIGHT_T> tentativeDistanceForward;
    vector<EDGEWEIGHT_T> tentativeDistanceBackward;
    vector<EDGERANK_T> rankForward;
    vector<EDGERANK_T> rankBackward;
};
