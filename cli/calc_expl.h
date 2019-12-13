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

#ifndef GTLIB2_EXPERIMENT_CALC_EXPL_H
#define GTLIB2_EXPERIMENT_CALC_EXPL_H

#include "cli_helpers.h"
#include "algorithms/evaluation.h"
#include "algorithms/onlineAvgStrategy.h"

namespace GTLib2::CLI {

StrategyProfile CFR_AverageStrategy(const Domain &domain, const AlgParams &cfg,
                                    unsigned int preplayBudget, BudgetType budgetType) {
    const auto alg = constructAlgWithData(domain, "CFR", cfg);
    const unique_ptr<GamePlayingAlgorithm> gameAlg = alg->prepare()(domain, Player(0));
    CFRAlgorithm *cfr = dynamic_cast<CFRAlgorithm *>(gameAlg.get());
    LOG_DEBUG("Calculating strategy")
    playForBudget(*gameAlg, PLAY_FROM_ROOT, preplayBudget, budgetType);
    return getAverageStrategy(cfr->getCache());
}

void Command_CalcExpl(args::Subparser &parser) {
    args::Group group(parser, "Algorithms to play");
    args::ValueFlag<unsigned int> preplayBudget(group, "ms", "Budget in preplay",
                                                {"preplay"}, 1000);
    args::ValueFlag<unsigned int> moveBudget(group, "ms", "Budget per move", {"move"}, 0);
    args::Group budgetType(group, "Budget type:", args::Group::Validators::Xor);
    args::Flag time(budgetType, "time", "", {"time"});
    args::Flag iterations(budgetType, "iterations", "", {"iterations", "iters"});
    args::Flag printProfile(parser, "printProfile", "Print strategy profile?", {"print"}, false);
    args::Flag dbar(parser, "dbar", "Compute double bar strategy (estimate expectation over average strats)",
                    {"dbar"}, false);
    initializeParser(parser); // always include this line in command

    if (args::get(args::tag_header)) {
        cout << "preplay" << ",";
        cout << "move" << ",";
        cout << "budget" << ",";
        cout << "expl" << endl;
        throw args::Header();
    }

    const auto algs = args::get(args::alg);
    if (algs.size() != 1) {
        LOG_ERROR("Exactly one algorithm must be specified!");
        exit(1);
    }

    const auto cfgs = CLI::algParams;
    const auto domain = constructDomain(args::get(args::domain));
    const unsigned int pb = args::get(preplayBudget);
    const unsigned int mb = args::get(moveBudget);
    const BudgetType btype = time ? BudgetTime : BudgetIterations;
    const auto alg = algs.at(0);
    const auto cfg = cfgs.at(0);

    if (args::get(args::tag)) {
        cout << pb << ",";
        cout << mb << ",";
        cout << (time ? "time" : "iterations") << ",";
    }

    LOG_INFO("Running expl computation on domain: " << domain->getInfo())
    LOG_INFO("Algorithm: " << alg)
    LOG_INFO("Config file: " << cfg)
    LOG_INFO("Preplay: " << pb)
    LOG_INFO("Per move: " << mb)
    LOG_INFO("Budget: " << (time ? "time" : "iterations"))
    LOG_INFO("------------------------")

    StrategyProfile profile;
    if (alg == "CFR") profile = CFR_AverageStrategy(*domain, cfg, pb, btype);
    else if (alg == "OOS") profile = OOS_AverageStrategy(*domain, cfg, pb, mb, btype);
    else if (alg == "MCCR") {
        if(dbar) profile = MCCR_DbarAverageStrategy(*domain, cfg, pb, mb, btype);
        else profile = MCCR_AverageStrategy(*domain, cfg, pb, mb, btype);
    }
    else LOG_ERROR("Algorthim " << alg << "does not have implemented exploitability calculation");

    if (printProfile) {
        for (int pl = 0; pl < 2; ++pl) {
            cout << "Behavioral strategy for player " << pl << endl;
            cout << profile.at(pl) << endl;
            cout << "-----------" << endl;
        }
    }

    LOG_DEBUG("Calculating exploitability")
    cout << calcExploitability(*domain, profile).expl << endl;
}

}

#endif // GTLIB2_EXPERIMENT_CALC_EXPL_H
