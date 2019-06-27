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

#include "normal_form_game.h"

#ifndef GTLIB2_SIMPLE_GAMES_H
#define GTLIB2_SIMPLE_GAMES_H


namespace GTLib2::domains {

class RPSDomain: public NFGDomain {
 public:
    inline RPSDomain() : NFGDomain(NFGSettings(
        {{0, 0}, {-1, 1}, {1, -1},
         {1, -1}, {0, 0}, {-1, 1},
         {-1, 1}, {1, -1}, {0, 0}},
        {3, 3}, 2,
        {{"Rock", "Paper", "Scissors"}, {"Rock", "Paper", "Scissors"}}
    )) {}
};

class BiasedRPSDomain: public NFGDomain {
 public:
    inline BiasedRPSDomain(double biasing) : NFGDomain(NFGSettings(
        {{0, 0}, {-1, 1}, {biasing, -biasing},
         {1, -1}, {0, 0}, {-1, 1},
         {-1, 1}, {1, -1}, {0, 0}},
        {3, 3}, 2,
        {{"Rock", "Paper", "Scissors"}, {"Rock", "Paper", "Scissors"}}
    )) {}
};

class MatchingPenniesDomain: public NFGDomain {
 public:
    inline MatchingPenniesDomain() : NFGDomain(NFGSettings(
        {{1, -1}, {-1, 1},
         {-1, 1}, {1, -1}},
        {2,2}, 2,
        {{"Head", "Tail"}, {"Head", "Tail"}}
    )) {}
};

class PrisonnersDilemmaDomain: public NFGDomain {
 public:
    inline PrisonnersDilemmaDomain() : NFGDomain(NFGSettings(
        {{-1, -1}, {-3, 0},
         {0, -3}, {-2, -2}},
        {2, 2}, 2,
        {{"Silent", "Betray"}, {"Silent", "Betray"}}
    )) {}
};

class BattleOfSexesDomain: public NFGDomain {
 public:
    inline BattleOfSexesDomain() : NFGDomain(NFGSettings(
        {{3, 2}, {0, 0},
         {0, 0}, {2, 3}},
        {2, 2}, 2,
        {{"Opera", "Football"}, {"Opera", "Football"}}
    )) {}
};

}

#endif //GTLIB2_SIMPLE_GAMES_H
