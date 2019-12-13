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

#include "Exp3SelectorFactory.h"
#include "Exp3Selector.h"
#include "Exp3LSelector.h"

namespace GTLib2::algorithms {

Exp3SelectorFactory::Exp3SelectorFactory(const EXP3_ISMCTSSettings &cfg)
    : cfg_(cfg), generator_(cfg_.seed) {}


unique_ptr<Selector> Exp3SelectorFactory::createSelector(int actionsNumber) const {
    if (cfg_.type == EXP3_ISMCTSSettings::Exp3L)
        return make_unique<Exp3LSelector>(actionsNumber, this);
    else
        return make_unique<Exp3Selector>(actionsNumber, this);
}

unique_ptr<Selector> Exp3SelectorFactory::createSelector(vector<shared_ptr<Action>> actions) const {
    if (cfg_.type == EXP3_ISMCTSSettings::Exp3L) return make_unique<Exp3LSelector>(actions, this);
    else return make_unique<Exp3Selector>(actions, this);
}

std::mt19937 Exp3SelectorFactory::getRandom() const {
    return generator_;
}

double Exp3SelectorFactory::normalizeValue(double value) const {
    assert (cfg_.minUtility_ <= value + 1e-5 && value <= cfg_.maxUtility_ + 1e-5);
    return (value - cfg_.minUtility_) / (cfg_.maxUtility_ - cfg_.minUtility_);
}

}
