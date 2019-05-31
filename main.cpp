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
#include <domains/normal_form_game.h>
#include "algorithms/cfr.h"
#include "domains/goofSpiel.h"
#include "utils/export.h"


using namespace GTLib2;

using domains::NFGDomain;
using domains::NFGSettings;
using domains::NFGInputVariant;
using domains::TwoPlayerSymmetricMatrix;
using domains::VectorOFUtilities;
using utils::exportGraphViz;
using utils::exportGambit;

void exampleBenchmarkCFR() {
    auto start = std::chrono::high_resolution_clock::now();

    auto nfgSettings = NFGSettings();
    nfgSettings.utilityMatrix = {{-1, -3}, {0, -2}};
    nfgSettings.dimensions = {2, 2};
    auto nfg =
        NFGDomain(nfgSettings);
    exportGambit(nfg, "./nfg.gbt");

    auto settings = algorithms::CFRSettings();
    auto cache = algorithms::CFRData(nfg, settings.cfrUpdating);
    algorithms::CFRAlgorithm cfr(nfg, cache, Player(0), settings);
    cfr.runIterations(100);

    auto end = std::chrono::high_resolution_clock::now();
    using ms = std::chrono::duration<int, std::milli>;
    cout << "Time " << std::chrono::duration_cast<ms>(end - start).count() << " ms" << '\n';
}

void exampleExportDomain() {
    auto nfgSettings1 = NFGSettings();
    nfgSettings1.utilityMatrix = {{-1, -3}, {0, -2}};
    nfgSettings1.dimensions = {2, 2};
    auto nfg1 =
        NFGDomain(nfgSettings1);

    auto rpsSettings = NFGSettings();
    rpsSettings.numPlayers = 2;
    rpsSettings.inputVariant = VectorOFUtilities;
    rpsSettings.dimensions = {3, 3};
    rpsSettings.utilities = {{0,0},{-1,1},{1,-1},{1,-1},{0,0},{-1,1},{-1,1},{1,-1},{0,0}};

    auto rps = NFGDomain(rpsSettings);

//    auto nfgSettings2 = NFGSettings();
//    nfgSettings2.dimensions = {2, 2, 2};
//    nfgSettings2.inputVariant = VectorOFUtilities;
//    nfgSettings2.numPlayers = 3;
//    nfgSettings2.utilities = {{-50, -50, -50},{-50, 0, -50},{-50, -50, 0},{100, 0, 0},{0, -50, -50},{0, 0, 100},{0, 100, 0},{0, 0, 0}};

//    auto nfg2 = NFGDomain(nfgSettings2);

//    exportGambit(nfg1, "./nfg1.gbt");
    exportGambit(rps, "./rps.gbt");
//    exportGambit(nfg2, "./nfg2.gbt");

    // you can run this for visualization
    // $ dot -Tsvg iigs3_seed.dot -o iigs3_seed.svg
//    exportGraphViz(nfg1, "./nfg1.dot");
    exportGraphViz(rps, "./rps.dot");
//    exportGraphViz(nfg2, "./nfg2.dot");
}

int main(int argc, char *argv[]) {
//    exampleBenchmarkCFR();
    exampleExportDomain();
    return 0;
}
