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


#include <chrono>
#include "algorithms/cfr.h"
#include "domains/goofSpiel.h"
#include "domains/randomGame.h"
#include "domains/simple_games.h"
#include "utils/export.h"
#include "utils/benchmark.h"


using namespace GTLib2;

using domains::GoofSpielDomain;
using domains::MatchingPenniesDomain;
using domains::GoofSpielVariant::CompleteObservations;
using domains::GoofSpielVariant::IncompleteObservations;
using utils::exportGraphViz;
using utils::exportGambit;
using utils::benchmark;

void exampleBenchmarkCFR() {
    const auto domain = GoofSpielDomain::IIGS_5();
    auto settings = algorithms::CFRSettings();
    auto cache = algorithms::CFRData(domain, settings.cfrUpdating);
    algorithms::CFRAlgorithm cfr(domain, cache, Player(0), settings);

    auto totalTime = benchmark([&]() {
        cout << "Build time: " << benchmark([&]() { cfr.getCache().buildTree(); }) << " ms" << endl;
        cout << "Iters Time: " << benchmark([&]() { cfr.runIterations(100); }) << " ms" << endl;
    });
    cout << "Total Time: " << totalTime << " ms" << endl;
}

void exampleExportDomain() {
    //@formatter:off
    auto gs2 =        GoofSpielDomain({variant:  CompleteObservations, numCards: 2, fixChanceCards: false, chanceCards: {}});
    auto gs3 =        GoofSpielDomain({variant:  CompleteObservations, numCards: 3, fixChanceCards: false, chanceCards: {}, binaryTerminalRewards: true});
    auto gs3_seed =   GoofSpielDomain({variant:  CompleteObservations, numCards: 3, fixChanceCards: true, chanceCards: {}});
    auto iigs3 =      GoofSpielDomain({variant:  IncompleteObservations, numCards: 3, fixChanceCards: false, chanceCards: {}});
    auto iigs1_seed = GoofSpielDomain({variant:  IncompleteObservations, numCards: 1, fixChanceCards: true, chanceCards: {}});
    auto iigs2_seed = GoofSpielDomain({variant:  IncompleteObservations, numCards: 2, fixChanceCards: true, chanceCards: {}});
    auto iigs3_seed = GoofSpielDomain({variant:  IncompleteObservations, numCards: 3, fixChanceCards: true, chanceCards: {}});
    //@formatter:on
    exportGambit(gs2, "./gs2.gbt");
    exportGambit(gs3, "./gs3.gbt");
    exportGambit(gs3_seed, "./gs3_seed.gbt");
    exportGambit(iigs3, "./iigs3.gbt");
    exportGambit(iigs1_seed, "./iigs1_seed.gbt");
    exportGambit(iigs2_seed, "./iigs2_seed.gbt");
    exportGambit(iigs3_seed, "./iigs3_seed.gbt");
    exportGambit(domains::RPSDomain(), "./rps.gbt");

    // you can run this for visualization
    // $ dot -Tsvg iigs3_seed.dot -o iigs3_seed.svg
    exportGraphViz(iigs3_seed, "./iigs3_seed.dot");
    exportGraphViz(iigs3, "./iigs3.dot");
    exportGraphViz(MatchingPenniesDomain(), "./mp.dot");
    exportGraphViz(domains::RPSDomain(), "./rps.dot");
}

int main(int argc, char *argv[]) {
//    exampleBenchmarkCFR();
    exampleExportDomain();
    return 0;
}
