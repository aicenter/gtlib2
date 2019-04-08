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
#include <algorithms/cfr.h>
#if GUROBIFOUND == 1
#include "LPsolvers/GurobiLPSolver.h"
#elif CPLEXFOUND == 1
#include "LPsolvers/CplexLPSolver.h"
#elif GLPKFOUND == 1
#include "LPsolvers/GlpkLPSolver.h"
#endif
#include "base/efg.h"
#include "base/cache.h"
#include "algorithms/utility.h"
#include "algorithms/cfr.h"
#include "algorithms/common.h"
#include "algorithms/normalFormLP.h"
#include "algorithms/tree.h"
#include "algorithms/bestResponse.h"
#include "algorithms/equilibrium.h"
#include "domains/goofSpiel.h"
#include "domains/phantomTTT.h"
#include "domains/pursuit.h"
#include "domains/matching_pennies.h"
#include "domains/RhodeIslandPoker.h"
#include "domains/genericPoker.h"


using std::endl;
using std::cout;
using namespace GTLib2;


int main(int argc, char *argv[]) {
    auto start = std::chrono::high_resolution_clock::now();

    domains::IIGoofSpielDomain domain(5, 5, 0);
    algorithms::CFRAlgorithm cfr(domain, Player(0));
    cfr.runIterations(100);

    auto end = std::chrono::high_resolution_clock::now();
    using ms = std::chrono::duration<int, std::milli>;
    cout << "Time " << std::chrono::duration_cast<ms>(end - start).count() << " ms" << '\n';
    return 0;
}
