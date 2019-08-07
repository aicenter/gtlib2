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

#ifndef GTLIB2_BENCHMARK_CFR_H
#define GTLIB2_BENCHMARK_CFR_H

#include "utils/cli_helpers.h"

#include "algorithms/cfr.h"
#include "domains/goofSpiel.h"
#include "utils/benchmark.h"

void Command_BenchmarkCFR(args::Subparser &parser) {
    using namespace GTLib2;
    using domains::GoofSpielDomain;
    using utils::benchmarkRuntime;
    using algorithms::CFRSettings;
    using algorithms::CFRData;
    using algorithms::CFRAlgorithm;

    initializeParser(parser); // always include this line in command

    const auto domain = GoofSpielDomain::IIGS(5);
    auto settings = CFRSettings();
    auto cache = CFRData(*domain, settings.cfrUpdating);
    CFRAlgorithm cfr(*domain, Player(0), cache, settings);

    auto totalTime = benchmarkRuntime([&]() {
        cout << "Build time: " << benchmarkRuntime([&]() { cfr.getCache().buildTree(); }) << " ms"
             << endl;
        cout << "Iters Time: " << benchmarkRuntime([&]() { cfr.runIterations(100); }) << " ms"
             << endl;
    });
    cout << "Total Time: " << totalTime << " ms" << endl;
}

#endif // GTLIB2_BENCHMARK_CFR_H
