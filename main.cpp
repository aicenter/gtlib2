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

#include "utils/global_args.h"

// All the possible commands
#include "experiments/benchmark_cfr.h"
#include "experiments/export_domain.h"
#include "experiments/cfr_regrets.h"

#include <iostream>
#include <algorithms/MCTS/ISMCTS.h>
#include <algorithms/MCTS/CPWISMCTS.h>
#include <domains/goofSpiel.h>

int main(int argc, const char **argv) {

        auto fact = make_shared<UCTSelectorFactory>(sqrt(2));
        int a = 0;
    ISMSTCSettings s = {.useBelief = true, .fact_ = std::static_pointer_cast<SelectorFactory>(fact), .randomSeed = 123};
    PreparedAlgorithm firstAction = createInitializer<CPWISMCTS>(s);
    PreparedAlgorithm lastAction = createInitializer<RandomPlayer>();

    GTLib2::domains::GoofSpielSettings settings
            ({variant:  GTLib2::domains::IncompleteObservations, numCards: 3, fixChanceCards: false});
//    settings.shuffleChanceCards(2);
    GoofSpielDomain domain(settings);
    vector<double> actualUtilities = playMatch(
            domain, vector<PreparedAlgorithm>{firstAction, lastAction},
            vector<int>{10000, 10000}, vector<int>{100, 100}, BudgetIterations, 0);

//    GTLib2::domains::StrategoSettings settings = {3,2,{},{'1', '2'}};
//    GTLib2::domains::StrategoDomain domain(settings);
//    vector<double> actualUtilities = playMatch(
//            domain, vector<PreparedAlgorithm>{firstAction, lastAction},
//            vector<int>{1000, 1000}, vector<int>{10, 10}, BudgetIterations, 0);

    args::ArgumentParser parser("Command runner for GTLib2");
    args::CompletionFlag completion(parser, {"complete"}); // bash completion
    args::GlobalOptions globals(parser, args::arguments);

    // --------- utils ----------
    args::Group utils(parser, "commands (utils)");
    args::Command export_domain(utils, "export_domain",
                                "Export domain to gambit or graphviz", &Command_ExportDomain);
    args::Command benchmark_cfr(utils, "benchmark_cfr",
                                "Calculate run time of CFR on IIGS-5.", &Command_BenchmarkCFR);

    // ------ experiments -------
    args::Group experiments(parser, "commands (experiments)");
    args::Command cfr_regrets(experiments, "cfr_regrets",
                              "Calculate regrets and strategies in CFR", &Command_CFRRegrets);

    try {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help &) {
        std::cout << parser;
        return 0;
    }
    catch (const args::Completion &e) {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Error &e) {
        std::cerr << e.what() << std::endl << parser;
        return 1;
    }

    return 0;
}
