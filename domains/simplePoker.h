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


#ifndef DOMAINS_SIMPLE_POKER_H_
#define DOMAINS_SIMPLE_POKER_H_

#include "base/base.h"

namespace GTLib2::domains {

const int kActionFold = 0;
const int kActionBet = 1;
const int kActionCall = 2;

class SimplePokerAction: public Action {
 public:
    inline SimplePokerAction() : Action(), actionType_(-1) {}
    inline SimplePokerAction(int actionType)
        : Action(actionType == kActionFold ? 0 : 1), actionType_(actionType) {}
    inline string toString() const override {
        switch (actionType_) {
            case kActionFold:
                return "Fold";
            case kActionBet:
                return "Bet";
            case kActionCall:
                return "Call";
            default:
                unreachable("unrecognized action");
        }
    };
    const int actionType_;
};

class SimplePokerDomain: public Domain {
 public:
    explicit SimplePokerDomain();
    inline string getInfo() const override { return "Simple poker game"; };
    vector <Player> getPlayers() const { return {Player(0), Player(1)}; }
};

class SimplePokerObservation: public Observation {
 public:
    inline SimplePokerObservation() : Observation(), playerCard_(-1) {};
    inline SimplePokerObservation(int playerCard)
        : Observation(playerCard), playerCard_(playerCard) {};
    const int playerCard_;
    inline string toString() const override {
        if (playerCard_ == 0) return "J";
        if (playerCard_ == 1) return "Q";
        unreachable("Unrecognized card!");
    };
};


class SimplePokerState: public State {
 public:
    inline SimplePokerState(const Domain *domain,
                            Player actingPlayer,
                            array<int, 2> cards,
                            bool isTerminal) :
        State(domain, hashCombine(3454345434231, actingPlayer, cards, isTerminal)),
        actingPlayer_(actingPlayer),
        cards_(cards),
        isTerminal_(isTerminal) {}

    unsigned long countAvailableActionsFor(Player player) const override;
    vector <shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector <shared_ptr<Action>> &actions) const override;
    inline vector <Player> getPlayers() const override {
        if(isTerminal_) return {};
        else return {actingPlayer_};
    };
    inline bool isTerminal() const override { return isTerminal_; };
    string toString() const override;
    bool operator==(const State &rhs) const override;

    const Player actingPlayer_;
    const array<int, 2> cards_;
    const bool isTerminal_;
};

}  // namespace GTLib2

#endif  // DOMAINS_SIMPLE_POKER_H_

#pragma clang diagnostic pop
