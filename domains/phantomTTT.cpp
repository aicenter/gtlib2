//
// Created by Jacob on 02.11.2017.
//

#include "phantomTTT.h"
#include "assert.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifdef DString
#  define D(x) x
#else
#  define D(x) x
#endif  // MyDEBUG

PhantomTTTAction::PhantomTTTAction(int id, int move): Action(id), move_(move) {}

PhantomTTTObservation::PhantomTTTObservation(int id, int value):
    Observation(id), value_(value) {}


PhantomTTTState::PhantomTTTState(const vector<vector<int>> &p,
                                 const vector<bool>& players):
  place_(p), players_(players) {
  strings_ = vector<string>(2);
}

vector<shared_ptr<Action>> PhantomTTTState::getAvailableActionsFor(int player) const {
  auto list = vector<shared_ptr<Action>>();
  GetActions(list, player);
  return list;
}

void PhantomTTTState::GetActions(vector<shared_ptr<Action>> &list,
                                 int player) const {
  int count = 0;
  for (int i = 0; i < 9; ++i) {
    if (place_[player][i] == 0) {
      list.push_back(make_shared<PhantomTTTAction>(count, i));
      ++count;
    }
  }
}

ProbDistribution
PhantomTTTState::PerformAction(const vector<shared_ptr<Action>> &actions2) {
  vector<shared_ptr<PhantomTTTAction>> actions =
      Cast<Action, PhantomTTTAction>(actions2);
  auto rew = vector<double>(2);
  auto obser = vector<shared_ptr<Observation>>();
  auto ob = vector<int>(2);
  vector<bool> pla2(2, true);
  obser.reserve(2);
  vector<vector<int>> moves = place_;
  if (actions[0]->getId() > -1) {
    if (moves[1][actions[0]->GetMove()] == 0) {
      moves[0][actions[0]->GetMove()] = 1;
      ob[0] = 1;
      pla2[0] = false;
    } else {
      moves[0][actions[0]->GetMove()] = 2;
      pla2[1] = false;
    }
    obser.push_back(MakeUnique<PhantomTTTObservation>(0, ob[0]));
    obser.push_back(MakeUnique<Observation>(NoOb));
  } else {
    if (moves[0][actions[1]->GetMove()] == 0) {
      moves[1][actions[1]->GetMove()] = 1;
      ob[1] = 1;
      pla2[1] = false;
    } else {
      moves[1][actions[1]->GetMove()] = 2;
      pla2[0] = false;
    }
    obser.push_back(MakeUnique<Observation>(NoOb));
    obser.push_back(MakeUnique<PhantomTTTObservation>(0, ob[1]));
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
      rew ={1.0, -1.0};
    } else if (board[0] == 2) {
      rew = {-1.0, 1.0};
    }
  }
  if ((board[8] == board[5] && board[5] == board[2])
      || (board[8] == board[7] && board[7] == board[6])) {
    if (board[8] == 1) {
      rew ={1.0, -1.0};
    } else if (board[8] == 2) {
      rew = {-1.0, 1.0};
    }
  }
  if ((board[4] == board[1] && board[4] == board[7])
      || (board[4] == board[3] && board[4] == board[5])) {
    if (board[4] == 1) {
      rew ={1.0, -1.0};
    } else if (board[4] == 2) {
      rew = {-1.0, 1.0};
    }
  }
  if ((board[2] == board[4] && board[4] == board[6])) {
    if (board[2] == 1) {
      rew ={1.0, -1.0};
    } else if (board[2] == 2) {
      rew = {-1.0, 1.0};
    }
  }
  if (rew[0] != 0) {
    for (int j = 0; j < 9; ++j) {
      if (moves[0][j] == 0) {
        moves[0][j] = -1;
      }
      if (moves[1][j] == 0) {
        moves[1][j] = -1;
      }
    }
    s = make_shared<PhantomTTTState>(moves, pla2);
    D(for (unsigned int j = 0; j < 2; ++j) {
      s->AddString(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
                   "  | OBS: " + obser[j]->toString()+ "  ||  END OF GAME", j);
    })
  } else {
    s = make_shared<PhantomTTTState>(moves, pla2);
    D(for (unsigned int j = 0; j < 2; ++j) {
      s->AddString(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
                   "  | OBS: " + obser[j]->toString(), j);
    })
  }

  Outcome p(move(s), move(obser), rew);
  vector<pair<Outcome, double>> pair{{move(p), 1}};
  ProbDistribution prob(move(pair));  // pair of an outcome and its probability
  return prob;
}

ProbDistribution PhantomTTTState::performActions(const unordered_map<int, shared_ptr<Action>> &actions) const {
  assert(false);
  return ProbDistribution(vector<pair<Outcome, double>>());
}


PhantomTTTDomain::PhantomTTTDomain(unsigned int max) :
    Domain(max, 2) {
  vector<pair<Outcome, double>> pairs;
  auto vec = vector<vector<int>>{{0, 0, 0, 0, 0, 0, 0, 0, 0},
                                 {0, 0, 0, 0, 0, 0, 0, 0, 0}};
  auto players = vector<bool>({true, false});
  Outcome o(make_shared<PhantomTTTState>(vec, players),
            move(vector<shared_ptr<Observation>>(numberOfPlayers)),
            vector<double>(numberOfPlayers));
  pairs.emplace_back(move(o), 1);
  rootStatesDistributionPtr = make_shared<ProbDistribution>(move(pairs));
}

string PhantomTTTDomain::GetInfo() {
  return "************ Phantom Tic Tac Toe *************\n" +
          rootStatesDistributionPtr->GetOutcomes()[0].GetState()->toString(0) + "\n" +
          rootStatesDistributionPtr->GetOutcomes()[0].GetState()->toString(1) + "\n";
}

#pragma clang diagnostic pop