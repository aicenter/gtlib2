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

If you want to use LP solvers (currently CPLEX/GUROBI/GLPK is supported), you have to make sure they are properly installed on your system.

Copy the file `example_BuildConfiguration.txt` to `BuildConfiguration.txt` and optionally enable the solver you'd like to use (set it `ON`). 

Follow the standard cmake build:

```
cd <project directory>
mkdir build && cd build
cmake ..
make
```

Please note there are two ways of building up the project: in _debug_ or _release_. Release uses additional compiler optimizations and runs faster, but takes longer time to build.

If you want to change your configuration later, you will have to reset CMake cache and rebuild the project:
- In CLion: `Tools -> CMake -> Reset Cache and Reload Project`

# High-level overview

Take a look at `base/` folder, which contains a number of abstract classes which are well documented. 

Game is modeled as a (possibly cyclic) graph whose nodes represent state of the game. The edges or transitions from state to state happen only when the actual game state has changed -- players might play actions, which do not change the state! Additionally, chance is encoded by *stochastic* transitions, i.e. the outcome of player's actions (in general) is not deterministic. 

Each edge has an outcome assigned to it: 
- the new state,
- vector of observations, and
- rewards for each player.

Extensive form game is built on top of this graph. There are some major differences to classical EFG definition in literature:
- Chance events are not encoded as a distinct histories: a single action can lead to multiple histories with some probability.
- Information sets are represented using action-observation history (AOH)
- Utilities in the leaves are distributed along the (root -> leaf) trajectory as rewards.

# Contributing

Please check the [CONTRIBUTING.md](CONTRIBUTING.md) file to understand how to contribute code to this repository. 
