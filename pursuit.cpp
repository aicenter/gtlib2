//
// Created by rozliv on 02.08.2017.
//

#include "pursuit.h"

PursuitAction::PursuitAction(int id, int move): Action(id), move_(move) {}

string PursuitAction::ToString() {
  return to_string(id_);
}


PursuitObservation::PursuitObservation(int id, int value):
    Observation(id), value_(value) {}

string PursuitObservation::ToString() {
  return to_string(id_);
}


PursuitState::PursuitState(const vector<Pos> &p):place_(p) {
  probdis_ = {0.1, 0.9};
}

PursuitState::PursuitState(const vector<Pos> &p, double prob):
    place_(p), prob_(prob) {
  probdis_ = {0.1, 0.9};
}

vector<shared_ptr<Action>> PursuitState::getActions(int player) {
  vector<shared_ptr<Action>> list = vector<shared_ptr<Action>>();
  getActions(list, player);
  return list;
}

void PursuitState::getActions(vector<shared_ptr<Action>> &list, int player) const {
  int count = 0;
  for (int i = 1; i < 5; ++i) {
    if ((place_[player].x + m_[i].x) >= 0 && (place_[player].x + m_[i].x)
                                            < PursuitDomain::width_ &&
        (place_[player].y + m_[i].y) >= 0 && (place_[player].y + m_[i].y)
                                            < PursuitDomain::height_) {
      list.push_back(std::make_shared<PursuitAction>(count, i));
      count++;
    }
  }
}

ProbDistribution PursuitState::PerformAction(const vector<shared_ptr<Action>>& actions2) {
  // TODO(rozlijak): one-line - casting to PursuitAction
  vector<shared_ptr<PursuitAction>> actions = Cast<Action, PursuitAction>(actions2);
  vector<Pos> moves = vector<Pos>();
  moves.reserve(actions.size());
  auto count = static_cast<unsigned int>(pow(2, actions.size()));
  vector<std::pair<Outcome, double>> pairs;
  for (int k = 0; k < count; ++k) {
    vector<double> rew = vector<double>(actions.size());
    double probability = prob_;
    for (int i = 0; i < actions.size(); ++i) {
      if (((k >> i) & 1) == 1) {
        moves.push_back({m_[actions[i]->getMove()].y + place_[i].y,
                         m_[actions[i]->getMove()].x + place_[i].x});
        probability *= probdis_[1];
      } else {
        moves.push_back({place_[i].y, place_[i].x});
        probability *= probdis_[0];
      }
    }
    unique_ptr<PursuitState>s = make_unique<PursuitState>(moves, probability);
    moves.clear();
    for (int i = 1; i < s->getPlace().size(); ++i) {
      if ((s->place_[0].x == place_[i].x && s->place_[0].y == place_[i].y &&
           s->place_[i].x == place_[0].x &&
           s->place_[i].y == place_[0].y) ||
          (s->place_[0].x == s->place_[i].x && s->place_[0].y == s->place_[i].y)) {
        ++rew[0];
        --rew[i];
      }
    }

    int index;
    vector<unique_ptr<Observation>> obs = vector<unique_ptr<Observation>>();
    obs.push_back(make_unique<PursuitObservation>(0, 2));
    for (int i = 1; i < s->place_.size(); ++i) {
      index = 0;
      for (int l = 1; l < eight.size(); ++l) {
        if (s->place_[0].x + eight[l].x == s->place_[i].x &&
            s->place_[0].y + eight[l].y == s->place_[i].y) {
          index = l;
        }
      }
//       obs[0] = PursuitObservation(0,index);
      // TODO(rozlijak): zeptat se jak je to s tim vektorem observaci
      obs.push_back(make_unique<PursuitObservation>(k, eight.size()-1-index));
    }
    double p2 = s->prob_/prob_;
    Outcome p(move(s), move(obs), rew);
    pairs.emplace_back(move(p), p2);
  }
  ProbDistribution prob(move(pairs));
  return prob;
}

PursuitState::PursuitState() = default;

PursuitDomain::PursuitDomain(int max, const unique_ptr<State> &r, int maxPlayers):
    Domain(r, maxPlayers), maxdepth_(max) {}

int PursuitDomain::height_ = 0;
int PursuitDomain::width_ = 0;

string PursuitDomain::GetInfo() {
  return "Rozmery pole jsou: " + to_string(PursuitDomain::height_) + " x " +
         to_string(PursuitDomain::width_) + " maximalni hloubka grafu je: " +
         to_string(maxdepth_) + " a pocatecni stav je: ...";
}

int count = 0;
vector<double> rewards;

void Pursuit(const unique_ptr<Domain>& domain, const unique_ptr<State> &state, int depth) {
  vector<shared_ptr<Action>> actions = state->getActions(0);
  vector<shared_ptr<Action>> actions2 = state->getActions(1);
  ++count;
  if (depth == 0) {
    return;
  }
  for (auto &action : actions) {
    for (auto &j : actions2) {
      // TODO(rozlijak): change Actions to unique pointers
      ProbDistribution prob = state->PerformAction({action, j});
      for (Outcome &o : prob.GetOutcomes()) {
        for (int i = 0; i < rewards.size(); ++i) {
           rewards[i] += o.getReward()[i];
        }
        Pursuit(domain, o.getState(), depth - 1);
      }
    }
  }
}
