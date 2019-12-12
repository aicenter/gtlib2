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

#include "gambit_games.h"

namespace GTLib2::domains::gambit {

unique_ptr<Domain> doublePokerDomain() {
    std::stringstream ss;
    ss << "EFG 2 R \"\" { \"Pl0\" \"Pl1\" }\n"
          "p \"\" 1 1000 \"100\" { \"Pl1 begins\" \"Pl2 begins\" } 0\n"
          " c \"\" 1 \"\" { \"CardsJJ\" 0.167 \"CardsJQ\" 0.333 \"CardsQJ\" 0.333 \"CardsQQ\" 0.167 } 0\n"
          "  p \"\" 1 1110 \"111\" { \"Fold\" \"Bet\" } 0\n"
          "   t \"\" 1 \"\" { -1, 1 }\n"
          "   p \"\" 2 1120 \"112\" { \"Fold\" \"Call\" } 0\n"
          "    t \"\" 2 \"\" { 1, -1 }\n"
          "    t \"\" 3 \"\" { 0, 0 }\n"
          "  p \"\" 1 1110 \"111\" { \"Fold\" \"Bet\" } 0\n"
          "   t \"\" 4 \"\" { -1, 1 }\n"
          "   p \"\" 2 1121 \"112\" { \"Fold\" \"Call\" } 0\n"
          "    t \"\" 5 \"\" { 1, -1 }\n"
          "    t \"\" 6 \"\" { -3, 3 }\n"
          "  p \"\" 1 1111 \"111\" { \"Fold\" \"Bet\" } 0\n"
          "   t \"\" 7 \"\" { -1, 1 }\n"
          "   p \"\" 2 1120 \"112\" { \"Fold\" \"Call\" } 0\n"
          "    t \"\" 8 \"\" { 1, -1 }\n"
          "    t \"\" 9 \"\" { 3, -3 }\n"
          "  p \"\" 1 1111 \"111\" { \"Fold\" \"Bet\" } 0\n"
          "   t \"\" 10 \"\" { -1, 1 }\n"
          "   p \"\" 2 1121 \"112\" { \"Fold\" \"Call\" } 0\n"
          "    t \"\" 11 \"\" { 1, -1 }\n"
          "    t \"\" 12 \"\" { 0, 0 }\n"
          " c \"\" 2 \"\" { \"CardsJJ\" 0.167 \"CardsJQ\" 0.333 \"CardsQJ\" 0.333 \"CardsQQ\" 0.167 } 0\n"
          "  p \"\" 2 1220 \"122\" { \"Fold\" \"Bet\" } 0\n"
          "   t \"\" 1 \"\" { -1, 1 }\n"
          "   p \"\" 1 1210 \"121\" { \"Fold\" \"Call\" } 0\n"
          "    t \"\" 2 \"\" { 1, -1 }\n"
          "    t \"\" 3 \"\" { 0, 0 }\n"
          "  p \"\" 2 1220 \"122\" { \"Fold\" \"Bet\" } 0\n"
          "   t \"\" 4 \"\" { -1, 1 }\n"
          "   p \"\" 1 1211 \"121\" { \"Fold\" \"Call\" } 0\n"
          "    t \"\" 5 \"\" { 1, -1 }\n"
          "    t \"\" 6 \"\" { -3, 3 }\n"
          "  p \"\" 2 1221 \"122\" { \"Fold\" \"Bet\" } 0\n"
          "   t \"\" 7 \"\" { -1, 1 }\n"
          "   p \"\" 1 1210 \"121\" { \"Fold\" \"Call\" } 0\n"
          "    t \"\" 8 \"\" { 1, -1 }\n"
          "    t \"\" 9 \"\" { 3, -3 }\n"
          "  p \"\" 2 1221 \"122\" { \"Fold\" \"Bet\" } 0\n"
          "   t \"\" 10 \"\" { -1, 1 }\n"
          "   p \"\" 1 1211 \"121\" { \"Fold\" \"Call\" } 0\n"
          "    t \"\" 11 \"\" { 1, -1 }\n"
          "    t \"\" 12 \"\" { 0, 0 }\n";
    return make_unique<GambitDomain>(ss);
}

unique_ptr<Domain> matchingPenniesGadgetGame() {
    std::stringstream ss;
    ss << "EFG 2 R \"\" { \"Pl0\" \"Pl1\" }\n"
          "c \"\" 0 \"\" { \"0\" 0.5 \"1\" 0.5 } 0\n"
          " p \"\" 1 1 \"-1\" { \"Follow\" \"Terminate\" } 0\n"
          " p \"\" 2 2 \"1\" { \"Head\" \"Tail\" } 0\n"
          "  t \"\" 0 \"\" { 1, -1}\n"
          "  t \"\" 1 \"\" { -1, 1}\n"
          "  t \"\" 2 \"\" { 0, -0}\n"
          " p \"\" 1 3 \"-1\" { \"Follow\" \"Terminate\" } 0\n"
          " p \"\" 2 2 \"1\" { \"Head\" \"Tail\" } 0\n"
          "  t \"\" 3 \"\" { -1, 1}\n"
          "  t \"\" 4 \"\" { 1, -1}\n"
          "  t \"\" 5 \"\" { 0, -0}\n";
    return make_unique<GambitDomain>(ss);
}

}
