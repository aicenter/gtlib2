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



#include "base/tree.h"
#include "base/fogefg.h"
#include "base/gadget.h"
#include "domains/gambit.h"
#include "domains/goofSpiel.h"
#include "utils/export.h"
#include "gtest/gtest.h"

namespace GTLib2 {

TEST(Gambit, ImportExampleFile) {
    // Player 0 plays with probs 0.2, 0.3, 0.5
    const auto iigs = domains::GoofSpielDomain::IIGS(3);
    utils::exportGambit(*iigs, "/tmp/test_gambit.gbt");
    const auto gbt = domains::GambitDomain("/tmp/test_gambit.gbt");
    utils::exportGambit(gbt, "/tmp/test_gambit2.gbt");
    // trees should be identical up to labelling -- no time to implement that now
}

}


