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
#ifndef DOMAINS_MATCHING_PENNIES_H_
#define DOMAINS_MATCHING_PENNIES_H_

#include <utility>
#include <vector>
#include <string>
#include "base/base.h"

namespace GTLib2::domains {
class MatchingPenniesDomain : public Domain {
 public:
  MatchingPenniesDomain();
  vector<Player> getPlayers() const override;

  string getInfo() const override { return "Matching pennies"; }
};

class SimultaneousMatchingPenniesDomain : public Domain {
 public:
  SimultaneousMatchingPenniesDomain();
  vector<Player> getPlayers() const override;

  string getInfo() const override { return "Matching pennies"; }
};

enum Move {
  Heads, Tails, Nothing
};
enum OtherMove {
  OtherHeads, OtherTails, OtherNothing
};

class MatchingPenniesAction : public Action {
 public:
  explicit MatchingPenniesAction(Move moveParm);

  string toString() const override;
  bool operator==(const Action &that) const override;
  size_t getHash() const override;

  Move move_;
};

class MatchingPenniesObservation : public Observation {
 public:
  explicit MatchingPenniesObservation(OtherMove otherMoveParm);

  OtherMove otherMove_;
};

class MatchingPenniesState : public State {
 public:
  MatchingPenniesState(Domain *domain, Move p1, Move p2);

  unsigned long countAvailableActionsFor(Player player) const override;
  vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;

  OutcomeDistribution
  performActions(const vector<PlayerAction> &actions) const override;

  int getNumberOfPlayers() const override;
  vector<Player> getPlayers() const override;
  size_t getHash() const override;

  bool operator==(const State &rhs) const override;

  vector<Player> players_;
  Move player1_;
  Move player2_;

  // ToString returns state description
  string toString() const override;
};

class SimultaneousMatchingPenniesState : public State {
 public:
  SimultaneousMatchingPenniesState(Domain *domain, Move p1, Move p2);

  unsigned long countAvailableActionsFor(Player player) const override;
  vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;

  OutcomeDistribution
  performActions(const vector<PlayerAction> &actions) const override;

  int getNumberOfPlayers() const override;

  vector<Player> getPlayers() const override;

  size_t getHash() const override;

  vector<Player> players_;
  Move player1_;
  Move player2_;

  // ToString returns state description
  string toString() const override;
};
}  // namespace GTLib2
#endif  // DOMAINS_MATCHING_PENNIES_H_

#pragma clang diagnostic pop
