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

#ifndef GTLIB2_RANDOMGAME_H
#define GTLIB2_RANDOMGAME_H

#include "base/base.h"
#include "utils/utils.h"


/**
 * Random game, players move simultaneously.
 */
namespace GTLib2::domains {

/**
 * Settings for random game
 * @var seed
 * @var maxDepth                depth of the game
 * @var maxBranchingFactor      max possible actions in each state for player, min 2
 * @var maxDifferentObservation max observation for player in each state, if == maxBranching factor, it is perfect observation game
 * @var maxRewardModification   max reward to gain in each state
 * @var maxUtility              max possible utility, only used, if binary utility and utility correlation are false
 * @var binaryUtility           if true, utility only -1, 0, 1
 * @var utilityCorrelation      if true, rewards are gained in each state, otherwise there are randomly generated in terminal nodes
 * @var fixedBranchingFactor    if true, there is exactly maxBranchingFactor actions in each state
 */
struct RandomGameSettings {
    long seed = 0;
    uint32_t maxDepth = 2;
    uint32_t maxBranchingFactor = 2;
    uint32_t maxDifferentObservations = 2;
    int maxRewardModification = 2;
    uint32_t maxUtility = 100;
    bool binaryUtility = true;
    bool utilityCorrelation = true;
    bool fixedBranchingFactor = true;
};

class RandomGameAction: public Action {
 public:
    inline RandomGameAction() : Action() {};
    inline explicit RandomGameAction(ActionId id) : Action(id) {};
    bool operator==(const Action &other) const override;
};

class RandomGameDomain: public Domain {
 public:
    explicit RandomGameDomain(RandomGameSettings settings);
    string getInfo() const override;
    inline vector<Player> getPlayers() const { return {0, 1}; }
    inline uint32_t getMaxBranchingFactor() const { return maxBranchingFactor_; }
    inline uint32_t getMaxDifferentObservations() const { return maxDifferentObservations_; }
    inline bool isBinaryUtility() const { return binaryUtility_; }
    inline bool isFixedBranchingFactor() const { return fixedBranchingFactor_; }
    inline int getMaxRewardModification() const { return maxRewardModification_; }
    inline bool isUtilityCorrelation() const { return utilityCorrelation_; }

 private:
    const long seed_;
    const uint32_t maxBranchingFactor_;
    const uint32_t maxDifferentObservations_;
    const int maxRewardModification_;
    const bool binaryUtility_;
    const bool fixedBranchingFactor_;
    const bool utilityCorrelation_;
};

class RandomGameState: public State {
 public:
    RandomGameState(const Domain *domain, int stateSeed, vector<long> playerSeeds,
                    double cumulativeReward, unsigned int depth) :
        State(domain, hashCombine(75623196115, stateSeed, playerSeeds, cumulativeReward, depth)),
        stateSeed_(stateSeed),
        cumulativeReward_(cumulativeReward),
        depth_(depth),
        playerActionSeeds_(move(playerSeeds)) {}
    unsigned long countAvailableActionsFor(Player player) const override;
    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector<shared_ptr<Action>> &actions) const override;
    string toString() const override;
    bool operator==(const State &other) const override;
    bool isTerminal() const override;
    inline vector<Player> getPlayers() const override {
        if (isTerminal()) return {};
        return {0, 1};
    }
 private:
    const long stateSeed_; // seed used for reward generating
    const double cumulativeReward_;
    const unsigned int depth_;
    // simple AOH history for each player, used as seed for move generating
    const vector<long> playerActionSeeds_;
};

class RandomGameObservation: public Observation {
 public:
    inline RandomGameObservation() : Observation() {};
    inline explicit RandomGameObservation(ObservationId id) : Observation(id) {};
};
} // GTLib2::domains
#endif //GTLIB2_RANDOMGAME_H
