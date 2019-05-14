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
#include <cstdlib>

#include <tlx/cmdline_parser.hpp>

#include <routingkit/contraction_hierarchy.h>

#include "definitions.h"
#include "edgeHierarchyGraph.h"
#include "edgeHierarchyQuery.h"
#include "edgeHierarchyGraphQueryOnly.h"
#include "edgeHierarchyQueryOnly.h"
#include "edgeHierarchyConstruction.h"
#include "dimacsGraphReader.h"
#include "edgeRanking/shortcutCountingRoundsEdgeRanker.h"
#include "edgeRanking/shortcutCountingSortingRoundsEdgeRanker.h"
#include "edgeRanking/levelShortcutsHopsEdgeRanker.h"


RoutingKit::ContractionHierarchy getCHFromGraph(EdgeHierarchyGraph &g) {
    std::vector<unsigned> tails, heads, weights;

    g.forAllNodes([&] (NODE_T tail) {
       g.forAllNeighborsOut(tail, [&] (NODE_T head, EDGEWEIGHT_T weight) {
           tails.push_back(tail);
           heads.push_back(head);
           weights.push_back(weight);
       });
    });

    return RoutingKit::ContractionHierarchy::build(
            g.getNumberOfNodes(),
            tails, heads,
            weights
    );
}

int main(int argc, char* argv[]) {
    tlx::CmdlineParser cp;

    cp.set_description("Benchmark program for EdgeHierarchies");
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

    cout << "Input graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;

    start = chrono::steady_clock::now();
    auto ch = getCHFromGraph(g);
    RoutingKit::ContractionHierarchyQuery chQuery(ch);
    end = chrono::steady_clock::now();

    cout << "CH Construction took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;


    cout << "CH has " << ch.forward.first_out.back() + ch.backward.first_out.back() << " edges" << endl;



    EdgeHierarchyQuery query(g);

    EdgeHierarchyGraph originalGraph(g);
    EdgeHierarchyQuery originalGraphQuery(originalGraph);

    // EdgeHierarchyConstruction<LevelShortcutsHopsEdgeRanker> construction(g, query);
    EdgeHierarchyConstruction<ShortcutCountingRoundsEdgeRanker> construction(g, query);
    // EdgeHierarchyConstruction<ShortcutCountingSortingRoundsEdgeRanker> construction(g, query);

    start = chrono::steady_clock::now();
    construction.run();
    g.sortEdges();
    // EdgeHierarchyGraph newG = g;
    // EdgeHierarchyQuery newQuery = query;
    EdgeHierarchyGraphQueryOnly newG = g.getDFSOrderGraph<EdgeHierarchyGraphQueryOnly>();
    EdgeHierarchyQueryOnly newQuery = EdgeHierarchyQueryOnly(newG);
    newG.makeConsecutive();
	end = chrono::steady_clock::now();

	cout << "EH Construction took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    cout << "Edge hierarchy graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;

    cout << "DFS ordered edge hierarchy graph has " << newG.getNumberOfNodes() << " vertices and " << newG.getNumberOfEdges() << " edges" << endl;

    cout << "Distance in Query graph was equal to removed path " << numEquals << " times" <<endl;

    srand (seed);
    int numMistakes = 0;
    int numCorrect = 0;
    for(unsigned i = 0; i < numQueries; ++i) {
        NODE_T u = rand() % g.getNumberOfNodes();
        NODE_T v = rand() % g.getNumberOfNodes();

        EDGEWEIGHT_T distance = query.getDistance(u, v);

//        EDGEWEIGHT_T originalGraphDistance = originalGraphQuery.getDistance(u, v);

        chQuery.reset().add_source(u).add_target(v).run();
        auto chDistance = chQuery.get_distance();

//        if(distance != originalGraphDistance) {
//
//            cout << "Wrong distance for " << u << " and " << v << ": " << distance << " (should be " << originalGraphDistance << ")" << endl;
//            numMistakes++;
//        } else {
//            numCorrect++;
//        }

        if(chDistance != distance) {
            cout << "Wrong distance for " << u << " and " << v << ": " << distance << " (should be " << chDistance << ")" << endl;
            numMistakes++;
        } else {
            numCorrect++;
        }
    }

    cout << numMistakes << " out of " << numQueries << " WRONG!!!" << endl;

    cout << numCorrect << " out of " << numQueries << " CORRECT!" << endl;

    cout << "Done checking. Measuring time..." << endl;


    srand (seed);

    // query.resetCounters();
    newQuery.resetCounters();

    start = chrono::steady_clock::now();
    for(unsigned i = 0; i < numQueries; ++i) {
        NODE_T u = rand() % g.getNumberOfNodes();
        NODE_T v = rand() % g.getNumberOfNodes();

        EDGEWEIGHT_T distance = newQuery.getDistance(u, v);
        // EDGEWEIGHT_T distance = query.getDistance(u, v);
        (void) distance;
        // auto now = chrono::steady_clock::now();
        // cout << "Average query time (EH): "
        //      << chrono::duration_cast<chrono::microseconds>(now - start).count() / (i + 1)
        //      << " us" << endl;
        // cout << "Average number of vertices settled (EH): "
        //      << newQuery.numVerticesSettled/numQueries
        //      << endl;
        // cout << "Average number of edges relaxed (EH): "
        //      << newQuery.numEdgesRelaxed/numQueries
        //      << endl;
    }
	end = chrono::steady_clock::now();

	cout << "Average query time (EH): "
         << chrono::duration_cast<chrono::microseconds>(end - start).count() / numQueries
         << " us" << endl;
	// cout << "Average number of vertices settled (EH): "
    //      << query.numVerticesSettled/numQueries
    //      << endl;
	// cout << "Average number of edges relaxed (EH): "
    //      << query.numEdgesRelaxed/numQueries
    //      << endl;
    cout << "Average number of vertices settled (EH): "
         << newQuery.numVerticesSettled/numQueries
         << endl;
    cout << "Average number of edges relaxed (EH): "
         << newQuery.numEdgesRelaxed/numQueries
         << endl;





    srand (seed);

    chQuery.resetCounters();
    start = chrono::steady_clock::now();
    for(unsigned i = 0; i < numQueries; ++i) {
        NODE_T u = rand() % g.getNumberOfNodes();
        NODE_T v = rand() % g.getNumberOfNodes();

        chQuery.reset().add_source(u).add_target(v).run();
        auto chDistance = chQuery.get_distance();
        (void) chDistance;
    }
    end = chrono::steady_clock::now();

    cout << "Average query time (CH): "
         << chrono::duration_cast<chrono::microseconds>(end - start).count() / numQueries
         << " us" << endl;
    chQuery.printCounters(numQueries);

    return numMistakes;
}
