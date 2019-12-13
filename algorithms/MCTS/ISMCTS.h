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

#ifndef GTLIB2_ISMCTS_H
#define GTLIB2_ISMCTS_H
#include <base/constrainingDomain.h>
#include "base/algorithm.h"
#include "base/random.h"

#include "algorithms/MCTS/ISMCTS_settings.h"
#include "algorithms/MCTS/selectors/selector.h"
#include "algorithms/MCTS/selectors/selectorFactory.h"
#include "selectors/UCTSelectorFactory.h"
#include "selectors/RMSelectorFactory.h"
#include "selectors/Exp3SelectorFactory.h"


namespace GTLib2::algorithms {
/**
 * Information Set Monte Carlo Tree Search algorithm (ISMCTS) is based on the MCTS algorithm,
 * with a change that allows it to work with imperfect information games.
 *
 * Algorithm is described in "Information Set Monte Carlo Tree Search" paper (Cowling, Powley and Whitehouse, 2012)
 * and improved in "Monte Carlo Tree Search in Imperfect-Information Games" doctoral thesis by Viliam Lisý.
 *
 * Just like the MCTS, algorithm has following steps: selection, expansion, simulation and backpropagation.
 * Unlike the MCTS, the ISMCTS tree consists of the information sets,
 * which are the sets of the nodes with the same action-observation histories.
 *
 * In this algorithm, in every iteration the tree search starts from the root of the tree,
 * no matter of the current state of the game. It allows the algorithm not to store the map IS -> nodes.
 * The downside is that this algorithm is more likely to reach undiscovered infoset.
 * When this happens, it will "give up", and play next moves uniformly randomly.
 *
 * The more advanced version - CPW_ISMCTS - iterates the tree down from the current infoset.
 * Using belief (reach probabilities over histories in current infoset) makes this algorithm
 * even more consistent against non-random strategies.
 */
class ISMCTS: public GamePlayingAlgorithm {
 public:
    explicit inline ISMCTS(const Domain &domain, Player playingPlayer, const ISMCTSSettings &config)
        :
        GamePlayingAlgorithm(domain, playingPlayer),
        config_(move(config)), factory_(move(config_.createFactory())),
        rootNode_(createRootEFGNode(domain)) {
        generator_ = std::mt19937(config.seed);
    }

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;

    virtual double iteration(const shared_ptr<EFGNode> &h);

 protected:
    double handleTerminalNode(const shared_ptr<EFGNode> &h);
    double handleChanceNode(const shared_ptr<EFGNode> &h);
    virtual double handlePlayerNode(const shared_ptr<EFGNode> &h);
    double simulate(const shared_ptr<EFGNode> &h);

    const ISMCTSSettings &config_;
    const unique_ptr<SelectorFactory> factory_;
    std::mt19937 generator_;
    unordered_map<shared_ptr<AOH>, unique_ptr<Selector>> infosetSelectors_;
    const shared_ptr<EFGNode> rootNode_;
};

}

#endif //GTLIB2_ISMCTS_H
