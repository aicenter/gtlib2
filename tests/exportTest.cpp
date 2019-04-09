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


#include <unordered_set>

#include "algorithms/bestResponse.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"
#include "utils/functools.h"
#include "utils/export.h"

#include <boost/range/combine.hpp>
#include <boost/test/unit_test.hpp>
#include <domains/genericPoker.h>

namespace GTLib2 {

using domains::IIGoofSpielDomain;
using domains::GoofSpielDomain;
using domains::MatchingPenniesDomain;
using domains::GenericPokerDomain;

using utils::exportGraphViz;
using utils::exportGambit;

BOOST_AUTO_TEST_SUITE(Export);

BOOST_AUTO_TEST_CASE(exportSmallDomain) {
    auto gs = GoofSpielDomain(2, 2, nullopt);
    exportGraphViz(gs, "/home/michal/Code/GT/gtlib2/utils/gs.dot");
    exportGambit(gs, "/home/michal/Code/GT/gtlib2/utils/gs.gbt");

    auto iigs = IIGoofSpielDomain(2, 2, nullopt);
    exportGraphViz(iigs, "/home/michal/Code/GT/gtlib2/utils/iigs.dot");
    exportGambit(iigs, "/home/michal/Code/GT/gtlib2/utils/iigs.gbt");

    auto iigs_seed = IIGoofSpielDomain(3, 3, 0);
    exportGraphViz(iigs_seed, "/home/michal/Code/GT/gtlib2/utils/iigs_seed.dot");
    exportGambit(iigs_seed, "/home/michal/Code/GT/gtlib2/utils/iigs_seed.gbt");

    auto mp = MatchingPenniesDomain();
    exportGraphViz(mp, "/home/michal/Code/GT/gtlib2/utils/mp.dot");
    exportGambit(mp, "/home/michal/Code/GT/gtlib2/utils/mp.gbt");

    auto poker = GenericPokerDomain(2, 2, 1, 2, 1);
    exportGraphViz(poker, "/home/michal/Code/GT/gtlib2/utils/poker.dot");
    exportGambit(poker, "/home/michal/Code/GT/gtlib2/utils/poker.gbt");

    // todo: finish test
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()

}
