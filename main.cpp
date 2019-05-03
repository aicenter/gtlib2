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
#include "utils/export.h"


using namespace GTLib2;

using domains::GoofSpielDomain;
using domains::GoofSpielVariant::CompleteObservations;
using domains::GoofSpielVariant::IncompleteObservations;
using utils::exportGraphViz;
using utils::exportGambit;

void exampleBenchmarkCFR() {
    auto start = std::chrono::high_resolution_clock::now();

    domains::GoofSpielDomain domain({
                                        variant:  IncompleteObservations,
                                        numCards: 5,
                                        fixChanceCards: true,
                                        chanceCards: {}
                                    });
    auto settings = algorithms::CFRSettings();
    auto cache = algorithms::CFRData(domain, settings.cfrUpdating);
    algorithms::CFRAlgorithm cfr(domain, cache, Player(0), settings);
    cfr.runIterations(100);

    auto end = std::chrono::high_resolution_clock::now();
    using ms = std::chrono::duration<int, std::milli>;
    cout << "Time " << std::chrono::duration_cast<ms>(end - start).count() << " ms" << '\n';
}

void exampleExportDomain() {
    auto gs2 =
        GoofSpielDomain({variant:  CompleteObservations, numCards: 2, fixChanceCards: false, chanceCards: {}});
    auto gs3 =
        GoofSpielDomain({variant:  CompleteObservations, numCards: 3, fixChanceCards: false, chanceCards: {}, binaryTerminalRewards: true});
    auto gs3_seed =
        GoofSpielDomain({variant:  CompleteObservations, numCards: 3, fixChanceCards: true, chanceCards: {}});
    auto iigs3 =
        GoofSpielDomain({variant:  IncompleteObservations, numCards: 3, fixChanceCards: false, chanceCards: {}});
    auto iigs3_seed =
        GoofSpielDomain({variant:  IncompleteObservations, numCards: 3, fixChanceCards: true, chanceCards: {}});
    exportGambit(gs2, "./gs2.gbt");
    exportGambit(gs3, "./gs3.gbt");
    exportGambit(gs3_seed, "./gs3_seed.gbt");
    exportGambit(iigs3, "./iigs3.gbt");
    exportGambit(iigs3_seed, "./iigs3_seed.gbt");

    // you can run this for visualization
    // $ dot -Tsvg iigs3_seed.dot -o iigs3_seed.svg
    exportGraphViz(iigs3_seed, "./iigs3_seed.dot");
}

int main(int argc, char *argv[]) {
    exampleBenchmarkCFR();
//    exampleExportDomain();
    return 0;
}
