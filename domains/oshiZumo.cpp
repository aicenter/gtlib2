//
// Created by Matej Sestak on 22.2.19.
//

#include "domains/oshiZumo.h"
#include "oshiZumo.h"


namespace GTLib2 {
namespace domains {

//ACTION

OshiZumoAction::OshiZumoAction(int bid) : Action(bid) {
    this->bid = bid;
}

bool OshiZumoAction::operator==(const Action &that) const {
    if (typeid (*this) == typeid(that)){
        const auto otherAction = static_cast<const OshiZumoAction*>(&that);
        return this->bid == otherAction->bid;
    }
    return false;
}

size_t OshiZumoAction::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, bid);
    return seed;
}
//strating/minBid
OshiZumoDomain::OshiZumoDomain(int startingCoins, int K, int minBid) :
        Domain(static_cast<unsigned int> (minBid == 0 ? startingCoins * 2 : startingCoins/minBid), 2),
        startingCoins(startingCoins), locationK(K), minBid(minBid) {

    auto rootState = make_shared<OshiZumoState>(this, K, startingCoins);
    auto newObservationP1 = make_shared<OshiZumoObservation>(-1);
    auto newObservationP2 = make_shared<OshiZumoObservation>(-1);

    vector<shared_ptr<Observation>> observations{newObservationP1, newObservationP2};
    vector<double> rewards{0.0, 0.0};
    Outcome outcome(rootState, observations, rewards);

    rootStatesDistribution.emplace_back(outcome, 1.0);
}

string OshiZumoDomain::getInfo() const {
    return "Oshi Zumo\nStarting coins: " + std::to_string(startingCoins) +
            "\nLocation K: " + std::to_string(locationK) +
            "\nMinimum bid: " +  std::to_string(minBid) +
            "\nMax depth: " + std::to_string(maxDepth) + '\n';
}



vector<int> OshiZumoDomain::getPlayers() const {
    return {0, 1};
}


OshiZumoState::OshiZumoState(Domain *domain, int wrestlerPosition, int startingCoins) :
        OshiZumoState(domain, wrestlerPosition, startingCoins, startingCoins) {}

OshiZumoState::OshiZumoState(Domain *domain, int wrestlerPosition, int p1Coins, int p2Coins) :
        State(domain), wrestlerLocation(wrestlerPosition), p1Coins(p1Coins), p2Coins(p2Coins) {}


vector<shared_ptr<Action>> OshiZumoState::getAvailableActionsFor(int player) const {
    vector<shared_ptr<Action>> actions;
    const auto OZdomain = static_cast<OshiZumoDomain *>(domain);
    const auto coins = player == 0 ? p1Coins : p2Coins;
    if (coins >= OZdomain->minBid){
        for (int bid = OZdomain->minBid; bid <= coins; bid++){
            actions.push_back(make_shared<OshiZumoAction>(bid));
        }
    } else {
        actions.push_back(make_shared<OshiZumoAction>(0));
    }
    return actions;
}

OutcomeDistribution OshiZumoState::performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const {
    auto p1Action = dynamic_cast<OshiZumoAction*>(actions[0].second.get());
    auto p2Action = dynamic_cast<OshiZumoAction*>(actions[1].second.get());

    assert(p1Action != nullptr && p2Action != nullptr);

    int p1C = this->p1Coins;
    int p2C = this->p2Coins;
    int wLoc = this->wrestlerLocation;

    if (p1Action->bid > p2Action->bid){
        wLoc++;
    } else if (p1Action->bid < p2Action->bid) {
        wLoc--;
    }
    p1C -= p1Action->bid;
    p2C -= p2Action->bid;

    auto newState = make_shared<OshiZumoState>(domain, wLoc, p1C, p2C);
    shared_ptr<OshiZumoObservation> newObserP1 = make_shared<OshiZumoObservation>(p2Action->bid);
    shared_ptr<OshiZumoObservation> newObserP2 = make_shared<OshiZumoObservation>(p1Action->bid);

    vector<shared_ptr<Observation>> observations{newObserP1, newObserP2};
    vector<double> rewards{0.0, 0.0};
    if (newState->isGameEnd()) {
        const auto OZdomain = static_cast<OshiZumoDomain *>(domain);

        if (wLoc < (OZdomain->locationK)) {
            rewards[0] = -1;
            rewards[1] = 1;
        } else if (wLoc > (OZdomain->locationK)) {
            rewards[0] = 1;
            rewards[1] = -1;
        }
    }
    Outcome outcome(newState, observations, rewards);
    OutcomeDistribution distribution;
    distribution.emplace_back(outcome, 1.0);


    return distribution;
}

vector<int> OshiZumoState::getPlayers() const {
    vector<int> players;
    if (!isGameEnd()) {
       players.push_back(0);
       players.push_back(1);
    }
    return players;
}

bool OshiZumoState::isGameEnd() const {
    const auto OZdomain = static_cast<OshiZumoDomain *>(domain);
    return (wrestlerLocation < 0 || wrestlerLocation >= (2*OZdomain->locationK+1)
            || (p1Coins < OZdomain->minBid && p2Coins < OZdomain->minBid));
}
string OshiZumoState::toString() const {
    const auto OZdomain = static_cast<OshiZumoDomain *>(domain);
    string str = "Coins: " +
                to_string(p1Coins) + ", " + to_string(p2Coins) + ", bids: " + "\n\n";
    for (auto i = 0; i < 2 * OZdomain->locationK + 1; i++){
        str+= (i == wrestlerLocation ? "w" : "-");
    }
    str += "\n";
    return str;
}

bool OshiZumoState::operator==(const State &that) const {
    auto other = static_cast<const OshiZumoState &>(that);
    return other.p1Coins == p1Coins && other.p2Coins == p2Coins
        && other.wrestlerLocation == wrestlerLocation;
}

size_t OshiZumoState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, p1Coins);
    boost::hash_combine(seed, p2Coins);
    boost::hash_combine(seed, wrestlerLocation);

    return seed;
}


OshiZumoObservation::OshiZumoObservation(int otherBid) :
        Observation(otherBid), otherBid(otherBid) {}


} // namespace domains
} // namespace GTLib2






