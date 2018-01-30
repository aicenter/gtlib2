//
// Created by rozliv on 02.08.2017.
//

#include "pursuit.h"
#include "assert.h"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifdef DString
#  define D(x) x
#else
#  define D(x) x
#endif  // MyDEBUG

PursuitAction::PursuitAction(int id, int move): Action(id), move_(move) {}


PursuitObservation::PursuitObservation(int id, vector<int> values):
    Observation(id), values_(move(values)) {}

string PursuitObservation::toString() const {
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

PursuitObservationLoc::PursuitObservationLoc(int id, vector<Pos> values):
    Observation(id), values_(move(values)) {}

string PursuitObservationLoc::toString() const {
  string string1;
  if (values_.empty()) {
    return "NoOb;";
  }
  for (unsigned int i = 0; i < values_.size()-1; ++i) {
    string1 += to_string(values_[i].x);
    string1 += ", ";
    string1 += to_string(values_[i].y);
    string1 += "; ";
  }
  string1 += "Result: ";
  string1 += values_[values_.size()-1].y == 1 ? "ok" : "fail";
  return string1;
}


PursuitState::PursuitState(const vector<Pos> &p):PursuitState(p, 1) {}

PursuitState::PursuitState(const vector<Pos> &p, double prob):
    place_(p), prob_(prob) {
  probdis_ = {0.1, 0.9};
  strings_ = vector<string>(p.size());
  players_ = vector<bool>(p.size(), true);
}

vector<shared_ptr<Action>> PursuitState::getAvailableActionsFor(int player) const {
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
      list.push_back(make_shared<PursuitAction>(count, i));
      count++;
    }
  }
}

OutcomeDistributionOld PursuitState::PerformAction(const vector<shared_ptr<Action>>& actions2) {
  vector<shared_ptr<PursuitAction>> actions = Cast<Action, PursuitAction>(actions2);
  // number of all combinations
  int actionssize = actions.size();
  int powsize = actions.size();
  for (auto &i : actions) {
    if (i->getId() == -1)
      powsize--;
  }
  int count = (1 << powsize);
  vector<pair<Outcome, double>> pairs;
  pairs.reserve(count);
  for (int k = 0; k < count; ++k) {
    auto rew = vector<double>(place_.size());
    double probability = prob_;
    // making moves and assigning probability
    vector<Pos> moves = place_;
    for (int i = 0; i < actionssize; ++i) {
      if (((k >> i) & 1) == 1) {
        if (actions[i]->getId() > -1) {
          moves[i].x = m_[actions[i]->GetMove()].x + place_[i].x;
          moves[i].y = m_[actions[i]->GetMove()].y + place_[i].y;
        }
        probability *= probdis_[1];
      } else {
        probability *= probdis_[0];
      }
    }

    shared_ptr<PursuitState> s = make_shared<PursuitState>(moves, probability);
    unsigned int size = s->place_.size();
    moves.clear();
    // detection if first has caught others
    for (unsigned int i = 1; i < size; ++i) {
      if ((s->place_[0].x == place_[i].x && s->place_[0].y == place_[i].y &&
           s->place_[i].x == place_[0].x &&
           s->place_[i].y == place_[0].y) ||
          (s->place_[0].x == s->place_[i].x && s->place_[0].y == s->place_[i].y)) {
        ++rew[0];
        --rew[i];
      }
    }
    int index;
    int maximum = 0;
    int id;
    auto obser = vector<shared_ptr<Observation>>();
    obser.reserve(size);
    vector<int> ob = vector<int>();
    ob.reserve(size+1);
    for (unsigned int m = 0; m < size; ++m) {  // making observations
      id = 0;
      maximum = 0;
      for (unsigned int i = 0, p = 0; i < size; ++i, ++p) {
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
        maximum += pow(id2, p+1);
        ob.push_back(index);
        id += index * pow(id2, p);  // counting observation id
      }
      id += ((k >> m) & 1) * maximum;
      ob.push_back(((k >> m) & 1));
      obser.push_back(MakeUnique<PursuitObservation>(id, ob));
      ob.clear();
    }
    D(for (unsigned int j = 0; j < size; ++j) {
      s->AddString(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
                   "  | OBS: " + obser[j]->toString(), j);
    })

    double p2 = s->prob_/prob_;
    Outcome p(move(s), move(obser), rew);
    pairs.emplace_back(move(p), p2);  // pair of an outcome and its probability
  }
  OutcomeDistributionOld prob(move(pairs));
  return prob;
}

OutcomeDistribution PursuitState::performActions(const unordered_map<int, shared_ptr<Action>> &actions) const {
    assert(("To be implemented",false));
    return OutcomeDistribution();
}


MMPursuitState::MMPursuitState(const vector<Pos> &p,
                               const vector<bool>& players, int movecount):
    PursuitState(p), players_(players), movecount_(movecount) {}

MMPursuitState::MMPursuitState(const vector<Pos> &p, double prob,
                               const vector<bool>& players, int movecount):
    PursuitState(p, prob), players_(players), movecount_(movecount) {}

OutcomeDistributionOld MMPursuitState::PerformAction(const vector<shared_ptr<Action>> &actions2) {
  vector<shared_ptr<PursuitAction>> actions = Cast<Action, PursuitAction>(actions2);
  unsigned int count = 2;
//  if (movecount_ == 1) {
//    count *= 2;
//  }
  int actionssize = actions.size();
  // number of all combinations
  vector<pair<Outcome, double>> pairs;
  pairs.reserve(count);
  for (int k = 0; k < count; ++k) {
    auto rew = vector<double>(place_.size());
    double probability = prob_;
    // making moves and assigning probability
    vector<Pos> moves = place_;
    for (int i = 0; i < actionssize; ++i) {
      if (actions[i]->getId() > -1) {
        moves[i].x = m_[actions[i]->GetMove()].x + place_[i].x;
        moves[i].y = m_[actions[i]->GetMove()].y + place_[i].y;
      }
    }
    shared_ptr<MMPursuitState> s;
    if (movecount_ == 1) {
      vector<bool> pla2(place_.size(), true);
      pla2[k & 1] = false;
      s = make_shared<MMPursuitState>(moves, probability, pla2, (k >> 1) + 1);
    } else {
      s = make_shared<MMPursuitState>(moves, probability, players_, movecount_ - 1);
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
    auto obs = vector<shared_ptr<Observation>>();
    obs.reserve(size);
    vector<int> ob = vector<int>();
    ob.reserve(size+1);
    for (int m = 0, pom = 0; m < size; ++m, ++pom) {  // making observations
      if (actions[m]->getId() == -1 && movecount_ > 1) {
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
    D(for (int j = 0; j < size; ++j) {
      s->AddString(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
                   "  | OBS: " + obs[j]->toString(), j);
    })
    double p2 = s->prob_/prob_;
    Outcome p(move(s), move(obs), rew);
    pairs.emplace_back(move(p), p2);  // pair of an outcome and its probability
  }
  OutcomeDistributionOld prob(move(pairs));
  return prob;
}

int MMPursuitState::getNumberOfPlayers() const {
  int sum = 0;
  for (auto i : players_) {
    if (i) {
      ++sum;
    }
  }
  return sum;
}


ObsPursuitState::ObsPursuitState(const vector<Pos> &p) : PursuitState(p) {}

ObsPursuitState::ObsPursuitState(const vector<Pos> &p, double prob) :
    PursuitState(p, prob) {}

OutcomeDistributionOld ObsPursuitState::PerformAction(const vector<shared_ptr<Action>> &actions2) {
  vector<shared_ptr<PursuitAction>> actions = Cast<Action, PursuitAction>(actions2);
  // number of all combinations
  int actionssize = actions.size();
  int powsize = actions.size();
  for (auto &i : actions) {
    if (i->getId() == -1)
      powsize--;
  }
  int count = 1 << powsize;
  vector<pair<Outcome, double>> pairs;
  pairs.reserve(count);
  for (int k = 0; k < count; ++k) {
    auto rew = vector<double>(place_.size());
    double probability = prob_;
    // making moves and assigning probability
    vector<Pos> moves = place_;
    for (int i = 0; i < actionssize; ++i) {
      if (((k >> i) & 1) == 1) {
        if (actions[i]->getId() > -1) {
          moves[i].x = m_[actions[i]->GetMove()].x + place_[i].x;
          moves[i].y = m_[actions[i]->GetMove()].y + place_[i].y;
        }
        probability *= probdis_[1];
      } else {
        probability *= probdis_[0];
      }
    }

    shared_ptr<ObsPursuitState> s =
        make_shared<ObsPursuitState>(moves, probability);
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
    int id;
    auto obs = vector<shared_ptr<Observation>>();
    obs.reserve(size);
    vector<Pos> ob = vector<Pos>();
    ob.reserve(size + 1);
    for (int m = 0; m < size; ++m) {  // making observations
      id = 0;
      for (int i = 0, p = 0; i < size; ++i, ++p) {
        if (m == i) {
          --p;
          continue;
        }
        int id2 = PursuitDomain::width_ * PursuitDomain::height_;

        ob.push_back({s->place_[i].y, s->place_[i].x});
        // counting observation id
        id += (s->place_[i].x + s->place_[i].y * PursuitDomain::width_) * pow(id2, p);
      }
      ob.push_back({((k >> m) & 1), -1});
      id += ((k >> m) & 1) * ob.size();
      obs.push_back(MakeUnique<PursuitObservationLoc>(id, ob));
      ob.clear();
    }
    D(for (int j = 0; j < size; ++j) {
      s->AddString(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
                   "  | OBS: " + obs[j]->toString(), j);
    })
    double p2 = s->prob_/prob_;
    Outcome p(move(s), move(obs), rew);
    pairs.emplace_back(move(p), p2);  // pair of an outcome and its probability
  }
  OutcomeDistributionOld prob(move(pairs));
  return prob;
}


PursuitDomain::PursuitDomain(unsigned int max, unsigned int maxplayers,
                             const vector<Pos> &loc):
    Domain(max, maxplayers) {
  vector<pair<Outcome, double>> pairs;
  Outcome o(make_shared<PursuitState>(loc),
            move(vector<shared_ptr<Observation>>(loc.size())),
            vector<double>(loc.size()));
  pairs.emplace_back(move(o), 1);
  rootStatesDistributionPtr = make_shared<OutcomeDistributionOld>(move(pairs));
}

int PursuitDomain::height_ = 0;
int PursuitDomain::width_ = 0;


string PursuitDomain::getInfo() const {
  return "Rozmery pole jsou: " + to_string(PursuitDomain::height_) + " x " +
      to_string(PursuitDomain::width_) + "\nmaximalni hloubka grafu je: " +
      to_string(maxDepth) + "\na pocatecni stav je: " +
          rootStatesDistributionPtr->GetOutcomes()[0].GetState()->toString(0) +
          rootStatesDistributionPtr->GetOutcomes()[0].GetState()->toString(1);
}

PursuitDomain::PursuitDomain(unsigned int max) :
    PursuitDomain(max, 2, vector<Pos>{{0, 0}, {PursuitDomain::height_ - 1,
                      PursuitDomain::width_ - 1}}) {}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int maxplayers,
                             const shared_ptr<MMPursuitState> &state):
    Domain(max, maxplayers) {
  vector<pair<Outcome, double>> pairs;
  Outcome o(state, move(vector<shared_ptr<Observation>>(maxplayers)),
            vector<double>(maxplayers));
  pairs.emplace_back(move(o), 1);
  rootStatesDistributionPtr = make_shared<OutcomeDistributionOld>(move(pairs));
}




PursuitDomainChance::PursuitDomainChance(unsigned int max,
                                         unsigned int maxplayers,
                                         const vector<Pos> &loc):
    PursuitDomain(max, maxplayers, loc) {
  vector<Pos> start1 = {{0, 0}, {0, 1}};
  vector<Pos> start2 = {{1, 0}, {1, 1}};
  vector<pair<Outcome, double>> pairs;
  for (int i = 0; i < 3; ++i) {
    Outcome o(make_shared<PursuitState>(vector<Pos>{start1[i], start2[2]}),
              move(vector<shared_ptr<Observation>>(loc.size())),
              vector<double>(loc.size()));
    pairs.emplace_back(move(o), 1);
  }
  rootStatesDistributionPtr = make_shared<OutcomeDistributionOld>(move(pairs));
}

PursuitDomainChance::PursuitDomainChance(unsigned int max) :
    PursuitDomainChance(max, 2, vector<Pos>{{0, 0}, {PursuitDomain::height_ - 1,
                            PursuitDomain::width_ - 1}}) {}

PursuitDomainChance::PursuitDomainChance(unsigned int max,
                                         unsigned int maxplayers,
                                         const shared_ptr<MMPursuitState> &state)
    : PursuitDomain(max, maxplayers, state) {
  vector<Pos> start1 = {{0, 0}, {0, 1}};
  vector<Pos> start2 = {{1, 0}, {1, 1}};
//  vector<Pos> start1 = {{0, 0}, {0, 1}, {1, 0}};
//  vector<Pos> start2 = {{1, 2}, {2, 1}, {2, 2}};
  vector<pair<Outcome, double>> pairs;
  for (int i = 0; i < 3; ++i) {
    Outcome o(state, move(vector<shared_ptr<Observation>>(maxplayers)),
              vector<double>(maxplayers));
    pairs.emplace_back(move(o), 1);
  }
  rootStatesDistributionPtr = make_shared<OutcomeDistributionOld>(move(pairs));
}


extern int countStates;  // temporary for testing treewalk
vector<double> reward;


void PursuitStart(const shared_ptr<Domain>& domain, unsigned int depth) {
  if (depth == 0)
    depth = domain->getMaxDepth();
  vector<Outcome> outcomes = domain->getRootStateDistributionPtr()->GetOutcomes();
  for (Outcome &o : outcomes) {
    reward += o.GetReward();
    ++countStates;
    Pursuit(domain, o.GetState().get(), depth, domain->getNumberOfPlayers());
  }
}

void Pursuit(const shared_ptr<Domain>& domain, State *state,
             unsigned int depth, int players) {
  if (state == nullptr) {
    return;
  }

  if (depth == 0) {
    return;
  }
  auto v = vector<vector<shared_ptr<Action>>>();
  for (int i = 0; i < players; ++i) {
    v.emplace_back(state->getAvailableActionsFor(i));
  }
  auto action = CartProduct(v);
  for (const auto &k : action) {
    OutcomeDistributionOld prob = state->PerformAction(k);
    for (Outcome &o : prob.GetOutcomes()) {
      reward += o.GetReward();
      ++countStates;
      Pursuit(domain, o.GetState().get(), depth - 1, players);
    }
  }
}

#pragma clang diagnostic pop