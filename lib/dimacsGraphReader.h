/*******************************************************************************
 * lib/dimacsGraphReader.h
 *
 * Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#pragma once

#include <fstream>
#include <sstream>
#include <string>

#include "definitions.h"
#include "edgeHierarchyGraph.h"

using namespace std;

EdgeHierarchyGraph readGraphDimacs(string fileName) {
    std::ifstream infile(fileName);

    NODE_T numVertices;
    EDGEID_T numEdges;
    string line;
    while (getline(infile, line)) {
        istringstream iss(line);
        char firstSymbol;
        if (!(iss >> firstSymbol)) { break; } // error

        if(firstSymbol == 'p') {
            string sp;
            iss >> sp >> numVertices >> numEdges;
            break;
        }
    }

    EdgeHierarchyGraph g(numVertices);

    while (getline(infile, line)) {
        istringstream iss(line);
        char firstSymbol;
        if (!(iss >> firstSymbol)) { break; } // error

        if(firstSymbol == 'a') {
            NODE_T u, v;
            EDGEWEIGHT_T weight;
            iss >> u >> v >> weight;
            if(!g.hasEdge(u - 1, v - 1)) {
                g.addEdge(u - 1, v - 1, weight);
            }
        }
    }

    return g;
}
