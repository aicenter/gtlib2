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

#include "evaluation.h"

namespace GTLib2::algorithms {

Exploitability calcExploitability(const Domain &domain, const StrategyProfile &profile) {
    assert(domain.getNumberOfPlayers() == 2);
    assert(domain.isZeroSum());

    const auto maxUtility = domain.getMaxUtility();
    const auto bestResp0 = bestResponseTo(profile[0], Player(1), domain).value / maxUtility;
    const auto bestResp1 = bestResponseTo(profile[1], Player(0), domain).value / maxUtility;
    const double expl = (bestResp0 + bestResp1) / 2;

    LOG_VAL("BR_pl1(strategy0) ", bestResp0)
    LOG_VAL("BR_pl0(strategy1) ", bestResp1)
    LOG_VAR(expl)
    assert(expl <= 1.0);
    assert(expl >= 0.0);
    return Exploitability{bestResp0, bestResp1, expl};
}

PlayerExploitability calcExploitability(const Domain &domain, const BehavioralStrategy &strat,
                                        Player pl, double gameValue) {
    assert(domain.getNumberOfPlayers() == 2);
    assert(domain.isZeroSum());
    assert(pl == Player(0) || pl == Player(1));

    const auto maxUtility = domain.getMaxUtility();
    const auto bestResp = bestResponseTo(strat, opponent(pl), domain).value / maxUtility;
    const double expl = (gameValue / maxUtility) + bestResp;

    LOG_VAL("BR_pl" << opponent(pl) << "(strategy" << pl << ") ", bestResp)
    LOG_VAR(expl)
    assert(expl <= 1.0);
    assert(expl >= 0.0);
    return PlayerExploitability{bestResp, expl};
}

}
