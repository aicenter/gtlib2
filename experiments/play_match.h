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

#ifndef GTLIB2_EXPERIMENT_PLAY_MATCH_H
#define GTLIB2_EXPERIMENT_PLAY_MATCH_H

#include "utils/cli_helpers.h"

namespace GTLib2::CLI {

void Command_PlayMatch(args::Subparser &parser) {
    args::Group group(parser, "Algorithms to play");
    args::ValueFlagList<unsigned int> preplayBudget(group, "ms", "Budget in preplay", {"preplay"});
    args::ValueFlagList<unsigned int> moveBudget(group, "ms", "Budget per move", {"move"});
    args::Group budgetType(group, "Budget type:", args::Group::Validators::Xor);
    args::Flag time(budgetType, "time", "", {"time"});
    args::Flag iterations(budgetType, "iterations", "", {"iterations", "iters"});

    args::ValueFlag<int> seed(group, "", "Match seed", {"seed"});
    initializeParser(parser); // always include this line in command


    const auto algs = args::get(args::alg);
    if (algs.size() != 2) {
        LOG_ERROR("Two algorithms must be specified!");
        exit(1);
    }
    const auto cfgs = args::get(args::algcfg);
    if (algs.size() != 2) {
        LOG_ERROR("Two algorithm configs must be specified!")
        exit(1);
    }

    const auto domain = constructDomain(args::get(args::domain));
    const auto instance1 = constructAlgWithData(*domain, algs.at(0), cfgs.at(0));
    const auto instance2 = constructAlgWithData(*domain, algs.at(1), cfgs.at(1));

    auto pb = args::get(preplayBudget);
    if (pb.size() == 1) pb = vector<int>(2, pb.at(0));
    if (pb.size() == 0) pb = vector<int>(2, 1000);

    auto mb = args::get(moveBudget);
    if (mb.size() == 1) mb = vector<int>(2, mb.at(0));
    if (mb.size() == 0) mb = vector<int>(2, 1000);

    const auto seedValue = seed ? args::get(seed) : 0;

    LOG_INFO("Running match on domain: " << domain->getInfo())
    LOG_INFO("Algorithms: " << algs)
    LOG_INFO("Config files: " << cfgs)
    LOG_INFO("Preplay: " << pb)
    LOG_INFO("Per move: " << mb)
    LOG_INFO("Budget: " << (time ? "time" : "iterations"))
    LOG_INFO("Match seed: " << seedValue)
    LOG_INFO("------------------------")

    cout << playMatch(*domain, {instance1->prepare(), instance2->prepare()},
                      pb, mb, time ? BudgetTime : BudgetIterations, seedValue) << endl;
}

}

#endif // GTLIB2_EXPERIMENT_PLAY_MATCH_H
