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


#include "domains/phantomTTT.h"
#include <cassert>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifdef DString
#  define D(x) x
#else
#  define DebugString(x) x
#endif  // MyDEBUG

namespace GTLib2 {
namespace domains {

PhantomTTTAction::PhantomTTTAction(ActionId id, int move) : Action(id), move_(move) {}

bool PhantomTTTAction::operator==(const Action &that) const {
  if (typeid(*this) == typeid(that)) {
    const auto rhsAction = static_cast<const PhantomTTTAction *>(&that);
    return move_ == rhsAction->move_;
  }
  return false;
}

size_t PhantomTTTAction::getHash() const {
  std::hash<int> h;
  return h(move_);
}

PhantomTTTObservation::PhantomTTTObservation(int id) : Observation(id) {}

PhantomTTTState::PhantomTTTState(Domain *domain, vector<vector<int>> p,
                                 vector<Player> players) : State(domain),
                                                        place_(move(p)), players_(move(players)) {
  strings_ = vector<string>(2);
}

// todo: this is only hotfix
unsigned long PhantomTTTState::countAvailableActionsFor(Player player) const {
  return getAvailableActionsFor(player).size();
}

vector<shared_ptr<Action>> PhantomTTTState::getAvailableActionsFor(Player player) const {
  auto list = vector<shared_ptr<Action>>();
  int count = 0;
  for (int i = 0; i < 9; ++i) {
    if (place_[player][i] == 0) {
      list.push_back(make_shared<PhantomTTTAction>(count, i));
      ++count;
    }
  }
  return list;
}

OutcomeDistribution PhantomTTTState::performActions
    (const vector<PlayerAction> &actions) const {
  auto a1 = dynamic_cast<PhantomTTTAction*>(actions[0].second.get());
  auto a2 = dynamic_cast<PhantomTTTAction*>(actions[1].second.get());
  vector<shared_ptr<Observation>> observations(2);
  vector<double> rewards(2);
  int success = 0;
  vector<Player> pla2;
  // observations.reserve(2);
  vector<vector<int>> moves = place_;
  if (a1) {
    if (moves[1][a1->GetMove()] == 0) {
      moves[0][a1->GetMove()] = 1;
      success = 1;
      pla2.emplace_back(1);
    } else {
      moves[0][a1->GetMove()] = 2;
      pla2.emplace_back(0);
    }
    observations[0] = make_shared<PhantomTTTObservation>(success);
    observations[1] = make_shared<Observation>(NO_OBSERVATION);
  } else {
    if (moves[0][a2->GetMove()] == 0) {
      moves[1][a2->GetMove()] = 1;
      success = 1;
      pla2.emplace_back(0);
    } else {
      moves[1][a2->GetMove()] = 2;
      pla2.emplace_back(1);
    }
    observations[0] = make_shared<Observation>(NO_OBSERVATION);
    observations[1] = make_shared<PhantomTTTObservation>(success);
  }
  auto board = moves[0];
  for (int i = 0; i < 9; ++i) {
    if (moves[1][i] == 1) {
      board[i] = 2;
    }
  }
  shared_ptr<PhantomTTTState> s;

  if ((board[0] == board[1] && board[1] == board[2])
      || (board[0] == board[3] && board[3] == board[6])
      || (board[0] == board[4] && board[4] == board[8])) {
    if (board[0] == 1) {
      rewards[0] = +1;
      rewards[1] = -1;
    } else if (board[0] == 2) {
      rewards[0] = -1;
      rewards[1] = +1;
    }
  }
  if ((board[8] == board[5] && board[5] == board[2])
      || (board[8] == board[7] && board[7] == board[6])) {
    if (board[8] == 1) {
      rewards[0] = +1;
      rewards[1] = -1;
    } else if (board[8] == 2) {
      rewards[0] = -1;
      rewards[1] = +1;
    }
  }
  if ((board[4] == board[1] && board[4] == board[7])
      || (board[4] == board[3] && board[4] == board[5])) {
    if (board[4] == 1) {
      rewards[0] = +1;
      rewards[1] = -1;
    } else if (board[4] == 2) {
      rewards[0] = -1;
      rewards[1] = +1;
    }
  }
  if ((board[2] == board[4] && board[4] == board[6])) {
    if (board[2] == 1) {
      rewards[0] = +1;
      rewards[1] = -1;
    } else if (board[2] == 2) {
      rewards[0] = -1;
      rewards[1] = +1;
    }
  }
  if (rewards[0] != 0) {
    for (int j = 0; j < 9; ++j) {
      if (moves[0][j] == 0) {
        moves[0][j] = -1;
      }
      if (moves[1][j] == 0) {
        moves[1][j] = -1;
      }
    }
    s = make_shared<PhantomTTTState>(domain_, moves, pla2);
    DebugString(s->strings_[0].append(strings_[0] + "  ||  ACTION: " +
                (a1 ? a1->toString() : "NoA") + "  | OBS: " + observations[0]->toString()
                + "  ||  END OF GAME");
                s->strings_[1].append(strings_[1] + "  ||  ACTION: " +
                (a2 ? a2->toString() : "Nothing") + "  | OBS: " + observations[1]->toString()
                + "  ||  END OF GAME");)
  } else {
    s = make_shared<PhantomTTTState>(domain_, moves, pla2);
    DebugString(s->strings_[0].append(strings_[0] + "  ||  ACTION: " +
                (a1 ? a1->toString() : "NoA") + "  | OBS: " + observations[0]->toString());
                s->strings_[1].append(strings_[1] + "  ||  ACTION: " +
                (a2 ? a2->toString() : "NoA") + "  | OBS: " + observations[1]->toString());)
  }
  Outcome o(move(s), observations, shared_ptr<Observation>(), rewards);
  OutcomeDistribution prob;
  prob.push_back(pair<Outcome, double>(move(o), 1.0));
  return prob;
}
size_t PhantomTTTState::getHash() const {
  size_t seed = 0;
  for (auto &i : place_) {
    boost::hash_combine(seed, i);
  }
  for (auto &i : players_) {
    boost::hash_combine(seed, i);
  }
  return seed;
}

bool PhantomTTTState::operator==(const State &rhs) const {
  auto State = dynamic_cast<const PhantomTTTState &>(rhs);

  return place_ == State.place_ &&
      strings_ == State.strings_ &&
      players_ == State.players_;
}

PhantomTTTDomain::PhantomTTTDomain(unsigned int max) :
    Domain(max, 2) {
  auto vec = vector<vector<int>>{{0, 0, 0, 0, 0, 0, 0, 0, 0},
                                 {0, 0, 0, 0, 0, 0, 0, 0, 0}};
  auto players = vector<Player>({0});
  vector<double> rewards(2);
  vector<shared_ptr<Observation>> Obs{make_shared<Observation>(NO_OBSERVATION), make_shared<Observation>(NO_OBSERVATION)};
  Outcome o(make_shared<PhantomTTTState>(this, vec, players),
            Obs,
            shared_ptr<Observation>(),
            rewards);
  rootStatesDistribution_.push_back(pair<Outcome, double>(move(o), 1.0));
}

string PhantomTTTDomain::getInfo() const {
  return "************ Phantom Tic Tac Toe *************\n" +
      rootStatesDistribution_[0].first.state_->toString() + "\n";
}
}  // namespace domains
}  // namespace GTLib2
#pragma clang diagnostic pop
