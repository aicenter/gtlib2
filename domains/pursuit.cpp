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
  players_ = vector<int>(p.size());
  std::iota(players_.begin(), players_.end(), 0);
}

vector<shared_ptr<Action>> PursuitState::getAvailableActionsFor(int player) const {
  auto list = vector<shared_ptr<Action>>();
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
  return list;
}

OutcomeDistribution PursuitState::performActions(const unordered_map<int, shared_ptr<Action>> &actions2) const {

  vector<shared_ptr<PursuitAction>> actions;
  for(auto &i : actions2) {
    actions.emplace_back(std::dynamic_pointer_cast<PursuitAction>(i.second));
  }
  // number of all combinations
  int actionssize = actions.size();
  int powsize = actions.size();
  for (auto &i : actions) {
    if (i->getId() == -1)
      powsize--;
  }
  int count = (1 << powsize);
  OutcomeDistribution prob;
  prob.reserve(count);
  for (unsigned int k = 0; k < count; ++k) {
    double probability = prob_;
    unordered_map<int,shared_ptr<Observation>> observations = unordered_map<int,shared_ptr<Observation>>();
    unordered_map<int,double> rewards = unordered_map<int,double>();
    observations.reserve(place_.size());
    rewards.reserve(place_.size());
    // making moves and assigning probability
    vector<Pos> moves = place_;
    for (unsigned int i = 0; i < actionssize; ++i) {
      if ((k >> i & 1) == 1) {
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
        ++rewards[0];
        --rewards[i];
      }
    }
    int index;
    int maximum = 0;
    int id;
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
      id += (k >> m & 1) * maximum;
      ob.push_back((k >> m & 1));
      observations[m] = std::make_unique<PursuitObservation>(id, ob);
      ob.clear();
    }
    D(for (unsigned int j = 0; j < size; ++j) {
      s->AddString(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
                   "  | OBS: " + observations[j]->toString(), j);
    })

    double p2 = s->prob_/prob_;
    Outcome o(move(s), observations, rewards);
    prob.emplace_back(move(o), p2);  // pair of an outcome and its probability
  }
  return prob;
}


MMPursuitState::MMPursuitState(const vector<Pos> &p,
                               const vector<int>& players, vector<int> numberOfMoves):
    MMPursuitState(p, players, numberOfMoves, numberOfMoves[0]) {}

MMPursuitState::MMPursuitState(const vector<Pos> &p, const vector<int> &players,
                               vector<int> numberOfMoves, int currentNOM):
    PursuitState(p), players_(players),
    numberOfMoves_(numberOfMoves), currentNOM_(currentNOM)  {}

MMPursuitState::MMPursuitState(const vector<Pos> &p, double prob,
                               const vector<int>& players, vector<int> numberOfMoves):
    MMPursuitState(p, prob, players, numberOfMoves, numberOfMoves[0]) {}

MMPursuitState::MMPursuitState(const vector<Pos> &p, double prob,
                               const vector<int>& players, vector<int> numberOfMoves,
                               int currentNOM):
    PursuitState(p, prob), players_(players),
    numberOfMoves_(numberOfMoves), currentNOM_(currentNOM) {}


OutcomeDistribution //TODO: upravit vice tahu vice hracu - zjistit princip
MMPursuitState::performActions(const unordered_map<int, shared_ptr<Action>> &actions2) const {
  vector<shared_ptr<PursuitAction>> actions;
  for(auto &i : actions2) {
    actions.emplace_back(std::dynamic_pointer_cast<PursuitAction>(i.second));
  }
  //unsigned int count = 2;
  int actionssize = actions.size();
  // number of all combinations
  OutcomeDistribution prob;
  prob.reserve(players_.size());
  for (auto &k:players_) {
    unordered_map<int,shared_ptr<Observation>> observations = unordered_map<int,shared_ptr<Observation>>();
    unordered_map<int,double> rewards = unordered_map<int,double>();
    observations.reserve(place_.size());
    rewards.reserve(place_.size());
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
    if (/*movecount_ == */1) {
      vector<int> pla2 = {k};
      s = make_shared<MMPursuitState>(moves, probability, pla2, numberOfMoves_, (k >> 1) + 1);
    } else {
      //s = make_shared<MMPursuitState>(moves, probability, players_, movecount_ - 1);
    }
    int size = s->place_.size();
    moves.clear();
    // detection if first has caught the others
    for (int i = 1; i < size; ++i) {
      if ((s->place_[0].x == place_[i].x && s->place_[0].y == place_[i].y &&
           s->place_[i].x == place_[0].x &&
           s->place_[i].y == place_[0].y) ||
          (s->place_[0].x == s->place_[i].x && s->place_[0].y == s->place_[i].y)) {
        ++rewards[0];
        --rewards[i];
      }
    }
    int index;
    int id;
    vector<int> ob = vector<int>();
    ob.reserve(size+1);
    for (int m = 0, pom = 0; m < size; ++m, ++pom) {  // making observations
      if (actions[m]->getId() == -1 /*&& movecount_ > 1*/) {
        observations[m] = std::make_unique<Observation>(-1);
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
      observations[m] = std::make_unique<PursuitObservation>(id, ob);
      ob.clear();
    }
    D(for (int j = 0; j < size; ++j) {
      s->AddString(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
                   "  | OBS: " + observations[j]->toString(), j);
    })
    double p2 = s->prob_/prob_;
    Outcome p(move(s), observations, rewards);
    prob.emplace_back(move(p), p2);  // pair of an outcome and its probability
  }
  return prob;
}


ObsPursuitState::ObsPursuitState(const vector<Pos> &p) : PursuitState(p) {}

ObsPursuitState::ObsPursuitState(const vector<Pos> &p, double prob) :
    PursuitState(p, prob) {}
OutcomeDistribution
ObsPursuitState::performActions(const unordered_map<int, shared_ptr<Action>> &actions2) const {
  vector<shared_ptr<PursuitAction>> actions;
  for(auto &i : actions2) {
    actions.emplace_back(std::dynamic_pointer_cast<PursuitAction>(i.second));
  }
  // number of all combinations
  int actionssize = actions.size();
  int powsize = actions.size();
  for (auto &i : actions) {
    if (i->getId() == -1)
      powsize--;
  }
  int count = 1 << powsize;
  OutcomeDistribution prob;
  prob.reserve(count);
  for (int k = 0; k < count; ++k) {
    unordered_map<int,shared_ptr<Observation>> observations = unordered_map<int,shared_ptr<Observation>>();
    unordered_map<int,double> rewards = unordered_map<int,double>();
    observations.reserve(place_.size());
    rewards.reserve(place_.size());
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
        ++rewards[0];
        --rewards[i];
      }
    }
    int id;
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
      observations[m] = std::make_unique<PursuitObservationLoc>(id, ob);
      ob.clear();
    }
    D(for (int j = 0; j < size; ++j) {
      s->AddString(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
                   "  | OBS: " + observations[j]->toString(), j);
    })
    double p2 = s->prob_/prob_;
    Outcome o(move(s), observations, rewards);
    prob.emplace_back(move(o), p2);  // pair of an outcome and its probability
  }
  return prob;
}


PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const vector<Pos> &loc, int height, int width):
    Domain(max, numberOfPlayers) {
  PursuitDomain::height_ = height;
  PursuitDomain::width_ = width;
  auto state = make_shared<PursuitState>(loc);
  unordered_map<int, double> rewards;
  unordered_map<int, shared_ptr<Observation>> Obs;
  for(int j = 0; j < numberOfPlayers; ++j) {
    rewards[j] = 0.0;
    Obs[j] = make_shared<Observation>(-1);
  }
  Outcome o(state, Obs, rewards);
  rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
}

int PursuitDomain::height_ = 0;
int PursuitDomain::width_ = 0;


string PursuitDomain::getInfo() const {
  return "Rozmery pole jsou: " + to_string(PursuitDomain::height_) + " x " +
         to_string(PursuitDomain::width_) + "\nmaximalni hloubka grafu je: " +
         to_string(maxDepth) + "\na pocatecni stav je: " +
         rootStatesDistribution[0].first.state->toString(0) +
         rootStatesDistribution[0].first.state->toString(1);
}

PursuitDomain::PursuitDomain(unsigned int max, int height, int width) :
    PursuitDomain(max, 2, vector<Pos>{{0, 0}, {height - 1,
                      width - 1}},height,width) {}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const shared_ptr<MMPursuitState> &state,
                             int height, int width):
    Domain(max, numberOfPlayers) {
  PursuitDomain::height_ = height;
  PursuitDomain::width_ = width;
  unordered_map<int, double> rewards;
  unordered_map<int, shared_ptr<Observation>> Obs;
  for (int j = 0; j < numberOfPlayers; ++j) {
    rewards[j] = 0.0;
    Obs[j] = make_shared<Observation>(-1);
  }

  Outcome o(state, Obs, rewards);
  rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
}

vector<int> PursuitDomain::getPlayers() const {
  auto players = vector<int>(numberOfPlayers);
  std::iota(players.begin(), players.end(), 0);
  return players;
}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const shared_ptr<ObsPursuitState> &state,
                             int height, int width):
    Domain(max, numberOfPlayers) {
  PursuitDomain::height_ = height;
  PursuitDomain::width_ = width;
  unordered_map<int, double> rewards;
  unordered_map<int, shared_ptr<Observation>> Obs;
  for (int j = 0; j < numberOfPlayers; ++j) {
    rewards[j] = 0.0;
    Obs[j] = make_shared<Observation>(-1);
  }
  Outcome o(state, Obs, rewards);
  rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
}

PursuitDomainChance::PursuitDomainChance(unsigned int max,
                                         unsigned int numberOfPlayers,
                                         const vector<Pos> &firstPlayerLocation,
                                         const vector<Pos> &secondPlayerLocation,
                                         int height, int width):
    PursuitDomain(max, numberOfPlayers, {firstPlayerLocation[0],secondPlayerLocation[0]},
                  height, width){
  rootStatesDistribution.clear();
  for (int i = 0; i < firstPlayerLocation.size(); ++i) {
    auto state = make_shared<PursuitState>(vector<Pos>{firstPlayerLocation[i],
                                                       secondPlayerLocation[i]});
    unordered_map<int, double> rewards;
    unordered_map<int, shared_ptr<Observation>> Obs;
    for(int j = 0; j < numberOfPlayers; ++j) {
      rewards[j] = 0.0;
      Obs[j] = make_shared<Observation>(-1);
    }

    Outcome o(state, Obs, rewards);
    rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
  }
}

PursuitDomainChance::PursuitDomainChance(unsigned int max,
                                         unsigned int numberOfPlayers,
                                         const shared_ptr<MMPursuitState> &state,
                                         int height, int width)
    : PursuitDomain(max, numberOfPlayers, state, height, width) { // TODO: rozlisit MMPursuitState od normalniho
  vector<Pos> start1 = {{0, 0}, {0, 1}};
  vector<Pos> start2 = {{1, 0}, {1, 1}};
  for (int i = 0; i < start1.size(); ++i) {
    auto new_state = make_shared<PursuitState>(vector<Pos>{start1[i], start2[i]});
    unordered_map<int, double> rewards;
    unordered_map<int, shared_ptr<Observation>> Obs;
    for(int j = 0; j < numberOfPlayers; ++j) {
      rewards[j] = 0.0;
      Obs[j] = make_shared<Observation>(-1);
    }

    Outcome o(new_state, Obs, rewards);
    rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
  }
}

vector<double> reward;


void PursuitStart(const shared_ptr<Domain>& domain, unsigned int depth) {
  if (depth == 0)
    depth = domain->getMaxDepth();
  auto distr = domain->getRootStatesDistribution(); //domain->getRootStateDistributionPtr()->GetOutcomes();
  for (auto &o : distr) {
    for(int j = 0; j < reward.size(); ++j) {
      reward[j] += o.first.rewards[j];
    }
    Pursuit(domain, o.first.state.get(), depth, domain->getNumberOfPlayers());
  }
}


void Pursuit(const shared_ptr<Domain>& domain, State *state,
             unsigned int depth, int players) {
  if (state == nullptr) {
    return;
  }

  if (depth == 0) {
    cout <<"leaf\n";
    return;
  }
  auto v = vector<vector<shared_ptr<Action>>>();
  for (int i = 0; i < players; ++i) {
    v.emplace_back(state->getAvailableActionsFor(i));
  }
  auto action = CartProduct(v);
  for (const auto &k : action) {
    unordered_map<int, shared_ptr<Action>> mp;
    for(int x = 0; x < k.size(); ++x) {
      mp[x] = k[x];
    }
    OutcomeDistribution distr = state->performActions(mp);
    for (auto &o : distr) {
      for(int j = 0; j < reward.size(); ++j) {
        reward[j] += o.first.rewards[j];
      }
      Pursuit(domain, o.first.state.get(), depth - 1, players);
    }
  }
}

#pragma clang diagnostic pop