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


using std::endl;
using std::cout;
using namespace GTLib2;

using domains::IIGoofSpielDomain;
using utils::exportGraphViz;
using utils::exportGambit;

void exampleBenchmarkCFR() {
    auto start = std::chrono::high_resolution_clock::now();
    domains::IIGoofSpielDomain domain(5, 5, 0);
    algorithms::CFRAlgorithm cfr(domain, Player(0), algorithms::CFRSettings());
    cfr.runIterations(100);

    auto end = std::chrono::high_resolution_clock::now();
    using ms = std::chrono::duration<int, std::milli>;
    cout << "Time " << std::chrono::duration_cast<ms>(end - start).count() << " ms" << '\n';
}

void exampleExportDomain() {
    auto iigs3_seed = IIGoofSpielDomain(3, 3, 0);
    exportGambit(iigs3_seed, "./iigs3_seed.gbt");

    // you can run this for visualization
    // $ dot -Tsvg iigs3_seed.dot -o iigs3_seed.svg
    exportGraphViz(iigs3_seed, "./iigs3_seed.dot");
}

int main(int argc, char *argv[]) {
    exampleBenchmarkCFR();
    exampleExportDomain();
    return 0;
}
