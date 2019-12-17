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
#include "domains/gambit/gambit.h"
#include "domains/goofSpiel.h"
#include "utils/export.h"
#include "gtest/gtest.h"

namespace GTLib2::domains {
namespace {

using namespace domains::gambit;

TEST(Gambit, ImportExampleFile) {
    const auto iigs = domains::GoofSpielDomain::IIGS(3);
    utils::exportGambit(*iigs, "/tmp/test_gambit.gbt");
    const auto gbt = loadFromFile("/tmp/test_gambit.gbt");
    utils::exportGambit(*gbt, "/tmp/test_gambit2.gbt");
    // trees should be identical up to labelling -- no time to implement that now
}


TEST(Gambit, ParsingGambitTests) {
    {
        auto actualNode = parseNodeLine("c \"\" 1 \"\" { \"\" 0.500 \"\" 0.500 } 0", 1);
        auto expectedNode = gambit::Node{
            /*node_type=*/'c',
            /*player=*/2,
            /*infoset_idx=*/0,
            /*pubstate_idx=*/0,
            /*num_actions=*/2,
            /*utils=*/{},
            /*probs=*/{0.5, 0.5},
            /*description=*/"",
            /*children=*/{}
        };
        assert(*actualNode == expectedNode);
    }
    {
        auto actualNode = parseNodeLine("c \"\" 1 \"\" { \"a\" 0.500 \"b\" 0.500 } 0", 1);
        auto expectedNode = gambit::Node{
            /*node_type=*/'c',
            /*player=*/2,
            /*infoset_idx=*/0,
            /*pubstate_idx=*/0,
            /*num_actions=*/2,
            /*utils=*/{},
            /*probs=*/{0.5, 0.5},
            /*description=*/"",
            /*children=*/{}
        };
        assert(*actualNode == expectedNode);
    }
    {
        auto actualNode = parseNodeLine("c \"\" 1 \"\" { \"\" 0.400 \"\" 0.600 } 0", 1);
        auto expectedNode = gambit::Node{
            /*node_type=*/'c',
            /*player=*/2,
            /*infoset_idx=*/0,
            /*pubstate_idx=*/0,
            /*num_actions=*/2,
            /*utils=*/{},
            /*probs=*/{0.4, 0.6},
            /*description=*/"",
            /*children=*/{}
        };
        assert(*actualNode == expectedNode);
    }
    {
        auto actualNode = parseNodeLine("p \"\" 1 9988899 \"\" { \"\" \"\" \"\" } 0", 1);
        auto expectedNode = gambit::Node{
            /*node_type=*/'p',
            /*player=*/0,
            /*infoset_idx=*/9988899,
            /*pubstate_idx=*/0,
            /*num_actions=*/3,
            /*utils=*/{},
            /*probs=*/{},
            /*description=*/"",
            /*children=*/{}
        };
        assert(*actualNode == expectedNode);
    }
    {
        auto actualNode = parseNodeLine("p \"\" 1 10 \"123\" { \"\" \"\" \"\" } 0", 1);
        auto expectedNode = gambit::Node{
            /*node_type=*/'p',
            /*player=*/0,
            /*infoset_idx=*/10,
            /*pubstate_idx=*/123,
            /*num_actions=*/3,
            /*utils=*/{},
            /*probs=*/{},
            /*description=*/"",
            /*children=*/{}
        };
        assert(*actualNode == expectedNode);
    }
    {
        auto actualNode = parseNodeLine("t \"\" 2 \"\" { 0, 0 }", 1);
        auto expectedNode = gambit::Node{
            /*node_type=*/'t',
            /*player=*/3,
            /*infoset_idx=*/0,
            /*pubstate_idx=*/0,
            /*num_actions=*/0,
            /*utils=*/{0, 0},
            /*probs=*/{},
            /*description=*/"",
            /*children=*/{}
        };
        assert(*actualNode == expectedNode);
    }
}

}
}


