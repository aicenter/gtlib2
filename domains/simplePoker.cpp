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


#include <base/random.h>
#include "domains/simplePoker.h"


namespace GTLib2::domains {


SimplePokerDomain::SimplePokerDomain() :
    Domain(3, 2, true,
           make_shared<SimplePokerAction>(),
           make_shared<SimplePokerObservation>()) {

    maxUtility_ = 3.;
    rootStatesDistribution_ = OutcomeDistribution();

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            auto deck = array<int, 2>{i, j};
            auto prob = j == i ? 1 / 6. : 1 / 3.;
            auto newState = make_shared<SimplePokerState>(this, Player(0), deck, false);
            auto player0Obs = make_shared<SimplePokerObservation>(i);
            auto player1Obs = make_shared<SimplePokerObservation>(j);

            Outcome outcome(newState, {player0Obs, player1Obs}, noObservation_, {0.0, 0.0});
            rootStatesDistribution_.emplace_back(OutcomeEntry{outcome, prob});
        }
    }
}

unsigned long SimplePokerState::countAvailableActionsFor(Player) const {
    return isTerminal_ ? 0 : 2;
}

vector<shared_ptr<Action>> SimplePokerState::getAvailableActionsFor(const Player player) const {
    if(isTerminal_ || player != actingPlayer_) return {};

    if (player == Player(0))
        return {
            make_shared<SimplePokerAction>(kActionFold),
            make_shared<SimplePokerAction>(kActionBet)
        };
    else
        return {
            make_shared<SimplePokerAction>(kActionFold),
            make_shared<SimplePokerAction>(kActionCall)
        };
}

OutcomeDistribution
SimplePokerState::performActions(const vector<shared_ptr<Action>> &actions) const {
    const auto currentAction = dynamic_pointer_cast<SimplePokerAction>(actions.at(actingPlayer_));
    const int actionType = currentAction->actionType_;
    const auto noObs = domain_->getNoObservation();
    const int sign = actingPlayer_ == Player(0) ? 1 : -1;

    switch (actionType) {
        case kActionFold:
            return OutcomeDistribution{OutcomeEntry(Outcome(
                make_shared<SimplePokerState>(domain_, NO_PLAYER, cards_, true),
                {noObs, noObs}, make_shared<SimplePokerObservation>(0), {-1. * sign, 1. * sign}
            ))};
        case kActionBet:
            assert(actingPlayer_ == Player(0));
            return OutcomeDistribution{OutcomeEntry(Outcome(
                make_shared<SimplePokerState>(domain_, opponent(actingPlayer_), cards_, false),
                {noObs, noObs}, make_shared<SimplePokerObservation>(1), {0, 0}
            ))};
        case kActionCall: {
            assert(actingPlayer_ == Player(1));
            double v = cards_[0] == cards_[1] ? 0 :
                       (cards_[0] > cards_[1] ? 3 : -3);
            return OutcomeDistribution{OutcomeEntry(Outcome(
                make_shared<SimplePokerState>(domain_, NO_PLAYER, cards_, true),
                {noObs, noObs}, make_shared<SimplePokerObservation>(1), {v, -v}
            ))};
        }
        default:
            unreachable("unrecognized action");
    }

}

string SimplePokerState::toString() const {
    std::stringstream ss;
    ss << "{ " << (cards_[0] ? "J " : "Q") << (cards_[0] ? "J " : "Q") << "}";
    return ss.str();
}

bool SimplePokerState::operator==(const State &rhs) const {
    auto spState = dynamic_cast<const SimplePokerState &>(rhs);
    return hash_ == spState.hash_
        && cards_ == spState.cards_
        && actingPlayer_ == spState.actingPlayer_
        && isTerminal_ == spState.isTerminal_;
}

}  // namespace GTLib2
