//
// Created by Matej Sestak on 22.2.19.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef GTLIB2_OSHIZUMO_H
#define GTLIB2_OSHIZUMO_H

#include <experimental/optional>
#include "base/base.h"

using std::experimental::optional;

namespace GTLib2{
namespace domains{

class OshiZumoAction : public Action {
public:
    explicit OshiZumoAction(int bid);
    bool operator==(const Action &that) const override;
    size_t getHash() const override;

    int bid;
};

class OshiZumoDomain : public Domain {
public:
    OshiZumoDomain(int startingCoins, int K, int minBid);
    string getInfo() const override;
    vector<int> getPlayers() const override;
    const int startingCoins;
    const int locationK;
    const int minBid;
};

class OshiZumoState : public State {
public:
    OshiZumoState(Domain *domain, int wrestlerPosition, int startingCoins);
    OshiZumoState(Domain *domain, int wrestlerPosition,
                    int p1Coins, int p2Coins);
    vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;
    OutcomeDistribution
    performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const override;
    vector<int> getPlayers() const override;
    bool isGameEnd() const;
    string toString() const override;
    bool operator==(const State &that) const override;
    size_t getHash() const override;

    int wrestlerLocation;
    int p1Coins;
    int p2Coins;

};

class OshiZumoObservation : public Observation {
public:
    explicit OshiZumoObservation(int otherBid);


    int otherBid;

};
} // namespace domains
} // namespace GTLib2


#endif //GTLIB2_OSHIZUMO_H
