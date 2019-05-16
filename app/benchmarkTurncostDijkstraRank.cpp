/*******************************************************************************
 * app/benchmarkTurncostDijkstraRank.cpp
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
#include <random>

#include <tlx/cmdline_parser.hpp>

#include <routingkit/contraction_hierarchy.h>
#include <routingkit/vector_io.h>
#include <routingkit/dijkstra.h>
#include <routingkit/inverse_vector.h>

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
#include "edgeRanking/shortcutsHopsRoundsEdgeRanker.h"

struct DijkstraRankRunningtime {
    unsigned source;
    unsigned target;
    unsigned rank;
    unsigned distance;
    int timeEH;
    int verticesSettledEH;
    int edgesRelaxedEH;
    int timeCH;
    int verticesSettledCH;
    int edgesRelaxedCH;
};


std::vector<DijkstraRankRunningtime> GenerateDijkstraRankQueries(unsigned numSourceNodes, int seed, EdgeHierarchyGraph &g) {
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> dist(0, g.getNumberOfNodes()-1);

    std::vector<unsigned> tails, heads, weights, first_out;

    g.forAllNodes([&] (NODE_T tail) {
            first_out.push_back(tails.size());
            g.forAllNeighborsOut(tail, [&] (NODE_T head, EDGEWEIGHT_T weight) {
                    tails.push_back(tail);
                    heads.push_back(head);
                    weights.push_back(weight);
                });
        });
    RoutingKit::Dijkstra dij(first_out, tails, heads);

    std::vector<DijkstraRankRunningtime> result;

    for(unsigned i=0; i<numSourceNodes; ++i){

        unsigned source_node = dist(gen);
        unsigned r = 0;
        unsigned n = 0;

        dij.reset().add_source(source_node);

        while(!dij.is_finished()){
            auto x = dij.settle(RoutingKit::ScalarGetWeight(weights));
            ++n;
            if(n == (1u << r)){

                if(r > 5){
                    result.push_back({source_node, x.node, r, x.distance, -1, -1, -1, -1});
                }

                ++r;
            }
        }
		}

    return result;
}

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

    auto start = chrono::high_resolution_clock::now();
    EdgeHierarchyGraph inputG = readGraphDimacs(filename);
	auto end = chrono::high_resolution_clock::now();

	cout << "Reading input graph took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    cout << "Input graph has " << inputG.getNumberOfNodes() << " vertices and " << inputG.getNumberOfEdges() << " edges" << endl;

    start = chrono::high_resolution_clock::now();
    EdgeHierarchyGraph g = inputG.getTurnCostGraph();
    end = chrono::high_resolution_clock::now();

	cout << "Adding turn costs took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    // start = chrono::high_resolution_clock::now();
    // g = g.getDFSOrderGraph();
	// end = chrono::high_resolution_clock::now();

	// cout << "Sorting graph took "
    //      << chrono::duration_cast<chrono::milliseconds>(end - start).count()
    //      << " ms" << endl;

    cout << "Turn cost graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;

    start = chrono::high_resolution_clock::now();
    auto ch = getCHFromGraph(g);
    RoutingKit::ContractionHierarchyQuery chQuery(ch);
    end = chrono::high_resolution_clock::now();

    cout << "CH Construction took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;


    cout << "CH has " << ch.forward.first_out.back() + ch.backward.first_out.back() << " edges" << endl;



    EdgeHierarchyQuery query(g);

    EdgeHierarchyGraph originalGraph(g);
    EdgeHierarchyQuery originalGraphQuery(originalGraph);

    // EdgeHierarchyConstruction<LevelShortcutsHopsEdgeRanker> construction(g, query);
    EdgeHierarchyConstruction<ShortcutCountingRoundsEdgeRanker> construction(g, query);
    // EdgeHierarchyConstruction<ShortcutsHopsRoundsEdgeRanker> construction(g, query);
    // EdgeHierarchyConstruction<ShortcutCountingSortingRoundsEdgeRanker> construction(g, query);

    start = chrono::high_resolution_clock::now();
    construction.run();
    g.sortEdges();
    EdgeHierarchyGraphQueryOnly newG = g.getDFSOrderGraph<EdgeHierarchyGraphQueryOnly>();
    EdgeHierarchyQueryOnly newQuery = EdgeHierarchyQueryOnly(newG);
    newG.makeConsecutive();
    // EdgeHierarchyGraph newG = g.getDFSOrderGraph<EdgeHierarchyGraph>();
    // EdgeHierarchyQuery newQuery = EdgeHierarchyQuery(newG);
    // newG.sortEdges();
	end = chrono::high_resolution_clock::now();

	cout << "EH Construction took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    // start = chrono::high_resolution_clock::now();
    // g = g.getDFSOrderGraph();
    // g.sortEdges();
	// end = chrono::high_resolution_clock::now();

	// cout << "Sorting graph took "
    //      << chrono::duration_cast<chrono::milliseconds>(end - start).count()
    //      << " ms" << endl;

    cout << "Edge hierarchy graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;

    cout << "Distance in Query graph was equal to removed path " << numEquals << " times" <<endl;

    cout << "Generating queries from " << numQueries << " sources" <<endl;
    start = chrono::high_resolution_clock::now();
    std::vector<DijkstraRankRunningtime> queries = GenerateDijkstraRankQueries(numQueries, seed, g);
	end = chrono::high_resolution_clock::now();
    cout << "Generated " << queries.size() << " queries" <<endl;
	cout << "Generating queries took "
         << chrono::duration_cast<chrono::seconds>(end - start).count()
         << " s" << endl;

    int numMistakes = 0;
    int numCorrect = 0;
    for(auto &generatedQuery: queries) {
        NODE_T u = generatedQuery.source;
        NODE_T v = generatedQuery.target;

        EDGEWEIGHT_T distance = newQuery.getDistance(u, v);

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

        if(generatedQuery.distance != distance) {
            cout << "EH: Wrong distance for " << u << " and " << v << ": " << distance << " (should be " << generatedQuery.distance << ")" << endl;
            numMistakes++;
        } else {
            numCorrect++;
        }

        if(generatedQuery.distance != chDistance) {
            cout << "CH: Wrong distance for " << u << " and " << v << ": " << chDistance << " (should be " << generatedQuery.distance << ")" << endl;
        } else {
        }
    }

    cout << numMistakes << " out of " << queries.size() << " WRONG!!!" << endl;

    cout << numCorrect << " out of " << queries.size() << " CORRECT!" << endl;

    cout << "Done checking. Measuring time..." << endl;


    srand (seed);

    // query.resetCounters();

    for(auto &generatedQuery: queries) {
        NODE_T u = generatedQuery.source;
        NODE_T v = generatedQuery.target;

        newQuery.resetCounters();
        start = chrono::high_resolution_clock::now();
        EDGEWEIGHT_T distance = newQuery.getDistance(u, v);
        // EDGEWEIGHT_T distance = query.getDistance(u, v);
        (void) distance;
        end = chrono::high_resolution_clock::now();
        generatedQuery.timeEH = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
        generatedQuery.verticesSettledEH = newQuery.numVerticesSettled;
        generatedQuery.edgesRelaxedEH = newQuery.numEdgesRelaxed;
    }

    srand (seed);

    for(auto &generatedQuery: queries) {
        NODE_T u = generatedQuery.source;
        NODE_T v = generatedQuery.target;

        chQuery.resetCounters();
        start = chrono::high_resolution_clock::now();
        chQuery.reset().add_source(u).add_target(v).run();
        auto chDistance = chQuery.get_distance();
        (void) chDistance;
        end = chrono::high_resolution_clock::now();
        generatedQuery.timeCH = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
        generatedQuery.verticesSettledCH = chQuery.getNumVerticesSettled();
        generatedQuery.edgesRelaxedCH = chQuery.getNumEdgesRelaxed();
    }



    std::cout << "Format: rank time vertices edges" << std::endl;
    for(auto &generatedQuery: queries) {
        unsigned rank = generatedQuery.rank;
        int timeEH = generatedQuery.timeEH;
        int numVerticesSettledEH = generatedQuery.verticesSettledEH;
        int numEdgesRelaxedEH = generatedQuery.edgesRelaxedEH;
        int timeCH = generatedQuery.timeCH;
        int numVerticesSettledCH = generatedQuery.verticesSettledCH;
        int numEdgesRelaxedCH = generatedQuery.verticesSettledCH;

        std::cout << "result EH: " << rank << " " << timeEH << " " << numVerticesSettledEH << " " << numEdgesRelaxedEH << std::endl;
        std::cout << "result CH: " << rank << " " << timeCH << " " << numVerticesSettledCH << " " << numEdgesRelaxedCH << std::endl;
    }

    return numMistakes;
}
