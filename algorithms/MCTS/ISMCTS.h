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
#include "base/algorithm.h"
#include "selectors/UCTSelector.h"
#include "base/random.h"

namespace GTLib2::algorithms {
struct ISMCTSSettings{
    /**
     * fact_ describes which of selectors will be used to choose action at current infoset. Following selectors are implemented:
     * - UCTSelector (Upper-bound Confidence for Trees)
     * - RMSelector (Regret Matching)
     * - Exp3Selector
     * - Exp3LSelector
     */
    shared_ptr<SelectorFactory> fact_;

    /**
     * IMPORTANT NOTES
     *
     * 1) If useBelief = true and budget is counted in iterations,
     *      1 iterate of budget becomes up to 101 iterations of algorithm, according to probability of selected node.
     *
     * 2) useBelief DOES NOT AFFECT usual ISMCTS algorithm, only CPW_ISMCTS, as it requires IS -> nodes map.
     *
     * When useBelief = true, if current infoset contains multiple number of nodes,
     * after the choosing randomly a node to iterate, algorithm makes up to 101 iterations,
     * according to the probability of the selected node (n = 1 + prob*100).
     * If there is only one node in the infoset, it will be iterated 101 times.
     *
     * Probability of the node counted as a multiplication of all probabilities,
     * that lead from the previous infoset to the current node.
     * If there are multiple ways from the old infoset to the node, they are summed.
     * In the end, probabilities for all nodes are normalized (see setCurrentInfoset and fillBelief at CPW_ISMCTS).
     */
    bool useBelief = false;

    int randomSeed = 0;
};
/**
 * Information Set Monte Carlo Tree Search algorithm (ISMCTS) is based on the MCTS algorithm,
 * with a changes that allow it to work with imperfect information games.
 *
 * Algorithm is described in "Information Set Monte Carlo Tree Search" paper (Cowling, Powley and Whitehouse, 2012)
 * and improved in "Monte Carlo Tree Search in Imperfect-Information Games" doctoral thesis by Viliam Lisý.
 *
 * Just like the MCTS, algorithm has following steps: selection, expansion, simulation and backpropagation.
 * Unlike the MCTS, the ISMCTS tree consists of the information sets,
 * which are the unions of the nodes with the same action-observation histories.
 *
 * In this algorithm, in every iteration the tree search starts from the root of the tree,
 * no matter of the current state of the games. It allows the algorithm not to store the map IS -> nodes.
 * The downside is that this algorithm is more likely to reach undiscovered infoset.
 *
 * The more advanced version - CPW_ISMCTS - iterates the tree down from the current infoset.
 * Using belief makes this algorithm even more consistent against non-random strategies.
 */
class ISMCTS : public GamePlayingAlgorithm {
public:
    explicit ISMCTS(const Domain &domain, Player playingPlayer, const ISMCTSSettings& config) :
        GamePlayingAlgorithm(domain, playingPlayer), config_(config), rootNode_(createRootEFGNode(domain)) {
        generator_ = std::mt19937(config.randomSeed);
    };

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;

protected:
    const ISMCTSSettings config_;
    std::mt19937 generator_;
    unordered_map<shared_ptr<AOH>, unique_ptr<Selector>> infosetSelectors_;
    const shared_ptr<EFGNode> rootNode_;


    virtual double iteration(const shared_ptr <EFGNode> &h);

    double handleTerminalNode(const shared_ptr <EFGNode> &h);

    double handleChanceNode(const shared_ptr <EFGNode> &h);

    virtual double handlePlayerNode(const shared_ptr <EFGNode> &h);

    double simulate(const shared_ptr <EFGNode> &h);
};
}

#endif //GTLIB2_ISMCTS_H
