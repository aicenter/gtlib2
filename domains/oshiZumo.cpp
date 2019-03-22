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

#include "domains/oshiZumo.h"

namespace GTLib2 {
namespace domains {


OshiZumoAction::OshiZumoAction(ActionId id, int bid) : Action(id) {
    this->bid = bid;
}


bool OshiZumoAction::operator==(const Action &that) const {
    if (typeid(*this) == typeid(that)) {
        const auto otherAction = static_cast<const OshiZumoAction *>(&that);

        return (this->bid == otherAction->bid
                && this->id == otherAction->id);
    }
    return false;
}


size_t OshiZumoAction::getHash() const {
    std::hash<size_t> h;
    return h(bid);
}


OshiZumoDomain::OshiZumoDomain(int startingCoins, int startingLoc, int minBid) :
        OshiZumoDomain(startingCoins, startingLoc, minBid, true) {}


OshiZumoDomain::OshiZumoDomain(int startingCoins, int startingLoc, int minBid, bool optimalEndGme) :
        Domain(static_cast<unsigned int> (minBid == 0 ? startingCoins * 2 : startingCoins / minBid), 2),
        startingCoins(startingCoins), startingLocation(startingLoc),
        minBid(minBid) , optimalEndGame(optimalEndGme){

    auto rootState = make_shared<OshiZumoState>(this, startingLoc, startingCoins);
    auto newObservationP1 = make_shared<OshiZumoObservation>(NO_OBSERVATION);
    auto newObservationP2 = make_shared<OshiZumoObservation>(NO_OBSERVATION);

    vector<shared_ptr<Observation>> observations{newObservationP1, newObservationP2};
    vector<double> rewards{0.0, 0.0};
    Outcome outcome(rootState, observations, rewards);

    rootStatesDistribution.emplace_back(outcome, 1.0);
}


string OshiZumoDomain::getInfo() const {
    return "Oshi Zumo\nStarting coins: " + std::to_string(startingCoins) +
           "\nStartingLocation: " + std::to_string(startingLocation) +
           "\nMinimum bid: " + std::to_string(minBid) +
           "\nMax depth: " + std::to_string(maxDepth) + '\n';
}


vector<Player> OshiZumoDomain::getPlayers() const {
    return {0, 1};
}


OshiZumoState::OshiZumoState(Domain *domain, int wrestlerPosition, int startingCoins) :
        OshiZumoState(domain, wrestlerPosition, {startingCoins, startingCoins}) {}


OshiZumoState::OshiZumoState(Domain *domain, int wrestlerPosition, vector<int> coins) :
        State(domain), wrestlerLocation(wrestlerPosition) {
    this->coins = move(coins);
}


vector<shared_ptr<Action>> OshiZumoState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;
    const auto OZdomain = static_cast<OshiZumoDomain *>(domain);
    int id = 0;
    if (coins[player] >= OZdomain->minBid) {
        for (int bid = OZdomain->minBid; bid <= coins[player]; bid++) {
            actions.push_back(make_shared<OshiZumoAction>(id, bid));
            id++;
        }
    } else {
        actions.push_back(make_shared<OshiZumoAction>(id, 0));
    }
    return actions;
}


OutcomeDistribution OshiZumoState::performActions(const vector<PlayerAction> &actions) const {
    auto p1Action = dynamic_cast<OshiZumoAction *>(actions[0].second.get());
    auto p2Action = dynamic_cast<OshiZumoAction *>(actions[1].second.get());
    const auto OZdomain = static_cast<OshiZumoDomain *>(domain);
    assert(p1Action != nullptr && p2Action != nullptr);

    vector<int> newCoins(coins);
    int newWrestlerLocation = this->wrestlerLocation;

    if (p1Action->bid > p2Action->bid) {
        newWrestlerLocation++;
    } else if (p1Action->bid < p2Action->bid) {
        newWrestlerLocation--;
    }
    newCoins[0] -= p1Action->bid;
    newCoins[1] -= p2Action->bid;

    //if optimalEndGame is allowed, check if wrestler is on board
    if (OZdomain->optimalEndGame && newWrestlerLocation >= 0 && newWrestlerLocation <= (2 * OZdomain->startingLocation)) {
        // if any player cannot make any legal bid from now on, simulate rest of the game,
        // as opponent plays optimal
        if (newCoins[0] == 0 || newCoins[1] == 0 || newCoins[0] < OZdomain->minBid || newCoins[1] < OZdomain->minBid) {
            int minBid = OZdomain->minBid == 0 ? 1 : OZdomain->minBid;
            newWrestlerLocation += newCoins[0] / minBid;
            newWrestlerLocation -= newCoins[1] / minBid;
            newCoins[0] = newCoins[1] = 0;
        }
    }

    auto newState = make_shared<OshiZumoState>(domain, newWrestlerLocation, newCoins);
    shared_ptr<OshiZumoObservation> newObserP1 = make_shared<OshiZumoObservation>(p2Action->bid);
    shared_ptr<OshiZumoObservation> newObserP2 = make_shared<OshiZumoObservation>(p1Action->bid);
    vector<shared_ptr<Observation>> observations{newObserP1, newObserP2};

    vector<double> rewards{0.0, 0.0};
    if (newState->isGameEnd()) {
        if (newWrestlerLocation < (OZdomain->startingLocation)) {
            rewards[0] = -1;
            rewards[1] = 1;
        } else if (newWrestlerLocation > (OZdomain->startingLocation)) {
            rewards[0] = 1;
            rewards[1] = -1;
        }
    }
    Outcome outcome(newState, observations, rewards);
    OutcomeDistribution distribution;
    distribution.emplace_back(outcome, 1.0);

    return distribution;
}


vector<Player> OshiZumoState::getPlayers() const {
    if (isGameEnd()) return {};
    return {0, 1};
}

bool OshiZumoState::isGameEnd() const {
    const auto OZdomain = static_cast<OshiZumoDomain *>(domain);
    return (wrestlerLocation < 0
            || wrestlerLocation > (2 * OZdomain->startingLocation)
            || (coins[0] < OZdomain->minBid && coins[1] < OZdomain->minBid)
            || (coins[0] == 0 && coins[1] == 0)); // if minBid == 0
}

string OshiZumoState::toString() const {
    const auto OZdomain = static_cast<OshiZumoDomain *>(domain);
    string str = "Coins: " +
                 to_string(coins[0]) + ", " + to_string(coins[1]) + ", bids: " + "\n\n";
    for (auto i = 0; i < 2 * OZdomain->startingLocation + 1; i++) {
        str += (i == wrestlerLocation ? "w" : "-");
    }
    str += "\n";
    return str;
}


bool OshiZumoState::operator==(const State &that) const {
    auto other = static_cast<const OshiZumoState &>(that);
    return other.coins[0] == coins[0]
           && other.coins[1] == coins[1]
           && other.wrestlerLocation == wrestlerLocation;
}


size_t OshiZumoState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, coins[0]);
    boost::hash_combine(seed, coins[1]);
    boost::hash_combine(seed, wrestlerLocation);
    return seed;
}


OshiZumoObservation::OshiZumoObservation(int otherBid) :
        Observation(otherBid), otherBid(otherBid) {}

} // namespace domains
} // namespace GTLib2






