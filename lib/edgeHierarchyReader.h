/*******************************************************************************
 * lib/edgeHierarchyReader.h
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


EdgeHierarchyGraph readEdgeHierarchy(string fileName) {
    std::ifstream infile(fileName);

    NODE_T numVertices;
    EDGEID_T numEdges;
    string line;

    getline(infile, line);

    istringstream iss(line);

    iss >> numVertices >> numEdges;

    EdgeHierarchyGraph g(numVertices);

    while (getline(infile, line)) {
        istringstream iss(line);
        NODE_T u, v;
        EDGEWEIGHT_T weight;
        EDGERANK_T rank;
        iss >> u >> v >> weight >> rank;
        if(!g.hasEdge(u, v)) {
            g.addEdge(u, v, weight);
            g.setEdgeRank(u, v, rank);
        }
    }

    return g;
    infile.close();
}
