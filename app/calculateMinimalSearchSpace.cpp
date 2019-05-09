/*******************************************************************************
 * app/calculateMinimalSearchSpace.cpp
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
#include "edgeHierarchyConstruction.h"
#include "dimacsGraphReader.h"
#include "edgeRanking/shortcutCountingRoundsEdgeRanker.h"
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

    cp.set_description("Program to calculate the minimal possible search space for EdgeHierarchies");
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
    g = g.getTurnCostGraph();
    end = chrono::steady_clock::now();

	cout << "Adding turn costs took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    cout << "Turn cost graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;

    start = chrono::steady_clock::now();
    auto ch = getCHFromGraph(g);
    RoutingKit::ContractionHierarchyQuery chQuery(ch);
    end = chrono::steady_clock::now();

    cout << "CH Construction took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;


    cout << "CH has " << ch.forward.first_out.back() + ch.backward.first_out.back() << " edges" << endl;

    EdgeHierarchyQuery query(g);

    // EdgeHierarchyConstruction<LevelShortcutsHopsEdgeRanker> construction(g, query);
    EdgeHierarchyConstruction<ShortcutCountingRoundsEdgeRanker> construction(g, query);

    start = chrono::steady_clock::now();
    construction.run();
    g.sortEdges();
	end = chrono::steady_clock::now();

	cout << "EH Construction took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    cout << "Edge hierarchy graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;

    srand (seed);

    query.resetCounters();
    RoutingKit::ContractionHierarchyQuery chQuery2(ch);

    int numVerticesSettledWithActualDistance = 0;

    for(unsigned i = 0; i < numQueries; ++i) {
        NODE_T u = rand() % g.getNumberOfNodes();
        NODE_T v = rand() % g.getNumberOfNodes();

        EDGEWEIGHT_T distance = query.getDistance(u, v);
        (void) distance;

        for(std::pair<NODE_T, EDGEWEIGHT_T> nodeDistancePair: query.verticesSettledForward) {
            chQuery2.reset().add_source(u).add_target(nodeDistancePair.first).run();
            unsigned actualDistance = chQuery2.get_distance();
            if(actualDistance == nodeDistancePair.second) {
                ++numVerticesSettledWithActualDistance;
            }
            if(actualDistance > nodeDistancePair.second) {
                std::cout << "IMPOSSIBLE!!!" <<std::endl;
            }
        }
        for(std::pair<NODE_T, EDGEWEIGHT_T> nodeDistancePair: query.verticesSettledBackward) {
            chQuery2.reset().add_source(nodeDistancePair.first).add_target(v).run();
            unsigned actualDistance = chQuery2.get_distance();
            if(actualDistance == nodeDistancePair.second) {
                ++numVerticesSettledWithActualDistance;
            }
            if(actualDistance > nodeDistancePair.second) {
                std::cout << "IMPOSSIBLE!!!" <<std::endl;
            }
        }
    }

	cout << "MINIMAL average number of vertices settled (EH): "
         << numVerticesSettledWithActualDistance/numQueries
         << endl;
	cout << "Average number of vertices settled (EH): "
         << query.numVerticesSettled/numQueries
         << endl;
	cout << "Average number of edges relaxed (EH): "
         << query.numEdgesRelaxed/numQueries
         << endl;

    srand (seed);

    chQuery.resetCounters();
    numVerticesSettledWithActualDistance = 0;

    start = chrono::steady_clock::now();
    for(unsigned i = 0; i < numQueries; ++i) {
        NODE_T u = rand() % g.getNumberOfNodes();
        NODE_T v = rand() % g.getNumberOfNodes();

        chQuery.reset().add_source(u).add_target(v).run();
        auto chDistance = chQuery.get_distance();
        (void) chDistance;

        auto verticesSettledForward = chQuery.getVerticesSettledForward();
        auto verticesSettledBackward = chQuery.getVerticesSettledBackward();

        for(const std::pair<unsigned, unsigned> &nodeDistancePair: verticesSettledForward) {
            chQuery2.reset().add_source(u).add_target(nodeDistancePair.first).run();
            unsigned actualDistance = chQuery2.get_distance();
            if(actualDistance == nodeDistancePair.second) {
                ++numVerticesSettledWithActualDistance;
            }
            if(actualDistance > nodeDistancePair.second) {
                std::cout << "IMPOSSIBLE!!! Distance from " << u << " to " << nodeDistancePair.first << " is " << actualDistance << " but was settled at " << nodeDistancePair.second <<std::endl;
            }
        }
        for(const std::pair<unsigned, unsigned> &nodeDistancePair: verticesSettledBackward) {
            chQuery2.reset().add_source(nodeDistancePair.first).add_target(v).run();
            unsigned actualDistance = chQuery2.get_distance();
            if(actualDistance == nodeDistancePair.second) {
                ++numVerticesSettledWithActualDistance;
            }
            if(actualDistance > nodeDistancePair.second) {
                std::cout << "IMPOSSIBLE!!! Distance from " << nodeDistancePair.first << " to " << v << " is " << actualDistance << " but was settled at " << nodeDistancePair.second <<std::endl;
            }
        }
    }
    end = chrono::steady_clock::now();

	cout << "MINIMAL average number of vertices settled (CH): "
         << numVerticesSettledWithActualDistance/numQueries
         << endl;

    chQuery.resetCounters();
    numVerticesSettledWithActualDistance = 0;

    start = chrono::steady_clock::now();
    for(unsigned i = 0; i < numQueries; ++i) {
        NODE_T u = rand() % g.getNumberOfNodes();
        NODE_T v = rand() % g.getNumberOfNodes();

        chQuery.reset().add_source(u).add_target(v).run();
        auto chDistance = chQuery.get_distance();
        (void) chDistance;
    }
    end = chrono::steady_clock::now();
    chQuery.printCounters(numQueries);

    return 0;
}
