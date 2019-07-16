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
#include <pthread.h>

#include <tlx/cmdline_parser.hpp>

#include <routingkit/contraction_hierarchy.h>
#include <routingkit/vector_io.h>
#include <routingkit/dijkstra.h>
#include <routingkit/inverse_vector.h>

#include <papi.hpp>

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

void pin_to_core(size_t core)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

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

template<bool EHForwardStalling, bool EHBackwardStalling, bool CHStallOnDemand, bool minimalSearchSpace>
int benchmark(bool dijkstraRank, bool test, EdgeHierarchyGraphQueryOnly &ehGraph, RoutingKit::ContractionHierarchyQuery &chQuery, std::vector<DijkstraRankRunningtime> &queries) {
    EdgeHierarchyQueryOnly<EHForwardStalling, EHBackwardStalling, minimalSearchSpace> newQuery = EdgeHierarchyQueryOnly<EHForwardStalling, EHBackwardStalling, minimalSearchSpace>(ehGraph);


    int numMistakes = 0;
    int numCorrect = 0;
    if(test) {
        for(auto &generatedQuery: queries) {
            NODE_T u = generatedQuery.source;
            NODE_T v = generatedQuery.target;

            EDGEWEIGHT_T distance = newQuery.getDistance(u, v);

            chQuery.reset().add_source(u).add_target(v).run<CHStallOnDemand, minimalSearchSpace>();
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

        cout << numMistakes << " out of " << queries.size() << " WRONG!!!" << endl;

        cout << numCorrect << " out of " << queries.size() << " CORRECT!" << endl;

        cout << "Done checking. Measuring time..." << endl;
    }



    int numVerticesSettledWithActualDistance = 0;
    newQuery.resetCounters();
    auto cache_counter_eh = common::papi_cache();
    auto start = chrono::steady_clock::now();
    for(auto &generatedQuery: queries) {
        NODE_T u = generatedQuery.source;
        NODE_T v = generatedQuery.target;

        if(dijkstraRank) {
            newQuery.resetCounters();
        }
        auto queryStart = chrono::high_resolution_clock::now();
        EDGEWEIGHT_T distance = newQuery.getDistance(u, v);
        (void) distance;
        auto queryEnd = chrono::high_resolution_clock::now();
        if(dijkstraRank) {
            generatedQuery.timeEH = chrono::duration_cast<chrono::nanoseconds>(queryEnd - queryStart).count();
            generatedQuery.verticesSettledEH = newQuery.numVerticesSettled;
            generatedQuery.edgesRelaxedEH = newQuery.numEdgesRelaxed + newQuery.numEdgesLookedAtForStalling;
        }

        if constexpr(minimalSearchSpace){
            for(std::pair<NODE_T, EDGEWEIGHT_T> nodeDistancePair: newQuery.verticesSettledForward) {
                chQuery.reset().add_source(u).add_target(nodeDistancePair.first).run();
                unsigned actualDistance = chQuery.get_distance();
                if(actualDistance == nodeDistancePair.second) {
                    ++numVerticesSettledWithActualDistance;
                }
                if(actualDistance > nodeDistancePair.second) {
                    std::cout << "IMPOSSIBLE!!!" <<std::endl;
                }
            }
            for(std::pair<NODE_T, EDGEWEIGHT_T> nodeDistancePair: newQuery.verticesSettledBackward) {
                chQuery.reset().add_source(nodeDistancePair.first).add_target(v).run();
                unsigned actualDistance = chQuery.get_distance();
                if(actualDistance == nodeDistancePair.second) {
                    ++numVerticesSettledWithActualDistance;
                }
                if(actualDistance > nodeDistancePair.second) {
                    std::cout << "IMPOSSIBLE!!!" <<std::endl;
                }
            }
        }
    }
    cache_counter_eh.stop();
    std::cout << cache_counter_eh.result() << std::endl;
	auto end = chrono::steady_clock::now();

    if(!dijkstraRank) {
        cout << "Average query time (EH): "
             << chrono::duration_cast<chrono::microseconds>(end - start).count() / queries.size()
             << " us" << endl;
        if constexpr(minimalSearchSpace) {
                cout << "MINIMAL average number of vertices settled (EH): "
                     << numVerticesSettledWithActualDistance/queries.size()
                     << endl;
            }
        cout << "Average number of vertices settled (EH): "
             << newQuery.numVerticesSettled/queries.size()
             << endl;
        cout << "Average number of edges relaxed (EH): "
             << newQuery.numEdgesRelaxed/queries.size()
             << endl;
        cout << "Average number of edges looked at for stalling (EH): "
             << newQuery.numEdgesLookedAtForStalling/queries.size()
             << endl;
    }

    numVerticesSettledWithActualDistance = 0;
    chQuery.resetCounters();
    start = chrono::steady_clock::now();
    auto cache_counter_ch = common::papi_cache();
    for(auto &generatedQuery: queries) {
        NODE_T u = generatedQuery.source;
        NODE_T v = generatedQuery.target;

        if(dijkstraRank) {
            chQuery.resetCounters();
        }
        auto queryStart = chrono::high_resolution_clock::now();
        chQuery.reset().add_source(u).add_target(v).run<CHStallOnDemand, minimalSearchSpace>();
        auto chDistance = chQuery.get_distance();
        (void) chDistance;
        auto queryEnd = chrono::high_resolution_clock::now();
        if(dijkstraRank) {
            generatedQuery.timeCH = chrono::duration_cast<chrono::nanoseconds>(queryEnd - queryStart).count();
            generatedQuery.verticesSettledCH = chQuery.getNumVerticesSettled();
            generatedQuery.edgesRelaxedCH = chQuery.getNumEdgesRelaxed() + chQuery.getNumEdgesLookedAtForStalling();
        }
        if constexpr(minimalSearchSpace){
            auto verticesSettledForward = chQuery.getVerticesSettledForward();
            auto verticesSettledBackward = chQuery.getVerticesSettledBackward();

            for(const std::pair<unsigned, unsigned> &nodeDistancePair: verticesSettledForward) {
                unsigned actualDistance = newQuery.getDistance(u, nodeDistancePair. first);
                if(actualDistance == nodeDistancePair.second) {
                    ++numVerticesSettledWithActualDistance;
                }
                if(actualDistance > nodeDistancePair.second) {
                    std::cout << "IMPOSSIBLE!!! Distance from " << u << " to " << nodeDistancePair.first << " is " << actualDistance << " but was settled at " << nodeDistancePair.second <<std::endl;
                }
            }
            for(const std::pair<unsigned, unsigned> &nodeDistancePair: verticesSettledBackward) {
                unsigned actualDistance = newQuery.getDistance(nodeDistancePair.first, v);
                if(actualDistance == nodeDistancePair.second) {
                    ++numVerticesSettledWithActualDistance;
                }
                if(actualDistance > nodeDistancePair.second) {
                    std::cout << "IMPOSSIBLE!!! Distance from " << nodeDistancePair.first << " to " << v << " is " << actualDistance << " but was settled at " << nodeDistancePair.second <<std::endl;
                }
            }
        }
    }
    cache_counter_ch.stop();
    std::cout << cache_counter_ch.result() << std::endl;
    end = chrono::steady_clock::now();

    if(!dijkstraRank) {
        cout << "Average query time (CH): "
             << chrono::duration_cast<chrono::microseconds>(end - start).count() / queries.size()
             << " us" << endl;
        if constexpr(minimalSearchSpace) {
            cout << "MINIMAL average number of vertices settled (CH): "
                 << numVerticesSettledWithActualDistance/queries.size()
                 << endl;
        }
        chQuery.printCounters(queries.size());
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

template<bool EHForwardStalling, bool EHBackwardStalling, bool CHStallOnDemand>
int benchmark(bool minimalSearchSpace, bool dijkstraRank, bool test, EdgeHierarchyGraphQueryOnly &ehGraph, RoutingKit::ContractionHierarchyQuery &chQuery, std::vector<DijkstraRankRunningtime> &queries) {
    if(minimalSearchSpace)
        return benchmark<EHForwardStalling, EHBackwardStalling, CHStallOnDemand, true>(dijkstraRank, test, ehGraph, chQuery, queries);
    else
        return benchmark<EHForwardStalling, EHBackwardStalling, CHStallOnDemand, false>(dijkstraRank, test, ehGraph, chQuery, queries);
}

template<bool EHForwardStalling, bool EHBackwardStalling>
int benchmark(bool CHStallOnDemand, bool minimalSearchSpace, bool dijkstraRank, bool test, EdgeHierarchyGraphQueryOnly &ehGraph, RoutingKit::ContractionHierarchyQuery &chQuery, std::vector<DijkstraRankRunningtime> &queries) {
    if(CHStallOnDemand)
        return benchmark<EHForwardStalling, EHBackwardStalling, true>(minimalSearchSpace, dijkstraRank, test, ehGraph, chQuery, queries);
    else
        return benchmark<EHForwardStalling, EHBackwardStalling, false>(minimalSearchSpace, dijkstraRank, test, ehGraph, chQuery, queries);
}

template<bool EHForwardStalling>
int benchmark(bool EHBackwardStalling, bool CHStallOnDemand, bool minimalSearchSpace, bool dijkstraRank, bool test, EdgeHierarchyGraphQueryOnly &ehGraph, RoutingKit::ContractionHierarchyQuery &chQuery, std::vector<DijkstraRankRunningtime> &queries) {
    if(EHBackwardStalling)
        return benchmark<EHForwardStalling, true>(CHStallOnDemand, minimalSearchSpace, dijkstraRank, test, ehGraph, chQuery, queries);
    else
        return benchmark<EHForwardStalling, false>(CHStallOnDemand, minimalSearchSpace, dijkstraRank, test, ehGraph, chQuery, queries);
}

int benchmark(bool EHForwardStalling, bool EHBackwardStalling, bool CHStallOnDemand, bool minimalSearchSpace, bool dijkstraRank, bool test, EdgeHierarchyGraphQueryOnly &ehGraph, RoutingKit::ContractionHierarchyQuery &chQuery, std::vector<DijkstraRankRunningtime> &queries) {
    if(EHForwardStalling)
        return benchmark<true>(EHBackwardStalling, CHStallOnDemand, minimalSearchSpace, dijkstraRank, test, ehGraph, chQuery, queries);
    else
        return benchmark<false>(EHBackwardStalling, CHStallOnDemand, minimalSearchSpace, dijkstraRank, test, ehGraph, chQuery, queries);
}


int main(int argc, char* argv[]) {
    pin_to_core(0);
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

    unsigned uTurnCost = 1000;
    cp.add_unsigned('u', "uTurnCost", uTurnCost,
                    "Cost to be added for u-turns (only has effect when turn costs are activated).");

    bool useCHForEHConstruction = false;
    cp.add_bool ("useCH", useCHForEHConstruction,
                 "If this flag is set, CH queries will be used during EH construction");

    bool DFSPreOrder = false;
    cp.add_bool ("DFSPreOrder", DFSPreOrder,
                 "If this flag is set, DFS ordering will use pre order instead of post order");

    bool CHOrder = false;
    cp.add_bool ("CHOrder", CHOrder,
                 "If this flag is set, EHs will use the node ordering from CHs");

    bool dijkstraRank = false;
    cp.add_bool ('d', "dijkstraRank", dijkstraRank,
                 "If this flag is set, queries are generated for dijkstra ranks of powers of two with numQueries source vertices.");

    bool EHForwardStalling = false;
    cp.add_bool ("EHForwardStalling", EHForwardStalling,
                 "If this flag is set, Edge Hierarchy queries will use forward stalling");

    bool EHBackwardStalling = false;
    cp.add_bool ("EHBackwardStalling", EHBackwardStalling,
                 "If this flag is set, Edge Hierarchy queries will use backward stalling");

    bool CHNoStallOnDemand = false;
    cp.add_bool ("CHNoStallOnDemand", CHNoStallOnDemand,
                 "If this flag is set, Contraction Hierarchy queries will NOT use stall on demand");

    bool minimalSearchSpace = false;
    cp.add_bool ("minimalSearchSpace", minimalSearchSpace,
                 "If this flag is set, the minimal search space will be calculated");

    bool test = false;
    cp.add_bool ("test", test,
                 "If this flag is set, correctness will be checked");

    bool rebuild = false;
    cp.add_bool ("rebuild", rebuild,
                 "If this flag is set, CH and EH are rebuilt");

    // process command line
    if (!cp.process(argc, argv))
        return -1; // some error occurred and help was always written to user.
    // output for debugging
    cp.print_result();

    bool CHStallOnDemand = !CHNoStallOnDemand;

    shortcutHelperUseCH = useCHForEHConstruction;

    std::string edgeHierarchyFilename = filename;
    if(addTurnCosts) {
        edgeHierarchyFilename += "Turncosts" + std::to_string(uTurnCost);
    }
    edgeHierarchyFilename += "ShortcutCountingRoundsEdgeRanker";
    if(useCHForEHConstruction) {
        edgeHierarchyFilename += "CHForConstruction";
    }
    edgeHierarchyFilename += ".eh";


    std::string contractionHierarchyFilename = filename;
    if(addTurnCosts) {
        contractionHierarchyFilename += "Turncosts" + std::to_string(uTurnCost);
    }
    contractionHierarchyFilename += ".ch";
    EdgeHierarchyGraph g(0);

    std::vector<DijkstraRankRunningtime> queries;
    if(!rebuild && fileExists(edgeHierarchyFilename) && fileExists(contractionHierarchyFilename) && !dijkstraRank) {
        std::cout << "Skip reading graph file because both EH and CH are already on disk and dijkstraRank is deactivated" << std::endl;
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
            g = g.getTurnCostGraph(uTurnCost);
            end = chrono::steady_clock::now();

            cout << "Adding turn costs took "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms" << endl;

            cout << "Turn cost graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;
        }

        if(dijkstraRank) {
            queries = GenerateDijkstraRankQueries(numQueries, seed, g);
        }
    }


    RoutingKit::ContractionHierarchy ch;
    if(!rebuild && fileExists(contractionHierarchyFilename)) {
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

    shortcutHelperChQuery = chQuery;

    if(!rebuild && fileExists(edgeHierarchyFilename)) {
        std::cout << "Edge Hierarchy already stored in file. Loading it..." << std::endl;
        g = readEdgeHierarchy(edgeHierarchyFilename);
    }
    else {
        std::cout << "Building Edge Hierarchy..." << std::endl;
        buildAndWriteEdgeHierarchy<ShortcutCountingRoundsEdgeRanker>(g, edgeHierarchyFilename);
    }
    g.sortEdges();
    cout << "Edge hierarchy graph has " << g.getNumberOfNodes() << " vertices and " << g.getNumberOfEdges() << " edges" << endl;
    EdgeHierarchyGraphQueryOnly newG(0);
    if(CHOrder) {
        newG = g.getReorderedGraph<EdgeHierarchyGraphQueryOnly>(ch.rank);
        cout << "Reordered edge hierarchy graph has " << newG.getNumberOfNodes() << " vertices and " << newG.getNumberOfEdges() << " edges" << endl;
    }
    else{
        if(DFSPreOrder) {
            newG = g.getDFSOrderGraph<EdgeHierarchyGraphQueryOnly, true>();
        }
        else {
            newG = g.getDFSOrderGraph<EdgeHierarchyGraphQueryOnly, false>();
        }
        cout << "DFS ordered edge hierarchy graph has " << newG.getNumberOfNodes() << " vertices and " << newG.getNumberOfEdges() << " edges" << endl;
    }
    newG.makeConsecutive();

    if(!dijkstraRank) {
        queries = GenerateRandomQueries(numQueries, seed, g);
    }


    return benchmark(EHForwardStalling, EHBackwardStalling, CHStallOnDemand, minimalSearchSpace, dijkstraRank, test, newG, chQuery, queries);

}
