//
// Created by Jakub Rozlivek on 14.08.2017.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef BASE_EFG_H_
#define BASE_EFG_H_

#include <experimental/optional>
#include <unordered_map>
#include <vector>
#include <utility>
#include <string>
#include <functional>
#include "base/base.h"

using std::unordered_map;
using std::experimental::nullopt;
using std::experimental::optional;

namespace GTLib2 {

class EFGNode;
typedef pair<shared_ptr<EFGNode>, double> EFGDistEntry;
typedef vector<EFGDistEntry> EFGNodesDistribution;

/**
 * EFGNode is a class that represents node in an extensive form game (EFG),
 * which contains
 * - action-observation history,
 * - state,
 * - rewards (utility) and
 * - information set.
 *
 * There are three types of EFGNodes:
 * - Player (inner) nodes
 * - Chance/nature (inner) nodes
 * - Terminal nodes
 */
class EFGNode final : public std::enable_shared_from_this<EFGNode const> {
 public:
  // Constructor for the same round node
  EFGNode(shared_ptr<EFGNode const> parent,
          const vector<pair<int, shared_ptr<Action>>> &performedActions,
          shared_ptr<Action> incomingAction, int depth);

  // Constructor for the new round node
  EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
          const vector<shared_ptr<Observation>> &observations,
          const vector<double> &rewards,
          double natureProbability, shared_ptr<Action> incomingAction, int depth);

  // Returns the sequence of actions performed by the player since the root.
  ActionSequence getActionsSeqOfPlayer(int player) const;

  double getProbabilityOfActionsSeqOfPlayer(int player, const BehavioralStrategy &strat) const;

  // Returns available actions for the current player
  vector<shared_ptr<Action>> availableActions() const;

  // Perform the given action and returns the next node
  // or nodes in case of stochastic games together with the probabilities.
  EFGNodesDistribution performAction(const shared_ptr<Action> &action) const;

  // Gets the information set of the node represented as ActionObservationHistory set.
  shared_ptr<AOH> getAOHInfSet() const;

  // Check if the node is in the given information set.
  bool isContainedInInformationSet(const shared_ptr<AOH> &infSet) const;

  // Gets the parent efg node.
  shared_ptr<EFGNode const> getParent() const;

  // Gets action that was performed at parent node and the result led to this node.
  const shared_ptr<Action>& getIncomingAction() const;

  // Gets id of action that was performed at parent node and the result led to this node.
  int getIncomingActionId() const;

  // Returns the game state of that is represented by EFG node.
  // Note that in simultaneous games one state corresponds to multiple efg nodes.
  shared_ptr<State> getState() const;

  // Check if NO actions were played in this round
  bool noActionPerformedInThisRound() const;

  // Return if this node is terminal (leaf)
  bool isTerminal() const;

  string toString() const;

  size_t getHash() const;

  int getDepth() const;

  bool operator==(const EFGNode &rhs) const;

  int getDistanceFromRoot() const;

  int getLastObservationIdOfCurrentPlayer() const;

  int getNumberOfRemainingPlayers() const;

  int getLastObservationOfPlayer(int player) const;

  optional<int> getCurrentPlayer() const;

  vector<double> rewards;

  double natureProbability;

 private:
  vector<pair<int, int>> getAOH(int player) const;
  bool compareAOH(const EFGNode &rhs) const;
  size_t getHashedAOHs() const;
  vector<shared_ptr<Observation>> observations;
  vector<pair<int, shared_ptr<Action>>> performedActionsInThisRound;
  vector<int> remainingPlayersInTheRound;
  shared_ptr<State> state;
  shared_ptr<EFGNode const> parent;
  shared_ptr<Action> incomingAction;  // Action performed in the parent node.
  optional<int> currentPlayer = nullopt;
  mutable size_t hashAOH = 0;
  int depth;
};
}  // namespace GTLib2

namespace std {
template<>
struct hash<shared_ptr<EFGNode>> {
  size_t operator()(const shared_ptr<EFGNode> &p) const {
    return p->getHash();
  }
};

template<>
struct equal_to<shared_ptr<EFGNode>> {
  bool operator()(const shared_ptr<EFGNode> &a,
                  const shared_ptr<EFGNode> &b) const {
    return *a == *b;
  }
};

template<>
struct hash<EFGNode *> {
  size_t operator()(const EFGNode *p) const {
    return p->getHash();
  }
};

template<>
struct equal_to<EFGNode *> {
  bool operator()(const EFGNode *a,
                  const EFGNode *b) const {
    return *a == *b;
  }
};
}  // namespace std

#endif  // BASE_EFG_H_

#pragma clang diagnostic pop
