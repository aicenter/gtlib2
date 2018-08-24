//
// Created by Pavel Rytir on 08/01/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef DOMAINS_MATCHING_PENNIES_H_
#define DOMAINS_MATCHING_PENNIES_H_

#include "../base/base.h"

namespace GTLib2 {
namespace domains {
class MatchingPenniesDomain : public Domain {
 public:
  MatchingPenniesDomain();
  vector<int> getPlayers() const override;

  string getInfo() const override { return "Matching pennies"; }
};

class SimultaneousMatchingPenniesDomain : public Domain {
 public:
  SimultaneousMatchingPenniesDomain();
  vector<int> getPlayers() const override;

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

  Move move;
};

class MatchingPenniesObservation : public Observation {
 public:
  explicit MatchingPenniesObservation(OtherMove otherMoveParm);

  OtherMove otherMove;
};

class MatchingPenniesState : public State {
 public:
  MatchingPenniesState(Domain *domain, Move p1, Move p2);

  vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;

  OutcomeDistribution
  performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const override;

  int getNumberOfPlayers() const override;
  vector<int> getPlayers() const override;
  size_t getHash() const override;

  bool operator==(const State &rhs) const override;

  vector<int> players;
  Move player1;
  Move player2;

  // ToString returns state description
  string toString() const override;
};

class SimultaneousMatchingPenniesState : public State {
 public:
  SimultaneousMatchingPenniesState(Domain *domain, Move p1, Move p2);

  vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;

  OutcomeDistribution
  performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const override;

  int getNumberOfPlayers() const override;

  vector<int> getPlayers() const override;

  size_t getHash() const override;

  vector<int> players;
  Move player1;
  Move player2;

  // ToString returns state description
  string toString() const override;
};
}  // namespace domains
}  // namespace GTLib2
#endif  // DOMAINS_MATCHING_PENNIES_H_

#pragma clang diagnostic pop
