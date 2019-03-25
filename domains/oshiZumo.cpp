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
    this->bid_ = bid;
}


bool OshiZumoAction::operator==(const Action &that) const {
    if (typeid(*this) != typeid(that)) {
        return false;
    }
    const auto otherAction = static_cast<const OshiZumoAction *>(&that);

    return (this->bid_ == otherAction->bid_
            && this->id == otherAction->id);
}


size_t OshiZumoAction::getHash() const {
    std::hash<size_t> h;
    return h(bid_);
}


OshiZumoDomain::OshiZumoDomain(int startingCoins, int startingLoc, int minBid) :
        OshiZumoDomain(startingCoins, startingLoc, minBid, true) {}


OshiZumoDomain::OshiZumoDomain(int startingCoins, int startingLoc, int minBid, bool optimalEndGme) :
        Domain(static_cast<unsigned int> (minBid == 0 ? startingCoins * 2 : startingCoins / minBid), 2),
        startingCoins_(startingCoins), startingLocation_(startingLoc),
        minBid_(minBid), optimalEndGame_(optimalEndGme) {
    assert(startingCoins >= 1);
    assert(startingLoc >= 0);
    assert(minBid >= 0);

    auto rootState = make_shared<OshiZumoState>(this, startingLoc, startingCoins);
    vector<shared_ptr<Observation>> observations{make_shared<OshiZumoObservation>(NO_OBSERVATION),
                                                 make_shared<OshiZumoObservation>(NO_OBSERVATION)};
    vector<double> rewards{0.0, 0.0};
    Outcome outcome(rootState, observations, rewards);

    rootStatesDistribution.emplace_back(outcome, 1.0);
}


string OshiZumoDomain::getInfo() const {
    return "Oshi Zumo"
           "\nStarting coins_: " + std::to_string(startingCoins_) +
           "\nStartingLocation: " + std::to_string(startingLocation_) +
           "\nMinimum bid: " + std::to_string(minBid_) +
           "\nMax depth: " + std::to_string(maxDepth) + '\n';
}


OshiZumoState::OshiZumoState(Domain *domain, int wrestlerPosition, int startingCoins) :
        OshiZumoState(domain, wrestlerPosition, {startingCoins, startingCoins}) {}


OshiZumoState::OshiZumoState(Domain *domain, int wrestlerPosition, vector<int> coinsPerPlayer) :
        State(domain), wrestlerLocation_(wrestlerPosition) {
    this->coins_ = move(coinsPerPlayer);
}


vector<shared_ptr<Action>> OshiZumoState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;
    const auto OZdomain = static_cast<OshiZumoDomain *>(domain);
    int id = 0;
    if (coins_[player] >= OZdomain->getMinBid()) {
        for (int bid = OZdomain->getMinBid(); bid <= coins_[player]; bid++) {
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

    vector<int> newCoins(coins_);
    int newWrestlerLocation = this->wrestlerLocation_;

    if (p1Action->getBid() > p2Action->getBid()) {
        newWrestlerLocation++;
    } else if (p1Action->getBid() < p2Action->getBid()) {
        newWrestlerLocation--;
    }
    newCoins[0] -= p1Action->getBid();
    newCoins[1] -= p2Action->getBid();

    //if optimalEndGame is allowed, check if wrestler is on board
    if (OZdomain->isOptimalEndGame()
        && newWrestlerLocation >= 0
        && newWrestlerLocation <= (2 * OZdomain->getStartingLocation())) {
        // if any player cannot make any legal bid from now on, simulate rest of the game,
        // as opponent plays optimal
        if (newCoins[0] == 0 || newCoins[1] == 0 || newCoins[0] < OZdomain->getMinBid() ||
            newCoins[1] < OZdomain->getMinBid()) {
            int minBid = OZdomain->getMinBid() == 0 ? 1 : OZdomain->getMinBid();
            newWrestlerLocation += newCoins[0] / minBid;
            newWrestlerLocation -= newCoins[1] / minBid;
            newCoins[0] = newCoins[1] = 0;
        }
    }

    auto newState = make_shared<OshiZumoState>(domain, newWrestlerLocation, newCoins);
    shared_ptr<OshiZumoObservation> newObserP1 = make_shared<OshiZumoObservation>(p2Action->getBid());
    shared_ptr<OshiZumoObservation> newObserP2 = make_shared<OshiZumoObservation>(p1Action->getBid());
    vector<shared_ptr<Observation>> observations{newObserP1, newObserP2};

    vector<double> rewards{0.0, 0.0};
    if (newState->isGameEnd()) {
        if (newWrestlerLocation < (OZdomain->getStartingLocation())) {
            rewards[0] = -1;
            rewards[1] = 1;
        } else if (newWrestlerLocation > (OZdomain->getStartingLocation())) {
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
    return (wrestlerLocation_ < 0
            || wrestlerLocation_ > (2 * OZdomain->getStartingLocation())
            || (coins_[0] < OZdomain->getMinBid() && coins_[1] < OZdomain->getMinBid())
            || (coins_[0] == 0 && coins_[1] == 0)); // if minBid_ == 0
}

string OshiZumoState::toString() const {
    const auto OZdomain = static_cast<OshiZumoDomain *>(domain);
    string str = "Coins: " +
                 to_string(coins_[0]) + ", " + to_string(coins_[1]) + ", bids: " + "\n\n";
    for (auto i = 0; i < 2 * OZdomain->getStartingLocation() + 1; i++) {
        str += (i == wrestlerLocation_ ? "w" : "-");
    }
    str += "\n";
    return str;
}


bool OshiZumoState::operator==(const State &that) const {
    auto other = static_cast<const OshiZumoState &>(that);
    return other.coins_[0] == coins_[0]
           && other.coins_[1] == coins_[1]
           && other.wrestlerLocation_ == wrestlerLocation_;
}


size_t OshiZumoState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, coins_[0]);
    boost::hash_combine(seed, coins_[1]);
    boost::hash_combine(seed, wrestlerLocation_);
    return seed;
}


OshiZumoObservation::OshiZumoObservation(int opponentBid) :
        Observation(opponentBid), opponentBid_(opponentBid) {}

} // namespace domains
} // namespace GTLib2






