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
    EdgeHierarchyQuery(EdgeHierarchyGraph &g) : g(g), PQForward(g.getNumberOfNodes()), PQBackward(g.getNumberOfNodes()) {

    };

    EDGEWEIGHT_T getDistance(NODE_T s, NODE_T t) {
        PQForward.push({s, 0});
        PQBackward.push({t, 0});
        bool forward = true;
        bool finished = false;

        EDGEWEIGHT_T shortestPathLength = numeric_limits<EDGEWEIGHT_T>::max();
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
                makeStep();
            }
            else {
                makeStep();
            }
            forward = !forward;
        }
        return shortestPathLength;
    }

protected:
    void makeStep() {
        
    }

    EdgeHierarchyGraph &g;
    RoutingKit::MinIDQueue PQForward;
    RoutingKit::MinIDQueue PQBackward;
};
