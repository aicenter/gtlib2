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

namespace GTLib2::domains {

struct RandomGameSettings {
  long seed = 0;
  uint32_t maximalDepth = 2;
  int maxBranchingFactor = 2;
  int maxDifferentObservations = 2;
  int maxCenterModification = 2;
  bool binaryUtility = true;
  bool fixedBranchingFactor = true;
};

class RandomGameAction : public Action {
 public:
  explicit RandomGameAction(ActionId id);
  bool operator==(const Action &other) const override;
  size_t getHash() const override;
  string toString() const override;
};

class RandomGameDomain : public Domain {
 public:
  explicit RandomGameDomain(RandomGameSettings settings);

  string getInfo() const override;
  inline vector<Player> getPlayers() const final {
      return {0, 1};
  }

  inline long getSeed() const {
      return seed_;
  }

  inline int getMaxBranchingFactor() const {
      return maxBranchingFactor_;
  }

  inline int getMaxDifferentObservations() const {
      return maxDifferentObservations_;
  }

  bool isBinaryUtility() const {
      return binaryUtility_;
  }

  inline bool isFixedBranchingFactor() const {
      return fixedBranchingFactor_;
  }
  inline int getMaxCenterModification() const {
      return maxCenterModification_;
  }

 private:
  long seed_;
  int maxBranchingFactor_;
  int maxDifferentObservations_;
  bool binaryUtility_;
  bool fixedBranchingFactor_;
  int maxCenterModification_;
};

class RandomGameState : public State {
 public:
  RandomGameState(Domain *domain, int id, vector<long> histories, int center, int depth);
  unsigned long countAvailableActionsFor(Player player) const override;
  vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
  OutcomeDistribution performActions(const vector<PlayerAction> &actions) const override;
  vector<Player> getPlayers() const override;
  string toString() const override;
  bool operator==(const State &other) const override;
  size_t getHash() const override;

 private:
  vector<long> playerHistories_;
  int stateId_; //is it important?
  int center_;
  int depth_;
};

class RandomGameObservation : public Observation {
 public:
  explicit RandomGameObservation(ObservationId id);
};
}
#endif //GTLIB2_RANDOMGAME_H
