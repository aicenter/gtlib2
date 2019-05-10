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

#include "base/base.h"
#include "algorithms/stats.h"

#include "tree.h"


namespace GTLib2::algorithms {

void calculateDomainStatistics(const Domain &domain, DomainStatistics *stats) {
    const auto numPlayers = domain.getNumberOfPlayers();

    auto collectIS = unordered_map<int, unordered_set<shared_ptr<AOH>>>();
    auto collectSequences = unordered_map<int, unordered_set<shared_ptr<ActionSequence>>>();

    for (int i = 0; i < numPlayers; ++i) {
        collectIS[i] = unordered_set<shared_ptr<AOH>>();
        collectSequences[i] = unordered_set<shared_ptr<ActionSequence>>();
    }

    stats->num_histories = vector<uint64_t>(numPlayers);
    stats->num_infosets = vector<uint64_t>(numPlayers);
    stats->num_sequences = vector<uint64_t>(numPlayers);

    auto countingFn = [&](shared_ptr<EFGNode> node) {
        stats->num_nodes++;

        if (!node->parent_ || node->parent_->stateDepth_ != node->stateDepth_) {
            stats->num_states++;
        }

        stats->max_EFGDepth = max(stats->max_EFGDepth, node->efgDepth_);
        stats->max_StateDepth = max(stats->max_StateDepth, node->stateDepth_);

        for (int i = 0; i < numPlayers; ++i) {
            const auto player = Player(i);
            const auto seq = node->getActionsSeqOfPlayer(player);
            collectSequences[player].emplace(seq);
        }

        if (node->type_ == TerminalNode) {
            stats->num_terminals++;
            return;
        }

        if (node->type_ == ChanceNode) { // todo:
            return;
        }

        // following stats are only for non-terminal nodes:
        const Player player = node->getPlayer();
        stats->num_histories[player]++;
        const auto infSet = node->getAOHInfSet();
        collectIS[player].emplace(infSet);
    };

    treeWalkEFG(domain, countingFn, domain.getMaxStateDepth());

    for (int i = 0; i < domain.getNumberOfPlayers(); ++i) {
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

}  // namespace GTLib2

