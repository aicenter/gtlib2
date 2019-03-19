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


#include "domains/matching_pennies.h"
#include <cassert>
#include <limits>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {
namespace domains {
MatchingPenniesDomain::MatchingPenniesDomain() : Domain(std::numeric_limits<int>::max(), 2) {
  auto rootState = make_shared<MatchingPenniesState>(this, Nothing, Nothing);

  auto newObservationP1 = make_shared<MatchingPenniesObservation>(OtherNothing);
  auto newObservationP2 = make_shared<MatchingPenniesObservation>(OtherNothing);

  vector<shared_ptr<Observation>> observations{newObservationP1, newObservationP2};
  vector<double> rewards(2);
  Outcome outcome(rootState, observations, rewards);

  rootStatesDistribution.push_back(pair<Outcome, double>(outcome, 1.0));
}

vector<Player> MatchingPenniesDomain::getPlayers() const {
  return {0, 1};
}

vector<shared_ptr<Action>> MatchingPenniesState::getAvailableActionsFor(Player player) const {
  vector<shared_ptr<Action>> actions = vector<shared_ptr<Action>>();
  if (player == 0 && player1 == Nothing && player2 == Nothing) {
    actions.push_back(make_shared<MatchingPenniesAction>(Heads));
    actions.push_back(make_shared<MatchingPenniesAction>(Tails));
  } else if (player == 1 && player1 != Nothing && player2 == Nothing) {
    actions.push_back(make_shared<MatchingPenniesAction>(Heads));
    actions.push_back(make_shared<MatchingPenniesAction>(Tails));
  }
  return actions;
}

MatchingPenniesState::MatchingPenniesState(Domain *domain, Move p1, Move p2) : State(domain) {
  player1 = p1;
  player2 = p2;
  if (player1 == Nothing && player2 == Nothing) {
    players.push_back(0);
  }
  if (player2 == Nothing && player1 != Nothing) {
    players.push_back(1);
  }
}

int MatchingPenniesState::getNumberOfPlayers() const {
  return static_cast<int> (players.size());
}

OutcomeDistribution
MatchingPenniesState::performActions(const vector<PlayerAction> &actions) const {
  auto p1Action = dynamic_cast<MatchingPenniesAction*>(actions[0].second.get());
  auto p2Action = dynamic_cast<MatchingPenniesAction*>(actions[1].second.get());

  assert(p1Action == nullptr || p2Action == nullptr);  // Only one action can be performed
  assert(player1 == Nothing || p2Action != nullptr);  //  player1 played -> player2 has to play.


  auto newState = make_shared<MatchingPenniesState>(domain,
                                                    p1Action == nullptr ? player1 : p1Action->move,
                                                    p2Action == nullptr ? Nothing : p2Action->move);

  const bool finalState = newState->player1 != Nothing && newState->player2 != Nothing;

  OtherMove
      p1obs = finalState ? (newState->player2 == Heads ? OtherHeads : OtherTails) : OtherNothing;
  OtherMove
      p2obs = finalState ? (newState->player1 == Heads ? OtherHeads : OtherTails) : OtherNothing;

  shared_ptr<MatchingPenniesObservation>
      newObservationP1 = make_shared<MatchingPenniesObservation>(p1obs);
  shared_ptr<MatchingPenniesObservation>
      newObservationP2 = make_shared<MatchingPenniesObservation>(p2obs);

  vector<shared_ptr<Observation>> observations{newObservationP1, newObservationP2};

  vector<double> rewards(2);
  if (newState->player1 == Nothing || newState->player2 == Nothing) {
    rewards[0] = 0;
    rewards[1] = 0;
  } else {
    if (newState->player1 == newState->player2) {
      rewards[0] = +1;
      rewards[1] = -1;
    } else {
      rewards[0] = -1;
      rewards[1] = +1;
    }
  }

  Outcome outcome(newState, observations, rewards);

  OutcomeDistribution distr;
  distr.push_back(pair<Outcome, double>(outcome, 1.0));
  return distr;
}

string MatchingPenniesState::toString() const {
  string desc = "Player 1: ";
  desc += player1 == Nothing ? "Nothing" : player1 == Heads ? "Heads" : "Tails";
  desc += " Player 2: ";
  desc += player2 == Nothing ? "Nothing" : player2 == Heads ? "Heads" : "Tails";

  return desc;
}

vector<Player> MatchingPenniesState::getPlayers() const {
  return players;
}

size_t MatchingPenniesState::getHash() const {
  size_t seed = 0;
  boost::hash_combine(seed, player1);
  boost::hash_combine(seed, player2);
  return seed;
}

bool MatchingPenniesState::operator==(const State &rhs) const {
  auto mpState = dynamic_cast<const MatchingPenniesState &>(rhs);
  return player1 == mpState.player1 && player2 == mpState.player2 && players == mpState.players;
}

MatchingPenniesObservation::MatchingPenniesObservation(OtherMove otherMoveParm) :
    Observation(static_cast<int>(otherMoveParm)) {
  otherMove = otherMoveParm;
}

MatchingPenniesAction::MatchingPenniesAction(Move moveParm) :
    Action(static_cast<int>(moveParm)) {
  move = moveParm;
}

string MatchingPenniesAction::toString() const {
  return move == Heads ? "Heads" : move == Tails ? "Tails" : "Nothing";
}

bool MatchingPenniesAction::operator==(const Action &that) const {
  if (typeid(*this) == typeid(that)) {
    const auto rhsAction = static_cast<const MatchingPenniesAction *>(&that);
    return this->move == rhsAction->move;
  }
  return false;
}

size_t MatchingPenniesAction::getHash() const {
  std::hash<int> h;
  return h(move);
}

SimultaneousMatchingPenniesState::SimultaneousMatchingPenniesState(Domain *domain, Move p1, Move p2)
    : State(domain) {
  assert((p1 != Nothing && p2 != Nothing) || (p1 == Nothing && p2 == Nothing));
  player1 = p1;
  player2 = p2;
  if (player1 == Nothing) {
    players.push_back(0);
  }
  if (player2 == Nothing) {
    players.push_back(1);
  }
}

vector<shared_ptr<Action>>
SimultaneousMatchingPenniesState::getAvailableActionsFor(Player player) const {
  vector<shared_ptr<Action>> actions = vector<shared_ptr<Action>>();
  if (player1 == Nothing && player2 == Nothing) {
    actions.push_back(make_shared<MatchingPenniesAction>(Heads));
    actions.push_back(make_shared<MatchingPenniesAction>(Tails));
  }
  return actions;
}

OutcomeDistribution
SimultaneousMatchingPenniesState::performActions(
    const vector<PlayerAction> &actions)
const {
  auto p1Action = dynamic_cast<MatchingPenniesAction*>(actions[0].second.get());
  auto p2Action = dynamic_cast<MatchingPenniesAction*>(actions[1].second.get());

  assert(p1Action != nullptr || p2Action != nullptr);  // Both action must be performed
  // assert(player1 == Nothing || p2Action != nullptr ); //  player1 played -> player2 has to play.


  auto newState = make_shared<MatchingPenniesState>(domain,
                                                    p1Action == nullptr ? player1 : p1Action->move,
                                                    p2Action == nullptr ? Nothing : p2Action->move);

  const bool finalState = newState->player1 != Nothing && newState->player2 != Nothing;

  OtherMove
      p1obs = finalState ? (newState->player2 == Heads ? OtherHeads : OtherTails) : OtherNothing;
  OtherMove
      p2obs = finalState ? (newState->player1 == Heads ? OtherHeads : OtherTails) : OtherNothing;

  auto newObservationP1 = make_shared<MatchingPenniesObservation>(p1obs);
  auto newObservationP2 = make_shared<MatchingPenniesObservation>(p2obs);

  vector<shared_ptr<Observation>> observations{newObservationP1, newObservationP2};

  vector<double> rewards(2);
  if (newState->player1 == Nothing || newState->player2 == Nothing) {
    rewards[0] = 0;
    rewards[1] = 0;
  } else {
    if (newState->player1 == newState->player2) {
      rewards[0] = +1;
      rewards[1] = -1;
    } else {
      rewards[0] = -1;
      rewards[1] = +1;
    }
  }

  Outcome outcome(newState, observations, rewards);

  OutcomeDistribution distr;
  distr.push_back(pair<Outcome, double>(outcome, 1.0));

  return distr;
}

int SimultaneousMatchingPenniesState::getNumberOfPlayers() const {
  return static_cast<int> (players.size());
}

string SimultaneousMatchingPenniesState::toString() const {
  std::string desc = "Player 1: ";
  desc += player1 == Nothing ? "Nothing" : player1 == Heads ? "Heads" : "Tails";
  desc += " Player 2: ";
  desc += player2 == Nothing ? "Nothing" : player2 == Heads ? "Heads" : "Tails";

  return desc;
}

vector<Player> SimultaneousMatchingPenniesState::getPlayers() const {
  return players;
}

size_t SimultaneousMatchingPenniesState::getHash() const {
  size_t seed = 0;
  boost::hash_combine(seed, player1);
  boost::hash_combine(seed, player2);
  return seed;
}

SimultaneousMatchingPenniesDomain::SimultaneousMatchingPenniesDomain() :
    Domain(std::numeric_limits<int>::max(), 2) {
  auto rootState = make_shared<SimultaneousMatchingPenniesState>(this, Nothing, Nothing);

  auto newObservationP1 = make_shared<MatchingPenniesObservation>(OtherNothing);
  auto newObservationP2 = make_shared<MatchingPenniesObservation>(OtherNothing);

  vector<shared_ptr<Observation>> observations{newObservationP1, newObservationP2};
  vector<double> rewards(2);
  Outcome outcome(rootState, observations, rewards);

  rootStatesDistribution.push_back(pair<Outcome, double>(outcome, 1.0));
}

vector<Player> SimultaneousMatchingPenniesDomain::getPlayers() const {
  return {0, 1};
}
}  // namespace domains
}  // namespace GTLib2
#pragma clang diagnostic pop
