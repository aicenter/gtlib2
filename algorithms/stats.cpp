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


#include "algorithms/stats.h"

#include <unordered_set>
#include <unordered_map>
#include <memory>

#include "tree.h"

using std::unordered_set;
using std::cout;

namespace GTLib2 {
namespace algorithms {

void calculateDomainStatistics(const Domain &domain, DomainStatistics *stats) {
    auto collectIS = unordered_map<int, unordered_set<shared_ptr<AOH>>>();
    auto collectSequences = unordered_map<int, unordered_set<ActionSequence>>();

    for (int i = 0; i < GAME_MAX_PLAYERS; ++i) {
        collectIS[i] = unordered_set<shared_ptr<AOH>>();
        collectSequences[i] = unordered_set<ActionSequence>();
    }
    auto countingFn = [&domain, &stats, &collectIS, &collectSequences]
        (shared_ptr<EFGNode> node) {
        stats->num_nodes++;

        if (!node->getParent()
            || node->getParent()->getDepth() != node->getDepth()) {
            stats->num_states++;
        }

        stats->max_EFGDepth = std::max(
            stats->max_EFGDepth, node->getDistanceFromRoot());
        stats->max_StateDepth = std::max(
            stats->max_StateDepth, node->getDepth());

        for (auto &player : domain.getPlayers()) {
            auto seq = node->getActionsSeqOfPlayer(player);
            collectSequences[player].emplace(seq);
        }

        if (node->isTerminal()) {
            stats->num_terminals++;
            return;
        }

        // following stats are only for non-terminal nodes:
        Player player = *node->getCurrentPlayer();
        stats->num_histories[player]++;
        auto infSet = node->getAOHInfSet();
        collectIS[player].emplace(infSet);
    };

    treeWalkEFG(domain, countingFn, domain.getMaxDepth());

    for (int i = 0; i < GAME_MAX_PLAYERS; ++i) {
        stats->num_infosets[i] = collectIS[i].size();
        stats->num_sequences[i] = collectSequences[i].size();
    }
}

void printDomainStatistics(const Domain &domain, std::ostream &ostr) {
    DomainStatistics stats;
    calculateDomainStatistics(domain, &stats);

    ostr << "\n";
    ostr << "Statistics for domain " << typeid(domain).name() << "\n";
    ostr << "---------------------\n";
    ostr << "Info:                \t" << domain.getInfo();
    ostr << stats;
}

}  // namespace algorithms
}  // namespace GTLib2

