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
#include <random>
#include <fstream>

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
#include "edgeHierarchyWriter.h"
#include "edgeHierarchyReader.h"
#include "edgeRanking/shortcutCountingRoundsEdgeRanker.h"
#include "edgeRanking/shortcutCountingSortingRoundsEdgeRanker.h"
#include "edgeRanking/levelShortcutsHopsEdgeRanker.h"

bool fileExists (const std::string& name) {
    ifstream f(name.c_str());
    return f.good();
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

template<class EdgeRanker>
void buildAndWriteEdgeHierarchy(EdgeHierarchyGraph &g, std::string edgeHierarchyFilename) {
    EdgeHierarchyQuery query(g);

    EdgeHierarchyConstruction<EdgeRanker> construction(g, query);

    auto start = chrono::steady_clock::now();
    construction.run();
	auto end = chrono::steady_clock::now();

	cout << "EH Construction took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    cout << "Distance in Query graph was equal to removed path " << numEquals << " times" <<endl;

    cout << "Writing Edge Hierarchy to " << edgeHierarchyFilename <<endl;

    start = chrono::steady_clock::now();
    writeEdgeHierarchy(edgeHierarchyFilename, g);
	end = chrono::steady_clock::now();

	cout << "Writing EH took "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;
}

#define INVALID_QUERY_DATA std::numeric_limits<unsigned>::max()

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

std::vector<DijkstraRankRunningtime> GenerateRandomQueries(unsigned numQueries, int seed, EdgeHierarchyGraph &g) {

    std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> dist(0, g.getNumberOfNodes()-1);

    std::vector<DijkstraRankRunningtime> result;

    for(unsigned i=0; i<numQueries; ++i){

        unsigned source_node = dist(gen);
        unsigned target_node = dist(gen);

        result.push_back({source_node, target_node, INVALID_QUERY_DATA, INVALID_QUERY_DATA, -1, -1, -1, -1});
    }

    return result;
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

    bool addTurnCosts = false;
    cp.add_bool ('t', "turnCosts", addTurnCosts,
                 "If this flag is set, turn costs are added to the input graph.");

    bool dijkstraRank = false;
    cp.add_bool ('d', "dijkstraRank", dijkstraRank,
                 "If this flag is set, queries are generated for dijkstra ranks of powers of two with numQueries source vertices.");

    // process command line
    if (!cp.process(argc, argv))
        return -1; // some error occurred and help was always written to user.
    // output for debugging
    cp.print_result();

    std::string edgeHierarchyFilename = filename;
    if(addTurnCosts) {
        edgeHierarchyFilename += "Turncosts";
    }
    edgeHierarchyFilename += "ShortcutCountingRoundsEdgeRanker";
    edgeHierarchyFilename += ".eh";


    std::string contractionHierarchyFilename = filename;
    if(addTurnCosts) {
        contractionHierarchyFilename += "Turncosts";
    }
    contractionHierarchyFilename += ".ch";
    EdgeHierarchyGraph g(0);

    if(fileExists(edgeHierarchyFilename) && fileExists(contractionHierarchyFilename)) {
        std::cout << "Skip reading graph file because both EH and CH are already on disk" << std::endl;
    }
    else {
        auto start = chrono::steady_clock::now();
        g = readGraphDimacs(filename);
        auto end = chrono::steady_clock::now();

        cout << "Reading input graph took "
             << chrono::duration_cast<chrono::milliseconds>(end - start).count()
             << " ms" << endl;

        cout << "Input graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;

        if(addTurnCosts){
            start = chrono::steady_clock::now();
            g = g.getTurnCostGraph();
            end = chrono::steady_clock::now();

            cout << "Adding turn costs took "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms" << endl;

            cout << "Turn cost graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;
        }
    }

    RoutingKit::ContractionHierarchy ch;
    if(fileExists(contractionHierarchyFilename)) {
        std::cout << "Contraction Hierarchy already stored in file. Loading it..." << std::endl;
        ch = RoutingKit::ContractionHierarchy::load_file(contractionHierarchyFilename);
    }
    else {
        std::cout << "Building Contraction Hierarchy..." << std::endl;
        auto start = chrono::steady_clock::now();
        ch = getCHFromGraph(g);
        auto end = chrono::steady_clock::now();

        cout << "CH Construction took "
             << chrono::duration_cast<chrono::milliseconds>(end - start).count()
             << " ms" << endl;
        ch.save_file(contractionHierarchyFilename);
    }

    RoutingKit::ContractionHierarchyQuery chQuery(ch);

    cout << "CH has " << ch.forward.first_out.back() + ch.backward.first_out.back() << " edges" << endl;


    if(fileExists(edgeHierarchyFilename)) {
        std::cout << "Edge Hierarchy already stored in file. Loading it..." << std::endl;
        g = readEdgeHierarchy(edgeHierarchyFilename);
    }
    else {
        std::cout << "Building Edge Hierarchy..." << std::endl;
        buildAndWriteEdgeHierarchy<ShortcutCountingRoundsEdgeRanker>(g, edgeHierarchyFilename);
    }
    g.sortEdges();
    EdgeHierarchyGraphQueryOnly newG = g.getDFSOrderGraph<EdgeHierarchyGraphQueryOnly>();
    EdgeHierarchyQueryOnly newQuery = EdgeHierarchyQueryOnly(newG);
    newG.makeConsecutive();

    cout << "Edge hierarchy graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;

    cout << "DFS ordered edge hierarchy graph has " << newG.getNumberOfNodes() << " vertices and " << newG.getNumberOfEdges() << " edges" << endl;

    std::vector<DijkstraRankRunningtime> queries;
    if(dijkstraRank) {
        queries = GenerateDijkstraRankQueries(numQueries, seed, g);
    } else {
        queries = GenerateRandomQueries(numQueries, seed, g);
    }

    int numMistakes = 0;
    int numCorrect = 0;
    for(auto &generatedQuery: queries) {
        NODE_T u = generatedQuery.source;
        NODE_T v = generatedQuery.target;

        EDGEWEIGHT_T distance = newQuery.getDistance(u, v);

        chQuery.reset().add_source(u).add_target(v).run();
        auto chDistance = chQuery.get_distance();

        if(generatedQuery.distance == INVALID_QUERY_DATA) {
            generatedQuery.distance = chDistance;
        }

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

    cout << numMistakes << " out of " << numQueries << " WRONG!!!" << endl;

    cout << numCorrect << " out of " << numQueries << " CORRECT!" << endl;

    cout << "Done checking. Measuring time..." << endl;



    newQuery.resetCounters();
    auto start = chrono::steady_clock::now();
    for(auto &generatedQuery: queries) {
        NODE_T u = generatedQuery.source;
        NODE_T v = generatedQuery.target;

        if(dijkstraRank) {
            newQuery.resetCounters();
        }
        auto queryStart = chrono::high_resolution_clock::now();
        EDGEWEIGHT_T distance = newQuery.getDistance(u, v);
        // EDGEWEIGHT_T distance = query.getDistance(u, v);
        (void) distance;
        auto queryEnd = chrono::high_resolution_clock::now();
        if(dijkstraRank) {
            generatedQuery.timeEH = chrono::duration_cast<chrono::nanoseconds>(queryEnd - queryStart).count();
            generatedQuery.verticesSettledEH = newQuery.numVerticesSettled;
            generatedQuery.edgesRelaxedEH = newQuery.numEdgesRelaxed;
        }
    }
	auto end = chrono::steady_clock::now();

    if(!dijkstraRank) {
        cout << "Average query time (EH): "
             << chrono::duration_cast<chrono::microseconds>(end - start).count() / queries.size()
             << " us" << endl;
        cout << "Average number of vertices settled (EH): "
             << newQuery.numVerticesSettled/queries.size()
             << endl;
        cout << "Average number of edges relaxed (EH): "
             << newQuery.numEdgesRelaxed/queries.size()
             << endl;
    }

    chQuery.resetCounters();
    start = chrono::steady_clock::now();
    for(auto &generatedQuery: queries) {
        NODE_T u = generatedQuery.source;
        NODE_T v = generatedQuery.target;

        if(dijkstraRank) {
            chQuery.resetCounters();
        }
        auto queryStart = chrono::high_resolution_clock::now();
        chQuery.reset().add_source(u).add_target(v).run();
        auto chDistance = chQuery.get_distance();
        (void) chDistance;
        auto queryEnd = chrono::high_resolution_clock::now();
        if(dijkstraRank) {
            generatedQuery.timeCH = chrono::duration_cast<chrono::nanoseconds>(queryEnd - queryStart).count();
            generatedQuery.verticesSettledCH = chQuery.getNumVerticesSettled();
            generatedQuery.edgesRelaxedCH = chQuery.getNumEdgesRelaxed();
        }
    }
    end = chrono::steady_clock::now();

    if(!dijkstraRank) {
        cout << "Average query time (CH): "
             << chrono::duration_cast<chrono::microseconds>(end - start).count() / numQueries
             << " us" << endl;
        chQuery.printCounters(numQueries);
    }

    if(dijkstraRank) {
        std::cout << "Format: rank time vertices edges" << std::endl;
        for(auto &generatedQuery: queries) {
            unsigned rank = generatedQuery.rank;
            int timeEH = generatedQuery.timeEH;
            int numVerticesSettledEH = generatedQuery.verticesSettledEH;
            int numEdgesRelaxedEH = generatedQuery.edgesRelaxedEH;
            int timeCH = generatedQuery.timeCH;
            int numVerticesSettledCH = generatedQuery.verticesSettledCH;
            int numEdgesRelaxedCH = generatedQuery.edgesRelaxedCH;

            std::cout << "result EH: " << rank << " " << timeEH << " " << numVerticesSettledEH << " " << numEdgesRelaxedEH << std::endl;
            std::cout << "result CH: " << rank << " " << timeCH << " " << numVerticesSettledCH << " " << numEdgesRelaxedCH << std::endl;
        }
    }

    return numMistakes;
}
