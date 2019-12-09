/*

    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague
    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GTLIB2_CPW_ISMCTS_H
#define GTLIB2_CPW_ISMCTS_H

#include "algorithms/MCTS/ISMCTS.h"
#include "algorithms/common.h"

/**
 * CPW_ISMCTS (Cowling, Powley and Whitehouse Information Set Monte Carlo Tree Search) algorithm.
 * Unlike the ISMCTS, iterates the tree down from the current infoset,
 * but requires IS -> nodes map to be stored.
 * Using belief (reach probabilities over histories in current infoset) makes this algorithm
 * even more consistent against non-random strategies.
 */
namespace GTLib2::algorithms {

class CPW_ISMCTS: public ISMCTS {
 public:
    explicit CPW_ISMCTS(const Domain &domain, Player playingPlayer, const ISMCTSSettings &config) :
        ISMCTS(domain, playingPlayer, config), belief_({1.0}) {};

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;

 private:

    double handlePlayerNode(const shared_ptr<EFGNode> &h) override;

    void fillBelief(const shared_ptr<EFGNode> &currentNode,
                    const shared_ptr<AOH> &newInfoset,
                    double reachProbability,
                    const vector<shared_ptr<EFGNode>> &newNodes);
 protected:
    unordered_map<shared_ptr<AOH>, vector<shared_ptr<EFGNode>>> nodesMap_;
    ProbDistribution belief_;
    shared_ptr<AOH> currentInfoset_;
    virtual void setCurrentInfoset(const shared_ptr<AOH> &newInfoset);
    // checks is the game at current state lets us generate node
    // for example, not valid for stratego during the setup state
    bool validState = false;
};

}

#endif //GTLIB2_CPW_ISMCTS_H
