/*******************************************************************************
 * app/benchmark.cpp
 *
 * Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
 *
 * All rights reserved.
 ******************************************************************************/

#include <string>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <stdlib.h>

#include <tlx/cmdline_parser.hpp>

#include "definitions.h"
#include "edgeHierarchyGraph.h"
#include "edgeHierarchyQuery.h"
#include "edgeHierarchyConstruction.h"
#include "dimacsGraphReader.h"
#include "edgeRanking/shortcutCountingRoundsEdgeRanker.h"

int main(int argc, char* argv[]) {
    tlx::CmdlineParser cp;

    cp.set_description("Benchmark programm for EdgeHierarchies");
    cp.set_author("Demian Hespe <hespe@kit.edu>");

    unsigned numQueries = 1000;
    cp.add_unsigned('q', "queries", "N", numQueries,
                    "Run N queries.");

    unsigned seed = 0;
    cp.add_unsigned('s', "seed", "N", seed,
                    "The seed to use for the prng.");

    std::string filename;
    cp.add_param_string("filename", filename,
                        "Filename of the graph to benchmark on");

    // process command line
    if (!cp.process(argc, argv))
        return -1; // some error occurred and help was always written to user.
    // output for debugging
    cp.print_result();

    auto start = chrono::steady_clock::now();
    EdgeHierarchyGraph g = readGraphDimacs(filename);
	auto end = chrono::steady_clock::now();

	cout << "Reading input graph took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    EdgeHierarchyQuery query(g);

    EdgeHierarchyGraph originalGraph(g);
    EdgeHierarchyQuery originalGraphQuery(originalGraph);

    EdgeHierarchyConstruction<ShortcutCountingRoundsEdgeRanker> construction(g, query);

    start = chrono::steady_clock::now();
    construction.run();
	end = chrono::steady_clock::now();

	cout << "Construction took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    srand (seed);

    int numMistakes = 0;
    for(unsigned i = 0; i < numQueries; ++i) {
        NODE_T u = rand() % g.getNumberOfNodes();
        NODE_T v = rand() % g.getNumberOfNodes();

        EDGEWEIGHT_T distance = query.getDistance(u, v);

        EDGEWEIGHT_T originalGraphDistance = originalGraphQuery.getDistance(u, v);

        if(distance != originalGraphDistance) {
            cout << "Wrong distance for " << u << " and " << v << ": " << distance << " (should be " << originalGraphDistance << ")" << endl;
            numMistakes++;
        }
    }

    cout << "Done checking. Measuring time..." << endl;


    srand (seed);

    start = chrono::steady_clock::now();
    for(unsigned i = 0; i < numQueries; ++i) {
        NODE_T u = rand() % g.getNumberOfNodes();
        NODE_T v = rand() % g.getNumberOfNodes();

        EDGEWEIGHT_T distance = query.getDistance(u, v);
    }
	end = chrono::steady_clock::now();

	cout << "Average query time : "
         << chrono::duration_cast<chrono::microseconds>(end - start).count()
         << " us" << endl;

    return numMistakes;
}
