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
#include "experiments/calc_expl.h"
#include "experiments/export_domain.h"
#include "experiments/export_ring.h"
#include "experiments/cfr_regrets.h"
#include "experiments/play_match.h"

int main(int argc, const char **argv) {
    using namespace GTLib2;
    using namespace GTLib2::CLI;

    args::ArgumentParser parser("Command runner for GTLib2");
    args::CompletionFlag completion(parser, {"complete"}); // bash completion
    args::GlobalOptions globals(parser, args::arguments); // add global args

    // --------- utils ----------
    args::Group utils(parser, "commands (utils)");
    args::Command export_domain(utils, "export_domain",
                                "Export domain to gambit or graphviz", &Command_ExportDomain);
    args::Command benchmark_cfr(utils, "benchmark_cfr",
                                "Calculate run time of CFR on IIGS-5.", &Command_BenchmarkCFR);
    args::Command play_match(utils, "play_match",
                             "Play match given preplay and per-move time.", &Command_PlayMatch);
    args::Command calc_expl(utils, "calc_expl",
                            "Calc exploitability of alg strategy "
                            "given preplay and per-move time.", &Command_CalcExpl);

    // ------ experiments -------
    args::Group experiments(parser, "commands (experiments)");
    args::Command cfr_regrets(experiments, "cfr_regrets",
                              "Calculate regrets and strategies in CFR", &Command_CFRRegrets);
    args::Command export_ring(experiments, "export_ring",
                              "Export leaf utilities of EFG as a color ring", &Command_ExportRing);

    try {
        parser.ParseCLI(argc, argv);
        if (args::get(args::run_time))
            LOG_INFO("Total runtime of the command " << time_diff(runStartTime))
    }
    catch (const args::Help &) {
        std::cout << parser;
        return 0;
    }
    catch (const args::Completion &e) {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Header &e) {
        return 0;
    }
    catch (const args::Error &e) {
        std::cerr << e.what() << std::endl << parser;
        return 1;
    }


    return 0;
}
