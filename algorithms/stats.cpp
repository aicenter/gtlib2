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

#include "treeWalk.h"

using std::unordered_set;
using std::cout;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {
namespace algorithms {

void calculateDomainStatistics(const Domain &domain, DomainStatistics *stats) {
    auto collectIS = unordered_map<int, unordered_set<shared_ptr<AOH>>>();

    for (int i = 0; i < GAME_MAX_PLAYERS; ++i) {
        collectIS[i] = unordered_set<shared_ptr<AOH>>();
    }

    auto countingFn = [&domain, &stats, &collectIS]
        (shared_ptr<EFGNode> node) {
      if (node->getCurrentPlayer()) stats->num_nodes++;

      if (!node->getParent()
          || node->getParent()->getDepth() != node->getDepth()) {
          stats->num_states++;
      }

      stats->max_EFGDepth = std::max(
          stats->max_EFGDepth, node->getDepth());

      if (node->isTerminal()) {
          stats->num_terminals++;
          return;
      }

      int player = *node->getCurrentPlayer();
      stats->num_histories[player]++;
      auto infSet = node->getAOHInfSet();
      collectIS[player].emplace(infSet);
    };

    treeWalkEFG(domain, countingFn, domain.getMaxDepth());

    for (int i = 0; i < GAME_MAX_PLAYERS; ++i) {
        stats->num_infosets[i] = collectIS[i].size();
    }
}

void printDomainStatistics(const Domain &domain, std::ostream& ostr) {
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

#pragma clang diagnostic pop

