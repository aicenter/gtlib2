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

/*
 * Oshi Zumo domain implementation. For game rules go to: http://mlanctot.info/files/papers/aij-2psimmove.pdf, p.43
 */
namespace GTLib2::domains {

enum OshiZumoVariant { CompleteObservation, IncompleteObservation };

/**
 * @param startingCoins number of coins each player has at beginning of a game
 * @param startingLocation starting position of wrestler, if startingLoc = 3, then there is (2*3 + 1) = 7 locations, wrestler is in the middle
 * @param minBid minimum allowed bid by player per round
 * @param optimalEndGame allow to simulate end optimal game, if one of the players can only bid 0
 */
struct OshiZumoSettings {
  OshiZumoVariant variant = CompleteObservation;
  int startingCoins = 3;
  int startingLocation = 3;
  int minBid = 1;
  bool optimalEndGame = true;
};

class OshiZumoAction : public Action {
 public:
  explicit OshiZumoAction(ActionId id, int bid);
  bool operator==(const Action &that) const override;
  size_t getHash() const override;
  string toString() const override;
  inline int getBid() const {
    return bid_;
  }

 private:
  int bid_;
};

class OshiZumoDomain : public Domain {
 public:
  explicit OshiZumoDomain(OshiZumoSettings settings);
  string getInfo() const override;
  inline vector<Player> getPlayers() const final{
    return {0, 1};
  }
  const int getStartingLocation() const;
  const int getMinBid() const;
  const bool isOptimalEndGame() const;
  const OshiZumoVariant getVariant() const;
  const int getStartingCoins() const;

 private:
  const int startingCoins_;
  const int startingLocation_;
  const int minBid_;
  const bool optimalEndGame_;
  const OshiZumoVariant variant_;
};

class OshiZumoState : public State {
 public:
  OshiZumoState(Domain *domain, int wrestlerPosition, int startingCoins);
  OshiZumoState(Domain *domain, int wrestlerPosition, vector<int> coinsPerPlayer);

  vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
  unsigned long countAvailableActionsFor(Player player) const override;
  OutcomeDistribution performActions(const vector<PlayerAction> &actions) const override;
  vector<Player> getPlayers() const override;
  bool isGameEnd() const;
  string toString() const override;
  bool operator==(const State &rhs) const override;
  size_t getHash() const override;
  int getWrestlerLocation() const;
  const vector<int> &getCoins() const;

 protected:
  int wrestlerLocation_;
  vector<int> coins_;

};

constexpr int NO_BID_OBSERVATION = -1;
enum OshiZumoRoundOutcome {
  DRAW = 0,
  PLAYER0_WIN = 1,
  PLAYER0_LOSE = -1,
};

class OshiZumoObservation : public Observation {
 public:
  explicit OshiZumoObservation(int player0Bid,
                               int player1Bid,
                               OshiZumoRoundOutcome roundResult);

 private:
  const int player0Bid_;
  const int player1Bid_;
  const OshiZumoRoundOutcome roundResult_;

};
} // namespace GTLib2
#endif //GTLIB2_OSHIZUMO_H
