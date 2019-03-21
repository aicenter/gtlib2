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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef GTLIB2_OSHIZUMO_H
#define GTLIB2_OSHIZUMO_H

#include "base/base.h"

namespace GTLib2{
namespace domains{

class OshiZumoAction : public Action {
public:
    explicit OshiZumoAction(ActionId id, int bid);
    bool operator==(const Action &that) const override;
    size_t getHash() const override;

    int bid;
};

class OshiZumoDomain : public Domain {
public:
    OshiZumoDomain(int startingCoins, int startingLoc, int minBid);
    string getInfo() const override;
    vector<Player> getPlayers() const override;
    const int startingCoins;
    // if startingLocation = 3, then there is (2*3 + 1) = 7 locations:   ---W---
    // wrestler starts in the middle(3):                                 0123456
    const int startingLocation;
    const int minBid;
};

class OshiZumoState : public State {
public:
    OshiZumoState(Domain *domain, int wrestlerPosition, int startingCoins);
    OshiZumoState(Domain *domain, int wrestlerPosition, vector<int> coins);
    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution
    performActions(const vector<PlayerAction> &actions) const override;
    vector<Player> getPlayers() const override;
    bool isGameEnd() const;
    string toString() const override;
    bool operator==(const State &that) const override;
    size_t getHash() const override;

    int wrestlerLocation;
    vector<int> coins;

};

class OshiZumoObservation : public Observation {
public:
    explicit OshiZumoObservation(int otherBid);

    int otherBid;

};
} // namespace domains
} // namespace GTLib2


#endif //GTLIB2_OSHIZUMO_H
