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
#ifndef GTLIB2_RANDOMGAME_H
#define GTLIB2_RANDOMGAME_H

#include "base/base.h"
#include "utils/highQualityRandom.h"

namespace GTLib2{
namespace domains{

class RandomGameAction : public Action {
public:
    explicit RandomGameAction(ActionId id);
//    bool operator==(const Action &other) const override;
//    size_t getHash() const override;
//    string toString() const;

private:
    string value_;
};

class RandomGameDomain : public Domain {
public:
    RandomGameDomain(unsigned int maxDepth, long seed, int maxBranchingFactor,
                     int maxDifferentObservations, bool binaryUtility,
                     bool fixedBranchingFactor, int maxCenterModification);
    string getInfo() const override;
    inline vector<Player> getPlayers() const final{
        return {0, 1};
    }

    inline long getSeed() const{
        return seed_;
    }

    inline int getMaxBranchingFactor() const{
        return maxBranchingFactor_;
    }

    inline int getMaxDifferentObservations() const{
        return maxDifferentObservations_;
    }

    bool isBinaryUtility() const{
        return binary_utility_;
    }

    inline bool isFixedBranchingFactor() const{
        return fixedBranchingFactor_;
    }
    inline int getMaxCenterModification() const {
        return maxCenterModification_;
    }

private:
    std::mt19937 random_;
    long seed_;
    int maxBranchingFactor_;
    int maxDifferentObservations_;
    bool binary_utility_;
    bool fixedBranchingFactor_;
    int maxCenterModification_;
};

class RandomGameState : public State {
public:
    RandomGameState(Domain *domain, int id, int center, int depth);
    unsigned long countAvailableActionsFor(Player player) const override;
    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector<PlayerAction> &actions) const override;
    vector<Player> getPlayers() const override;
//    string toString() const override;
//    bool operator==(const State &other) const override;
    size_t getHash() const override;

private:
    int ID_;
    int center_;
    int depth_;
};

class RandomGameObservation : public Observation {
public:
    explicit RandomGameObservation(ObservationId id);
};
}
}
#endif //GTLIB2_RANDOMGAME_H
