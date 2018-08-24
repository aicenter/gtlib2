//
// Created by Jakub Rozlivek on 14.08.2017.
//

#include "efg.h"

#include <utility>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <vector>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {

EFGNodesDistribution EFGNode::performAction(const shared_ptr<Action> &action) const {
  vector<pair<int, shared_ptr<Action>>> actionsToBePerformed(performedActionsInThisRound);
  actionsToBePerformed.emplace_back(*currentPlayer, action);

  EFGNodesDistribution newNodes;
  if (remainingPlayersInTheRound.size() == 1) {
    for (auto &i : state->getDomain()->getPlayers()) {
      auto action2 = std::find_if(actionsToBePerformed.begin(), actionsToBePerformed.end(),
                                  [&i](pair<int, shared_ptr<Action>> const &elem) {
                                    return elem.first == i;
                                  });
      if (action2 == actionsToBePerformed.end()) {
        actionsToBePerformed.emplace_back(i, make_shared<Action>(-1));
      }
    }
    // Last player in the round. So we proceed to the next state
    auto probDist = state->performActions(actionsToBePerformed);
    for (auto const&[outcome, prob] : probDist) {  // works in GCC 7.3
//            for (auto const& outcomeProb : probDist) {
//                auto outcome = outcomeProb.first;
//                auto prob = outcomeProb.second;
      auto newNode = make_shared<EFGNode>(outcome.state, shared_from_this(),
                                          outcome.observations, outcome.rewards,
                                          actionsToBePerformed, prob * natureProbability, action);
      newNodes.emplace_back(newNode, prob);
    }
  } else {
    auto newNode = make_shared<EFGNode>(shared_from_this(), actionsToBePerformed, action);
    newNodes.emplace_back(newNode, 1.0);
  }
  return newNodes;
}

EFGNode::EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                 const vector<shared_ptr<Observation>> &observations,
                 const vector<double> &rewards,
                 const vector<pair<int, shared_ptr<Action>>> &lastRoundActions,
                 double natureProbability, shared_ptr<Action> incomingAction) {
  this->state = move(newState);
  this->observations = observations;
  this->previousRoundActions = lastRoundActions;
  this->rewards = rewards;
  this->natureProbability = natureProbability;

  remainingPlayersInTheRound = state->getPlayers();
  std::reverse(remainingPlayersInTheRound.begin(), remainingPlayersInTheRound.end());
  if (!remainingPlayersInTheRound.empty()) {
    currentPlayer = remainingPlayersInTheRound.back();
  } else {
    currentPlayer = nullopt;
  }

  this->parent = move(parent);
  this->incomingAction = move(incomingAction);
}

EFGNode::EFGNode(shared_ptr<EFGNode const> parent,
                 const vector<pair<int, shared_ptr<Action>>> &performedActions,
                 shared_ptr<Action> incomingAction) {
  this->state = parent->getState();
  this->observations = parent->observations;
  this->rewards = parent->rewards;
  this->natureProbability = parent->natureProbability;
  this->previousRoundActions = parent->previousRoundActions;
  this->incomingAction = move(incomingAction);

  this->performedActionsInThisRound = performedActions;

  remainingPlayersInTheRound = parent->remainingPlayersInTheRound;
  remainingPlayersInTheRound.pop_back();
  if (!remainingPlayersInTheRound.empty()) {
    currentPlayer = remainingPlayersInTheRound.back();
  } else {
    currentPlayer = nullopt;
  }
  this->parent = move(parent);
}

vector<shared_ptr<Action>> EFGNode::availableActions() const {
  if (currentPlayer) {
    return move(state->getAvailableActionsFor(*currentPlayer));
  }
  return vector<shared_ptr<Action>>();
}

shared_ptr<AOH> EFGNode::getAOHInfSet() const {
  if (currentPlayer) {
    auto aoh = getAOH(*currentPlayer);
    if (remainingPlayersInTheRound.size() > 1 || !parent) {
      auto action = std::find_if(previousRoundActions.begin(), previousRoundActions.end(),
                                 [this](pair<int, shared_ptr<Action>> const &elem) {
                                   return elem.first == *currentPlayer;
                                 });
      if (action != previousRoundActions.end()) {
        aoh.emplace_back(action->second->getId(), observations[*currentPlayer]->getId());
      }
    }
    return make_shared<AOH>(*currentPlayer, aoh);
  } else {
    return nullptr;
  }
}

vector<std::pair<int, int>> EFGNode::getAOH(int player) const {
  auto aoh = this->parent ? this->parent->getAOH(player) : vector<std::pair<int, int>>{
      std::make_pair(-1, this->observations[player]->getId())};
  if (remainingPlayersInTheRound.size() == 1) {
    auto action = std::find_if(previousRoundActions.begin(), previousRoundActions.end(),
                               [&player](pair<int, shared_ptr<Action>> const &elem) {
                                 return elem.first == player;
                               });
    if (action != previousRoundActions.end()) {
      aoh.emplace_back(action->second->getId(), observations[player]->getId());
    }  // TODO: try to rewrite it with incomingAction
  }
  return aoh;
}

optional<int> EFGNode::getCurrentPlayer() const {
  return currentPlayer;
}

ActionSequence EFGNode::getActionsSeqOfPlayer(int player) const {
  auto actSeq = this->parent ? this->parent->getActionsSeqOfPlayer(player) : ActionSequence();
  if (parent && parent->currentPlayer && *parent->currentPlayer == player) {
    actSeq.emplace_back(parent->getAOHInfSet(), incomingAction);
  }
  return actSeq;
}

shared_ptr<EFGNode const> EFGNode::getParent() const {
  return parent;
}

shared_ptr<State> EFGNode::getState() const {
  return state;
}

bool EFGNode::isContainedInInformationSet(const shared_ptr<AOH> &infSet) const {
  auto mySet = this->getAOHInfSet();
  if (mySet == nullptr) {
    return false;
  }
  return *mySet == *infSet;
}

shared_ptr<Action> EFGNode::getIncomingAction() const {
  return incomingAction;
}

double EFGNode::getProbabilityOfActionsSeqOfPlayer(int player,
                                                   const BehavioralStrategy &strat) const {
  if (!parent) {
    return 1.0;
  }

  auto prob = parent->getProbabilityOfActionsSeqOfPlayer(player, strat);

  if (*parent->getCurrentPlayer() == player) {
    auto parentInfSet = parent->getAOHInfSet();
    auto actionsProbs = strat.at(parentInfSet);
    double actionProb = (actionsProbs.find(incomingAction) != actionsProbs.end()) ?
                        actionsProbs.at(incomingAction) : 0.0;
    return actionProb * prob;
  } else {
    return prob;
  }
}

size_t EFGNode::getHash() const {
  auto seed = state->getHash();
  boost::hash_combine(seed, performedActionsInThisRound.size());
  boost::hash_combine(seed, remainingPlayersInTheRound.size());
  return seed;
}

bool EFGNode::operator==(const EFGNode &rhs) const {
  if (this->performedActionsInThisRound.size() != rhs.performedActionsInThisRound.size()) {
    return false;
  }
  for (auto const&[player, action] : this->performedActionsInThisRound) {  // works in GCC 7.3
//          for (auto const& it : this->performedActionsInThisRound) {
//            auto player = it.first;
//            auto action = it.second;
    auto action2 =
        std::find_if(rhs.performedActionsInThisRound.begin(), rhs.performedActionsInThisRound.end(),
                     [&player](pair<int, shared_ptr<Action>> const &elem) {
                       return elem.first == player;
                     });
    if (action2 == rhs.performedActionsInThisRound.end()
        || !(*action2->second == *action)) {
      return false;
    }
  }
  // TODO: maybe AOHs are enouqh or change to something faster
  return this->remainingPlayersInTheRound == rhs.remainingPlayersInTheRound &&
      *(this->state) == *(rhs.state) && getAOH(0) == rhs.getAOH(0) && getAOH(1) == rhs.getAOH(1);
}

int EFGNode::getDistanceFromRoot() const {
  if (!parent)
    return 0;
  return 1 + parent->getDistanceFromRoot();
}

int EFGNode::getLastObservationIdOfCurrentPlayer() const {
  return observations[*currentPlayer]->getId();
}

string EFGNode::toString() const {
  string s = "Player: " + to_string(*currentPlayer) + ", incoming action: ";
  s += incomingAction ? incomingAction->toString() : "none (root)";
  s += ", nature probability: " + to_string(natureProbability) + "\n" +
      state->toString() + "\nRewards: [";
  std::stringstream rews;
  std::copy(rewards.begin(), rewards.end(), std::ostream_iterator<int>(rews, ", "));
  std::stringstream rem;
  std::copy(remainingPlayersInTheRound.begin(), remainingPlayersInTheRound.end(),
            std::ostream_iterator<int>(rem, ", "));
  s += rews.str().substr(0, rews.str().length() - 2) + "]\nObs: [";
  for (auto &i : observations) {
    s += i->toString() + " ";
  }
  s += "]\nRemaining players: [" + rem.str().substr(0, rem.str().length() - 2)
      + "]\nPerformed actions in this round:\n";
  for (auto &i : performedActionsInThisRound) {
    s += to_string(i.first) + "  " + i.second->toString() + "\n";
  }
  s += "\n";
  return s;
}

int EFGNode::getNumberOfRemainingPlayers() const {
  return static_cast<int>(remainingPlayersInTheRound.size());
}

int EFGNode::getLastObservationOfPlayer(int player) const {
  return observations[player]->getId();
}
}  // namespace GTLib2

#pragma clang diagnostic pop
