# EdgeHierarchies

To compile:

    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make benchmark
To run the benchmark program in its default configuration as used in the paper:

    app/benchmark [inputGraph] --rebuild --useCH -q 100000 --DFSPreOrder --EHBackwardStalling --partialStallingPercent -2
Where `[inputGraph]` is a directed, weighted graph in the DIMACS format.

To add turn penalties, add `-t -u [UTurnCosts]` to the command line parameters, where `[UTurnCosts]` are the costs for taking a U-turn.

## License
Our code is released under the MIT License, but external libraries might be under different licenses. See the respective directories under the `extern` or `googletest` directory.
