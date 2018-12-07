/*******************************************************************************
 * lib/edgeHierarchyQuery.h
 *
 * Copyright (C) 2018 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include "routingkit/id_queue.h"
#include "routingkit/timestamp_flag.h"

#include "definitions.h"
#include "edgeHierarchyGraph.h"

class EdgeHierarchyQuery {
public:
    EdgeHierarchyQuery(EdgeHierarchyGraph &g) : g(g),
                                                PQForward(g.getNumberOfNodes()),
                                                PQBackward(g.getNumberOfNodes()),
                                                wasPushedForward(g.getNumberOfNodes()),
                                                wasPushedBackward(g.getNumberOfNodes()),
                                                tentativeDistanceForward(g.getNumberOfNodes()),
                                                tentativeDistanceBackward(g.getNumberOfNodes()),
                                                levelForward(g.getNumberOfNodes()),
                                                levelBackward(g.getNumberOfNodes()) {

    };

    EDGEWEIGHT_T getDistance(NODE_T s, NODE_T t) {
        wasPushedForward.reset_all();
        wasPushedBackward.reset_all();

        PQForward.push({s, 0});
        PQBackward.push({t, 0});
        wasPushedForward.set(s);
        wasPushedBackward.set(t);
        tentativeDistanceForward[s] = 0;
        tentativeDistanceBackward[t] = 0;
        levelForward[s] = 0;
        levelBackward[t] = 0;

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
        }
        PQForward.clear();
        PQBackward.clear();
        return shortestPathLength;
    }

protected:

    template<bool forward>
    void makeStep(NODE_T &shortestPathMeetingNode, EDGEWEIGHT_T &shortestPathLength) {
        RoutingKit::MinIDQueue &PQCurrent = forward ? PQForward : PQBackward;
        RoutingKit::TimestampFlags &wasPushedCurrent = forward ? wasPushedForward : wasPushedBackward;
        RoutingKit::TimestampFlags &wasPushedOther = forward ? wasPushedBackward : wasPushedForward;
        vector<EDGEWEIGHT_T> &tentativeDistanceCurrent = forward ? tentativeDistanceForward : tentativeDistanceBackward;
        vector<EDGEWEIGHT_T> &tentativeDistanceOther = forward ? tentativeDistanceBackward : tentativeDistanceForward;
        vector<EDGELEVEL_T> &levelCurrent = forward ? levelForward : levelBackward;

        auto popped = PQCurrent.pop();

        NODE_T u = popped.id;
        EDGELEVEL_T distanceU = popped.key;
        assert(distanceU == tentativeDistanceCurrent[u]);

        if(wasPushedOther.is_set(u)){
			if(shortestPathLength > distanceU + tentativeDistanceOther[u]){
				shortestPathLength = distanceU + tentativeDistanceOther[u];
				shortestPathMeetingNode = u;
			}
		}

        auto relaxFunc = [&] (NODE_T v, EDGELEVEL_T level, EDGEWEIGHT_T weight) {
            EDGEWEIGHT_T distanceV = distanceU + weight;
            if(wasPushedCurrent.is_set(v)) {
                if(distanceV < tentativeDistanceForward[v]) {
                    PQCurrent.decrease_key({v, distanceV});
                    tentativeDistanceCurrent[v] = distanceV;
                    levelCurrent[v] = level;
                }
            }
            else {
                PQCurrent.push({v, distanceV});
                tentativeDistanceCurrent[v] = distanceV;
                wasPushedCurrent.set(v);
                levelCurrent[v] = level;
            }
        };

        if(forward) {
            g.forAllNeighborsOutWithHighLevel(u, levelCurrent[u], relaxFunc);
        }
        else {
            g.forAllNeighborsInWithHighLevel(u, levelCurrent[u], relaxFunc);
        }
    }

    EdgeHierarchyGraph &g;
    RoutingKit::MinIDQueue PQForward;
    RoutingKit::MinIDQueue PQBackward;
    RoutingKit::TimestampFlags wasPushedForward;
    RoutingKit::TimestampFlags wasPushedBackward;
    vector<EDGEWEIGHT_T> tentativeDistanceForward;
    vector<EDGEWEIGHT_T> tentativeDistanceBackward;
    vector<EDGELEVEL_T> levelForward;
    vector<EDGELEVEL_T> levelBackward;
};
