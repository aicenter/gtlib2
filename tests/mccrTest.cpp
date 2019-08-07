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

    You should have received a copy of the GNU Lesser General Public
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/



#include "base/base.h"
#include "base/fogefg.h"

#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/strategy.h"
#include "algorithms/continualResolving.h"
#include "algorithms/oos.h"
#include "algorithms/mccr.h"

#include "algorithms/utility.h"
#include "algorithms/evaluation.h"
#include "domains/goofSpiel.h"
#include "domains/simple_games.h"

#include "gtest/gtest.h"


namespace GTLib2::algorithms {

using domains::GoofSpielDomain;
using domains::RPSDomain;

TEST(MCCR, ResolveDomain) {
    auto d = GoofSpielDomain::IIGS(4);
    auto root = createRootEFGNode(*d);
    auto cache = OOSData(*d);
    auto cr = MCCRAlgorithm(*d, Player(0), cache, MCCRSettings());
    cr.solveEntireGame(1000, 1000, BudgetIterations);

    cout << "expl " << calcExploitability(*d, getAverageStrategy(cache)) << endl;
}

}
