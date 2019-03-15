Game theoretic library developed at [AIC FEE CTU](http://aic.fel.cvut.cz/)

Currently contains implementations of:

- Domains:
    - Poker
    - Goofspiel
    - Kriegspiel
    - Matching pennies
    - Phantom Tic-Tac-Toe
    - Pursuit-evasion
- Algorithms:
    - Best response calculation
    - Counterfactual regret minimization (CFR)
    - Normal-form linear program
    - Tree walk
    - Utility calculation for given strategy

# Dependencies

Setup which was tested to work:

- Ubuntu 18.04
- g++ 7.4 installed
- cmake 3.10.2
- boost 1.65.1

You do not need to have CPLEX or GUROBI solvers installed, but the functionality of the library will be obviously more limited.

This project uses C++17. 

For ubuntu:

    # At the time of writing this, at most gcc 7.3 was available for Ubuntu 18.04  
    sudo add-apt-repository ppa:jonathonf/gcc-7.4
    sudo apt update
    
    sudo apt install build-essentials gcc-7 g++-7 cmake libboost-dev libboost-test-dev libboost-system-dev 

Notice that if you compiled the library with older GCC versions which may not work, you need to delete all the cache files before trying again with newer version.

# Build

Follow the standard cmake build:

```
cd <project directory>
mkdir build && cd build
cmake ..
make
```

# Contributing

Please check the [CONTRIBUTING.md](CONTRIBUTING.md) file to understand how to contribute code to this repository. 
