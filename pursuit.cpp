//
// Created by rozliv on 02.08.2017.
//

#include "pursuit.h"

PursuitAction::PursuitAction(int id, int move): Action(id), move_(move) {}

string PursuitAction::ToString() {
  return to_string(id_);
}


PursuitObservation::PursuitObservation(int id, vector<int> values):
    Observation(id), values_(move(values)) {}

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

vector<shared_ptr<Action>> PursuitState::GetActions(int player) {
  vector<shared_ptr<Action>> list = vector<shared_ptr<Action>>();
  GetActions(list, player);
  return list;
}

void PursuitState::GetActions(vector<shared_ptr<Action>> &list, int player) const {
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
        moves.push_back({m_[actions[i]->GetMove()].y + place_[i].y,
                         m_[actions[i]->GetMove()].x + place_[i].x});
        probability *= probdis_[1];
      } else {
        moves.push_back({place_[i].y, place_[i].x});
        probability *= probdis_[0];
      }
    }
    unique_ptr<PursuitState>s = make_unique<PursuitState>(moves, probability);
    moves.clear();
    for (int i = 1; i < s->GetPlace().size(); ++i) {
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
    vector<int> ob = vector<int>();
    for(int m = 0; m < s->place_.size(); ++m) {
      ob.push_back(0);
      for (int i = 0; i < s->place_.size(); ++i) {
        index = 0;
        for (int l = 1; l < eight.size(); ++l) {
          if (s->place_[0].x + eight[l].x == s->place_[i].x &&
              s->place_[0].y + eight[l].y == s->place_[i].y) {
            index = l;
          }
        }
        ob[0] = index;
        ob.push_back(eight.size() - 1 - index);
      }
      obs.push_back(make_unique<PursuitObservation>(m,ob)); // TODO(rozlijak): make correct id
      ob.clear();
    }

    double p2 = s->prob_/prob_;
    Outcome p(move(s), move(obs), rew);
    pairs.emplace_back(move(p), p2);
  }
  ProbDistribution prob(move(pairs));
  return prob;
}

PursuitDomain::PursuitDomain(const vector<Pos> &loc, int maxPlayers, int max):
    Domain(maxPlayers, max) {
  root_ = make_unique<PursuitState>(loc);
}

int PursuitDomain::height_ = 0;
int PursuitDomain::width_ = 0;


string PursuitDomain::GetInfo() {
  return "Rozmery pole jsou: " + to_string(PursuitDomain::height_) + " x " +
      to_string(PursuitDomain::width_) + " maximalni hloubka grafu je: " +
      to_string(maxdepth_) + " a pocatecni stav je: " +
      to_string(root_->GetPlace()[0].x) +" "+ to_string(root_->GetPlace()[0].y) +
      "    " + to_string(root_->GetPlace()[1].x) +" "+
      to_string(root_->GetPlace()[1].y) + "\n";
}

int count = 0;
vector<double> rew;

void Pursuit(const unique_ptr<Domain>& domain, const unique_ptr<State> &state, int depth, int players) {
  vector<vector<shared_ptr<Action>>> v = vector<vector<shared_ptr<Action>>>();
  for (int i = 0; i < players; ++i) {
    v.emplace_back(state->GetActions(i));
  }
  ++count;
  if (depth == 0) {
    return;
  }
  vector<vector<shared_ptr<Action>>> action = cart_product<Action>(v);
  for (const auto &k : action) {
    ProbDistribution prob = state->PerformAction(k);
    for (Outcome &o : prob.GetOutcomes()) {
      for (int i = 0; i < rew.size(); ++i) {
        rew[i] += o.GetReward()[i];
      }
      Pursuit(domain, o.GetState(), depth - 1,players);
    }
  }
}
