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


#ifndef GTLIB2_EVALUATION_H
#define GTLIB2_EVALUATION_H

#include "algorithms/bestResponse.h"

namespace GTLib2::algorithms {

struct Exploitability {
    double bestResp0; // oponentStrategy profile[0], responder Player(1), normalized values
    double bestResp1; // oponentStrategy profile[1], responder Player(0), normalized values
    double expl; // average of normalized values
};

/**
 * Calculate normalized exploitability of the entire strategy profile in two-player zero-sum game.
 */
Exploitability calcExploitability(const Domain &domain, const StrategyProfile &profile);

struct PlayerExploitability {
    double bestResp;
    double expl; 
};

/**
 * Calculate normalized exploitability of player's behavioral strategy in two-player zero-sum game.
 *
 * Game value is utility for Player(0) under Nash eq. strategy.
 * This can be hard to calculate in general, but for some specific games can be easy
 * (for example for II-GoofSpiel it's zero).
 *
 * @image html expl_player.png
 */
PlayerExploitability calcExploitability(const Domain &domain, const BehavioralStrategy &strat,
                          Player pl, double gameValue);

}
#endif //GTLIB2_EVALUATION_H
