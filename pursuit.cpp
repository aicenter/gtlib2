//
// Created by rozliv on 02.08.2017.
//

#include "pursuit.h"


PursuitAction::PursuitAction(int id, int move): Action(id), move_(move) {}


PursuitObservation::PursuitObservation(int id, vector<int> values):
    Observation(id), values_(move(values)) {}

string PursuitObservation::ToString() {
  string string1;
  if (values_.empty()) {
    return "NoOb;";
  }
  for (unsigned int i = 0; i < values_.size()-1; ++i) {
    string1 += eightdes_[values_[i]];
    string1 += "; ";
  }
  string1 += "Result: ";
  string1 += values_[values_.size()-1] == 1 ? "ok" : "fail";
  return string1;
}


PursuitState::PursuitState(const vector<Pos> &p):PursuitState(p, 1) {}

PursuitState::PursuitState(const vector<Pos> &p, double prob):
    place_(p), prob_(prob) {
  probdis_ = {0.1, 0.9};
  aoh_ = vector<vector<int>>(p.size());
  strings_ = vector<string>(p.size());
  numplayers_ = p.size();
  players_ = vector<bool>(p.size(), true);
}

vector<shared_ptr<Action>> PursuitState::GetActions(int player) {
  auto list = vector<shared_ptr<Action>>();
  GetActions(list, player);
  return list;
}

void PursuitState::GetActions(vector<shared_ptr<Action>> &list, int player) const {
  int count = 0;
  for (int i = 1; i < 5; ++i) {  // verifies whether moves are correct
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
  // number of all combinations
  int actionssize = actions.size();
  int powsize = actions.size();
  for (auto &i : actions) {
    if (i->GetID() == -1)
      powsize--;
  }
  auto count = pow(2, powsize);
  vector<std::pair<Outcome, double>> pairs;
  for (int k = 0; k < count; ++k) {
    auto rew = vector<double>(place_.size());
    double probability = prob_;
    // making moves and assigning probability
    vector<Pos> moves = place_;
    for (int i = 0; i < actionssize; ++i) {
      if (((k >> i) & 1) == 1) {
        if (actions[i]->GetID() > -1) {
          moves[i].x = m_[actions[i]->GetMove()].x + place_[i].x;
          moves[i].y = m_[actions[i]->GetMove()].y + place_[i].y;
        }
        probability *= probdis_[1];
      } else {
        probability *= probdis_[0];
      }
    }

    shared_ptr<PursuitState> s = std::make_shared<PursuitState>(moves, probability);
    int size = s->place_.size();
    moves.clear();
    // detection if first has caught others
    for (int i = 1; i < size; ++i) {
      if ((s->place_[0].x == place_[i].x && s->place_[0].y == place_[i].y &&
           s->place_[i].x == place_[0].x &&
           s->place_[i].y == place_[0].y) ||
          (s->place_[0].x == s->place_[i].x && s->place_[0].y == s->place_[i].y)) {
        ++rew[0];
        --rew[i];
      }
    }
    int index;
    int id;
    auto obs = vector<unique_ptr<Observation>>();
    vector<int> ob = vector<int>();
    for (int m = 0; m < size; ++m) {  // making observations
      id = 0;
      for (int i = 0, p = 0; i < size; ++i, ++p) {
        if (m == i) {
          --p;
          continue;
        }
        index = 0;
        int id2 = 1;
        for (unsigned int l = 1; l < eight_.size(); ++l) {
          if ((s->place_[m].x + eight_[l].x) >= 0 && (s->place_[m].x + eight_[l].x)
                                                     < PursuitDomain::width_ &&
              (s->place_[m].y + eight_[l].y) >= 0 && (s->place_[m].y + eight_[l].y)
                                                     < PursuitDomain::height_) {
            id2++;
          }

          if (s->place_[m].x + eight_[l].x == s->place_[i].x &&
              s->place_[m].y + eight_[l].y == s->place_[i].y) {
            index = l;
          }
        }
        ob.push_back(index);
        id += index * pow(id2, p);  // counting observation id
      }
        ob.push_back(((k >> m) & 1));
        id += ((k >> m) & 1) * ob.size();  // TODO(rozlijak): vyresit jinak
      obs.push_back(MakeUnique<PursuitObservation>(id, ob));
      ob.clear();
    }

    double p2 = s->prob_/prob_;
    Outcome p(move(s), move(obs), rew);
    pairs.emplace_back(move(p), p2);  // pair of an outcome and its probability
  }
  ProbDistribution prob(move(pairs));
  return prob;
}


MMPursuitState::MMPursuitState(const vector<Pos> &p,
                               const vector<bool>& players, int movecount):
    PursuitState(p), players_(players), movecount_(movecount) {}

MMPursuitState::MMPursuitState(const vector<Pos> &p, double prob,
                               const vector<bool>& players, int movecount):
    PursuitState(p, prob), players_(players), movecount_(movecount) {}

ProbDistribution MMPursuitState::PerformAction(const vector<shared_ptr<Action>> &actions2) {
  vector<shared_ptr<PursuitAction>> actions = Cast<Action, PursuitAction>(actions2);
  int count = 2;
  if (movecount_ == 1) {
    count *= 2;
  }
  int actionssize = actions.size();
  // number of all combinations
  vector<std::pair<Outcome, double>> pairs;
  for (int k = 0; k < count; ++k) {  // TODO(rozlijak): prepocitat
    auto rew = vector<double>(place_.size());
    double probability = prob_;
    // making moves and assigning probability
    vector<Pos> moves = place_;
    for (int i = 0; i < actionssize; ++i) {
      if (actions[i]->GetID() > -1) {
        moves[i].x = m_[actions[i]->GetMove()].x + place_[i].x;
        moves[i].y = m_[actions[i]->GetMove()].y + place_[i].y;
      }
      probability *= probdis_[1];
    }
    unique_ptr<MMPursuitState> s;
    if (movecount_ == 1) {
      vector<bool> pla2(place_.size(), true);
      pla2[k & 1] = false;
      s = MakeUnique<MMPursuitState>(moves, probability, pla2, (k >> 1) + 1);
    } else {
      s = MakeUnique<MMPursuitState>(moves, probability, players_, movecount_ - 1);
    }
    int size = s->place_.size();
    moves.clear();
    // detection if first has caught the others
    for (int i = 1; i < size; ++i) {
      if ((s->place_[0].x == place_[i].x && s->place_[0].y == place_[i].y &&
           s->place_[i].x == place_[0].x &&
           s->place_[i].y == place_[0].y) ||
          (s->place_[0].x == s->place_[i].x && s->place_[0].y == s->place_[i].y)) {
        ++rew[0];
        --rew[i];
      }
    }
    int index;
    int id;
    auto obs = vector<unique_ptr<Observation>>();
    vector<int> ob = vector<int>();
    for (int m = 0, pom = 0; m < size; ++m, ++pom) {  // making observations
      if (actions[m]->GetID() == -1) {
        obs.push_back(MakeUnique<Observation>(NoOb));
        --pom;
        continue;
      }

      id = 0;
      for (int i = 0, p = 0; i < size; ++i, ++p) {
        if (m == i) {
          --p;
          continue;
        }
        index = 0;
        int id2 = 1;
        for (unsigned int l = 1; l < eight_.size(); ++l) {
          if ((s->place_[m].x + eight_[l].x) >= 0 && (s->place_[m].x + eight_[l].x)
                                                     < PursuitDomain::width_ &&
              (s->place_[m].y + eight_[l].y) >= 0 && (s->place_[m].y + eight_[l].y)
                                                     < PursuitDomain::height_) {
            id2++;
          }

          if (s->place_[m].x + eight_[l].x == s->place_[i].x &&
              s->place_[m].y + eight_[l].y == s->place_[i].y) {
            index = l;
          }
        }
        ob.push_back(index);
        id += index * pow(id2, p);  // counting observation id
      }
      ob.push_back(1);
      obs.push_back(MakeUnique<PursuitObservation>(id, ob));
      ob.clear();
    }

    double p2 = s->prob_/prob_;
    Outcome p(move(s), move(obs), rew);
    pairs.emplace_back(move(p), p2);  // pair of an outcome and its probability
  }
  ProbDistribution prob(move(pairs));
  return prob;
}

const int MMPursuitState::GetNumPlayers() const {
  int sum = 0;
  for (auto i : players_) {
    if (i) {
      ++sum;
    }
  }
  return sum;
}


PursuitDomain::PursuitDomain(const vector<Pos> &loc, int maxplayers, int max):
    Domain(maxplayers, max) {
  root_ = MakeUnique<PursuitState>(loc);
}

int PursuitDomain::height_ = 0;
int PursuitDomain::width_ = 0;


string PursuitDomain::GetInfo() {
  shared_ptr<PursuitState> root = std::dynamic_pointer_cast<PursuitState>(root_);
  return "Rozmery pole jsou: " + to_string(PursuitDomain::height_) + " x " +
         to_string(PursuitDomain::width_) + " maximalni hloubka grafu je: " +
         to_string(maxdepth_) + " a pocatecni stav je: " +
         to_string(root->GetPlace()[0].x) +" "+ to_string(root->GetPlace()[0].y) +
         "    " + to_string(root->GetPlace()[1].x) +" "+
         to_string(root->GetPlace()[1].y) + "\n";
}

int count = 0;
vector<double> reward;
std::vector<string> graph;
std::vector<int> pole;
std::vector<int> playarr;
std::vector<int> arrid;

void Pursuit(const unique_ptr<Domain>& domain, State *state,
             int depth, int players) {
  if (state == nullptr) {
    throw("State is NULL");
  }

  if (depth == 0) {
    return;
  }
  auto v = vector<vector<shared_ptr<Action>>>();
  for (int i = 0; i < players; ++i) {
    v.emplace_back(state->GetActions(i));
  }
  auto action = CartProduct<shared_ptr<Action>>(v);
  for (const auto &k : action) {
    ProbDistribution prob = state->PerformAction(k);
    for (Outcome &o : prob.GetOutcomes()) {
      for (unsigned int i = 0; i < reward.size(); ++i) {
        reward[i] += o.GetReward()[i];
      }
      ++count;
      Pursuit(domain, o.GetState().get(), depth - 1, players);
    }
  }
}
