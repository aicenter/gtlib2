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

double calcExploitability(Domain &domain, const StrategyProfile &profile) {
    assert(domain.getNumberOfPlayers() == 2);
    assert(domain.isZeroSum());

    const auto bestResp0 = bestResponseTo(profile[0], Player(1), domain).value;
    const auto bestResp1 = bestResponseTo(profile[1], Player(0), domain).value;
    const double expl = (bestResp0 + bestResp1) / (2 * domain.getMaxUtility());

    assert(expl <= 1.0);
    assert(expl >= 0.0);
    return expl;
}

double calcExploitability(Domain &domain, const BehavioralStrategy &strat,
                          Player pl, double gameValue) {
    assert(domain.getNumberOfPlayers() == 2);
    assert(domain.isZeroSum());
    assert(pl == Player(0) || pl == Player(1));

    const auto bestRespValue = bestResponseTo(strat, opponent(pl), domain).value;
    const double expl = (gameValue + bestRespValue) / domain.getMaxUtility();

    assert(expl <= 1.0);
    assert(expl >= 0.0);
    return expl;
}

}
