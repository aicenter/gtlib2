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

#include "base/base.h"
#include "utils/utils.h"
#include "domains/oshiZumo.h"

namespace GTLib2::domains {

OshiZumoAction::OshiZumoAction(ActionId id, int bid) : Action(id) {
    this->bid_ = bid;
}

bool OshiZumoAction::operator==(const Action &that) const {
    if (typeid(*this) != typeid(that)) {
        return false;
    }
    const auto otherAction = dynamic_cast<const OshiZumoAction &>(that);
    return this->bid_ == otherAction.bid_;
}

string OshiZumoAction::toString() const {
    return "id: " + to_string(id_) +
        ", bid: " + to_string(bid_);
}

size_t OshiZumoAction::getHash() const {
    return bid_;
}

OshiZumoDomain::OshiZumoDomain(OshiZumoSettings settings) :
    Domain(static_cast<unsigned> (settings.minBid == 0 ? settings.startingCoins * 2 :
                                  settings.startingCoins / settings.minBid), 2),
    startingCoins_(settings.startingCoins),
    startingLocation_(settings.startingLocation),
    minBid_(settings.minBid),
    optimalEndGame_(settings.optimalEndGame),
    variant_(settings.variant) {
    assert(settings.startingCoins >= 1);
    assert(settings.startingLoc >= 0);
    assert(settings.minBid >= 0);

    maxUtility_ = startingCoins_ == 1 ? 0.0 : 1.0;

    auto rootState = make_shared<OshiZumoState>(this, startingLocation_, startingCoins_);
    auto publicObs = make_shared<OshiZumoObservation>(NO_BID_OBSERVATION, NO_BID_OBSERVATION, DRAW);
    auto player0Obs = make_shared<OshiZumoObservation>(*publicObs);
    auto player1Obs = make_shared<OshiZumoObservation>(*publicObs);
    vector<double> rewards{0.0, 0.0};

    Outcome outcome(rootState, {player0Obs, player1Obs}, publicObs, rewards);
    rootStatesDistribution_.emplace_back(outcome, 1.0);
}

string OshiZumoDomain::getInfo() const {
    string variant = variant_ ? "IIOshiZumo" : "OshiZumo";
    return "Oshi Zumo"
           "\nVariant: " + variant +
        "\nStarting coins: " + to_string(startingCoins_) +
        "\nStartingLocation: " + to_string(startingLocation_) +
        "\nMinimum bid: " + to_string(minBid_) +
        "\nMax depth: " + to_string(maxDepth_) + '\n';
}

const int OshiZumoDomain::getStartingLocation() const {
    return startingLocation_;
}

const int OshiZumoDomain::getMinBid() const {
    return minBid_;
}

const bool OshiZumoDomain::isOptimalEndGame() const {
    return optimalEndGame_;
}

const OshiZumoVariant OshiZumoDomain::getVariant() const {
    return variant_;
}
const int OshiZumoDomain::getStartingCoins() const {
    return startingCoins_;
}

OshiZumoState::OshiZumoState(Domain *domain, int wrestlerPosition, int startingCoins) :
    OshiZumoState(domain, wrestlerPosition, {startingCoins, startingCoins}) {}

OshiZumoState::OshiZumoState(Domain *domain, int wrestlerPosition, vector<int> coinsPerPlayer) :
    State(domain), wrestlerLocation_(wrestlerPosition) {
    this->coins_ = move(coinsPerPlayer);
}

vector<shared_ptr<Action>> OshiZumoState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;
    const auto OZdomain = dynamic_cast<OshiZumoDomain *>(domain_);
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

unsigned long OshiZumoState::countAvailableActionsFor(Player player) const {
    const auto OZdomain = dynamic_cast<OshiZumoDomain *>(domain_);
    if (coins_[player] >= OZdomain->getMinBid()) {
        return coins_[player] - OZdomain->getMinBid() + 1;
    } else {
        return 1;
    }
}

OutcomeDistribution OshiZumoState::performActions(const vector<PlayerAction> &actions) const {
    auto player0Action = dynamic_cast<OshiZumoAction *>(actions[0].second.get());
    auto player1Action = dynamic_cast<OshiZumoAction *>(actions[1].second.get());
    const auto OZdomain = dynamic_cast<OshiZumoDomain *>(domain_);
    assert(player0Action != nullptr && player1Action != nullptr);

    vector<int> newCoins(coins_);
    int newWrestlerLocation = this->wrestlerLocation_;

    OshiZumoRoundOutcome roundResult = DRAW;
    if (player0Action->getBid() > player1Action->getBid()) {
        newWrestlerLocation++;
        roundResult = PLAYER0_WIN;
    } else if (player0Action->getBid() < player1Action->getBid()) {
        newWrestlerLocation--;
        roundResult = PLAYER0_LOSE;
    }
    newCoins[0] -= player0Action->getBid();
    newCoins[1] -= player1Action->getBid();

    //if optimalEndGame is allowed, check if wrestler is on board
    // if any player cannot make any legal bid from now on, simulate rest of the game,
    // as opponent plays optimal
    if (OZdomain->isOptimalEndGame()
        && newWrestlerLocation >= 0
        && newWrestlerLocation <= (2 * OZdomain->getStartingLocation())
        && (newCoins[0] == 0 || newCoins[1] == 0 ||
            newCoins[0] < OZdomain->getMinBid() ||
            newCoins[1] < OZdomain->getMinBid())) {

        int minBid = OZdomain->getMinBid() == 0 ? 1 : OZdomain->getMinBid();
        newWrestlerLocation += newCoins[0] / minBid;
        newWrestlerLocation -= newCoins[1] / minBid;
        newCoins[0] = newCoins[1] = 0;
    }

    auto newState = make_shared<OshiZumoState>(domain_, newWrestlerLocation, newCoins);

    shared_ptr<OshiZumoObservation> publicObs;
    shared_ptr<OshiZumoObservation> player0Obs;
    shared_ptr<OshiZumoObservation> player1Obs;

    if (OZdomain->getVariant() == IncompleteObservation) {
        publicObs =
            make_shared<OshiZumoObservation>(NO_BID_OBSERVATION, NO_BID_OBSERVATION, roundResult);
        player0Obs = make_shared<OshiZumoObservation>(player0Action->getBid(),
                                                      NO_BID_OBSERVATION,
                                                      roundResult);
        player1Obs = make_shared<OshiZumoObservation>(NO_BID_OBSERVATION,
                                                      player1Action->getBid(),
                                                      roundResult);
    } else {
        publicObs = make_shared<OshiZumoObservation>(player0Action->getBid(),
                                                     player1Action->getBid(),
                                                     roundResult);
        player0Obs = make_shared<OshiZumoObservation>(*publicObs);
        player1Obs = make_shared<OshiZumoObservation>(*publicObs);
    }

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

    Outcome outcome(newState, {player0Obs, player1Obs}, publicObs, rewards);
    OutcomeDistribution distribution;
    distribution.emplace_back(outcome, 1.0);
    return distribution;
}

vector<Player> OshiZumoState::getPlayers() const {
    if (isGameEnd()) return {};
    return {0, 1};
}

bool OshiZumoState::isGameEnd() const {
    const auto OZdomain = dynamic_cast<OshiZumoDomain *>(domain_);
    return (wrestlerLocation_ < 0
        || wrestlerLocation_ > (2 * OZdomain->getStartingLocation())
        || (coins_[0] < OZdomain->getMinBid() && coins_[1] < OZdomain->getMinBid())
        || (coins_[0] == 0 && coins_[1] == 0)); // if minBid_ == 0
}

string OshiZumoState::toString() const {
    const auto OZdomain = dynamic_cast<OshiZumoDomain *>(domain_);
    string str = "Coins: " +
        to_string(coins_[0]) + ", " + to_string(coins_[1]) + ", bids: " + "\n\n";
    for (auto i = 0; i < 2 * OZdomain->getStartingLocation() + 1; i++) {
        str += (i == wrestlerLocation_ ? "w" : "-");
    }
    str += "\n";
    return str;
}

bool OshiZumoState::operator==(const State &rhs) const {
    auto ozState = dynamic_cast<const OshiZumoState &>(rhs);

    return ozState.coins_[0] == coins_[0]
        && ozState.coins_[1] == coins_[1]
        && ozState.wrestlerLocation_ == wrestlerLocation_;
}

size_t OshiZumoState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, coins_[0]);
    boost::hash_combine(seed, coins_[1]);
    boost::hash_combine(seed, wrestlerLocation_);
    return seed;
}

int OshiZumoState::getWrestlerLocation() const {
    return wrestlerLocation_;
}

const vector<int> &OshiZumoState::getCoins() const {
    return coins_;
}

OshiZumoObservation::OshiZumoObservation(int player0Bid,
                                         int player1Bid,
                                         OshiZumoRoundOutcome roundResult) :
    Observation(),
    player0Bid_(player0Bid),
    player1Bid_(player1Bid),
    roundResult_(roundResult) {
    //playerBid + 1 -> 0, 1, 2,...
    //using Szudzik pairing function to get unique IDs for different observations
    unsigned int a = player0Bid_ + 1;
    unsigned int b = player1Bid_ + 1;
    unsigned int mapping = elegantPair(a, b);
    id_ = (roundResult_ + 1) | (mapping << 2);
}

} // namespace GTLib2






