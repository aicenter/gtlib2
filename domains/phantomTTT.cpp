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

PhantomTTTObservation::PhantomTTTObservation(int id) : Observation(id) {}

PhantomTTTState::PhantomTTTState(const vector<vector<int>> &p,
                                 const vector<int>& players):
  place_(p), players_(players) {
  strings_ = vector<string>(2);
}

vector<shared_ptr<Action>> PhantomTTTState::getAvailableActionsFor(int player) const {
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
    (const unordered_map<int, shared_ptr<Action>> &actions) const { // TODO: prepsat
  auto a1 = std::dynamic_pointer_cast<PhantomTTTAction>(actions.find(0)->second);
  auto a2 = std::dynamic_pointer_cast<PhantomTTTAction>(actions.find(1)->second);
  unordered_map<int,shared_ptr<Observation>> observations = unordered_map<int,shared_ptr<Observation>>();
  unordered_map<int,double> rewards = unordered_map<int,double>();
  int success = 0;
  vector<int> pla2;
  observations.reserve(2);
  rewards.reserve(2);
  vector<vector<int>> moves = place_;
  if (a1->getId() > -1) {
    if (moves[1][a1->GetMove()] == 0) {
      moves[0][a1->GetMove()] = 1;
      success = 1;
      pla2.emplace_back(1);
    } else {
      moves[0][a1->GetMove()] = 2;
      pla2.emplace_back(0);
    }
    observations[0]=make_unique<PhantomTTTObservation>(success);
    observations[1]= make_unique<Observation>(-1);
  } else {
    if (moves[0][a2->GetMove()] == 0) {
      moves[1][a2->GetMove()] = 1;
      success = 1;
      pla2.emplace_back(0);
    } else {
      moves[1][a2->GetMove()] = 2;
      pla2.emplace_back(1);
    }
    observations[0] = make_unique<Observation>(-1);
    observations[1] = make_unique<PhantomTTTObservation>(success);
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
  auto actions2 = vector<shared_ptr<PhantomTTTAction>>{a1,a2};
  if (rewards[0] != 0) {
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
      s->AddString(strings_[j] + "  ||  ACTION: " + actions2[j]->toString() +
                   "  | OBS: " + observations[j]->toString()+ "  ||  END OF GAME", j);
    })
  } else {
    s = make_shared<PhantomTTTState>(moves, pla2);
    D(for (unsigned int j = 0; j < 2; ++j) {
      s->AddString(strings_[j] + "  ||  ACTION: " + actions2[j]->toString() +
                   "  | OBS: " + observations[j]->toString(), j);
    })
  }
  Outcome o(move(s), observations, rewards);
  OutcomeDistribution prob;
  prob.push_back(pair<Outcome,double>(move(o),1.0));
  return prob;
}


PhantomTTTDomain::PhantomTTTDomain(unsigned int max) :
    Domain(max, 2) {
  auto vec = vector<vector<int>>{{0, 0, 0, 0, 0, 0, 0, 0, 0},
                                 {0, 0, 0, 0, 0, 0, 0, 0, 0}};
  auto players = vector<int>({0});
  unordered_map<int,double> rewards;
  rewards[0] = 0.0;
  rewards[1] = 0.0;
  unordered_map<int,shared_ptr<Observation>> Obs;
  Obs[0] = make_shared<Observation>(-1);
  Obs[1] = make_shared<Observation>(-1);
  Outcome o(make_shared<PhantomTTTState>(vec, players), Obs, rewards);
  rootStatesDistribution.push_back(pair<Outcome,double>(move(o),1.0));
}

string PhantomTTTDomain::getInfo() const {
  return "************ Phantom Tic Tac Toe *************\n" +
         rootStatesDistribution[0].first.state->toString(0) + "\n" +
         rootStatesDistribution[0].first.state->toString(1) + "\n";
}

#pragma clang diagnostic pop