/*******************************************************************************
 * lib/edgeHierarchyWriter.h
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


void writeEdgeHierarchy(string fileName, EdgeHierarchyGraph &g) {
    std::ofstream outfile(fileName);

    outfile << g.getNumberOfNodes() << " " << g.getNumberOfEdges() << std::endl;

    g.forAllNodes([&] (NODE_T u) {
            g.forAllNeighborsOutWithHighRank(u, 0, [&] (NODE_T v, EDGERANK_T rank, EDGEWEIGHT_T weight) {
                    outfile << u << " " << v << " " << weight << " " << rank << std::endl;
                });
        });

    outfile.close();
}
