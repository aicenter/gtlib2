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


#include "domains/pursuit.h"
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
PursuitAction::PursuitAction(int id, int move) : Action(id), move_(move) {}

bool PursuitAction::operator==(const Action &that) const {
  if (typeid(*this) == typeid(that)) {
    const auto rhsAction = static_cast<const PursuitAction *>(&that);
    return this->move_ == rhsAction->move_;
  }
  return false;
}

size_t PursuitAction::getHash() const {
  std::hash<int> h;
  return h(move_);
}

PursuitObservation::PursuitObservation(int id, vector<int> values) :
    Observation(id), values_(move(values)) {}

string PursuitObservation::toString() const {
  string string1;
  if (values_.empty()) {
    return "NoOb;";
  }
  for (unsigned int i = 0; i < values_.size() - 1; ++i) {
    string1 += eightdes_[values_[i]];
    string1 += "; ";
  }
  string1 += "Result: ";
  string1 += values_[values_.size() - 1] == 1 ? "ok" : "fail";
  return string1;
}

PursuitObservationLoc::PursuitObservationLoc(int id, vector<Pos> values) :
    Observation(id), values_(move(values)) {}

string PursuitObservationLoc::toString() const {
  string string1;
  if (values_.empty()) {
    return "NoOb;";
  }
  for (unsigned int i = 0; i < values_.size() - 1; ++i) {
    string1 += to_string(values_[i].x);
    string1 += ", ";
    string1 += to_string(values_[i].y);
    string1 += "; ";
  }
  string1 += "Result: ";
  string1 += values_[values_.size() - 1].y == 1 ? "ok" : "fail";
  return string1;
}

PursuitState::PursuitState(Domain *domain, const vector<Pos> &p) : PursuitState(domain, p, 1) {}

PursuitState::PursuitState(Domain *domain, const vector<Pos> &p, double prob) :
    State(domain), place_(p), prob_(prob) {
  strings_ = vector<string>(p.size());
  players_ = vector<int>(p.size());
  std::iota(players_.begin(), players_.end(), 0);
}

vector<shared_ptr<Action>> PursuitState::getAvailableActionsFor(int player) const {
  auto list = vector<shared_ptr<Action>>();
  for (int i = 1; i < players_.size(); ++i) {
    if (place_[players_[0]].x == place_[players_[i]].x && place_[0].y == place_[players_[i]].y) {
      return list;
    }
  }
  auto purDomain = static_cast<PursuitDomain *>(domain);
  int count = 0;
  for (int i = 1; i < 5; ++i) {  // verifies whether moves are correct
    if ((place_[player].x + pursuitMoves[i].x) >= 0 && (place_[player].x + pursuitMoves[i].x)
        < purDomain->width &&
        (place_[player].y + pursuitMoves[i].y) >= 0 && (place_[player].y + pursuitMoves[i].y)
        < purDomain->height) {
      list.push_back(make_shared<PursuitAction>(count, i));
      count++;
    }
  }
  return list;
}

OutcomeDistribution
PursuitState::performActions(const vector<pair<int, shared_ptr<Action>>> &actions2) const {
  vector<PursuitAction*> actions(actions2.size());
  auto purDomain = static_cast<PursuitDomain *>(domain);
  for (auto &i : actions2) {
    actions[i.first] = dynamic_cast<PursuitAction*>(i.second.get());
  }
  // number of all combinations
  auto actionssize = static_cast<unsigned int>(actions.size());
  auto powsize = static_cast<unsigned int>(actions.size());
  for (auto &i : actions) {
    if (i->getId() == -1)
      powsize--;
  }
  int count = (1 << powsize);
  OutcomeDistribution prob;
  prob.reserve(static_cast<unsigned long>(count));
//  count = 1;
  for (unsigned int k = 0; k < count; ++k) {
    double probability = prob_;
    vector<shared_ptr<Observation>> observations(place_.size());
    vector<double> rewards(place_.size());
    // making moves and assigning probability
    vector<Pos> moves = place_;
    for (unsigned int i = 0; i < actionssize; ++i) {
      if ((k >> i & 1) == 1) {
        if (actions[i]->getId() > -1) {
          moves[i].x = pursuitMoves[actions[i]->GetMove()].x + place_[i].x;
          moves[i].y = pursuitMoves[actions[i]->GetMove()].y + place_[i].y;
        }
        probability *= purDomain->probability[1];
      } else {
        probability *= purDomain->probability[0];
      }
    }

    shared_ptr<PursuitState> s = make_shared<PursuitState>(domain, moves, probability);
    auto size = static_cast<unsigned int>(s->place_.size());
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
    ob.reserve(size + 1);
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
        for (unsigned int l = 1; l < pursuitEightSurrounding.size(); ++l) {
          if ((s->place_[m].x + pursuitEightSurrounding[l].x) >= 0
              && (s->place_[m].x + pursuitEightSurrounding[l].x)
                  < purDomain->width &&
              (s->place_[m].y + pursuitEightSurrounding[l].y) >= 0
              && (s->place_[m].y + pursuitEightSurrounding[l].y)
                  < purDomain->height) {
            id2++;
          }

          if (s->place_[m].x + pursuitEightSurrounding[l].x == s->place_[i].x &&
              s->place_[m].y + pursuitEightSurrounding[l].y == s->place_[i].y) {
            index = l;
          }
        }
        maximum += pow(id2, p + 1);
        ob.push_back(index);
        id += index * pow(id2, p);  // counting observation id
      }
      id += (k >> m & 1) * maximum;
      ob.push_back((k >> m & 1));
//      ob.push_back(1);
      observations[m] = std::make_shared<PursuitObservation>(id, ob);
      ob.clear();
    }
    DebugString(for (unsigned int j = 0; j < size; ++j) {
      s->strings_[j].append(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
          "  | OBS: " + observations[j]->toString());
    })

    double p2 = s->prob_ / prob_;
    Outcome o(move(s), observations, rewards);
    prob.emplace_back(move(o), p2);  // pair of an outcome and its probability
  }
  return prob;
}

bool PursuitState::operator==(const State &rhs) const {
  auto State = dynamic_cast<const PursuitState &>(rhs);

  return place_[0].x == State.place_[0].x &&
      place_[1].x == State.place_[1].x &&
      place_[0].y == State.place_[0].y &&
      place_[1].y == State.place_[1].y &&
      players_ == State.players_ &&
      prob_ == State.prob_;
}

size_t PursuitState::getHash() const {
  size_t seed = 0;
  for (auto &i : place_) {
    boost::hash_combine(seed, i.x);
    boost::hash_combine(seed, i.y);
  }
  for (auto &i : players_) {
    boost::hash_combine(seed, i);
  }
  boost::hash_combine(seed, prob_);
  return seed;
}

MMPursuitState::MMPursuitState(Domain *domain, const vector<Pos> &p,
                               const vector<int> &players, vector<int> numberOfMoves) :
    MMPursuitState(domain, p, players, numberOfMoves, numberOfMoves[0], 0) {}

MMPursuitState::MMPursuitState(Domain *domain, const vector<Pos> &p, const vector<int> &players,
                               vector<int> numberOfMoves, int currentNOM, int currentPlayer) :
    PursuitState(domain, p), players_(players), numberOfMoves_(move(numberOfMoves)),
    currentNOM_(currentNOM), currentPlayer_(currentPlayer) {}

MMPursuitState::MMPursuitState(Domain *domain, const vector<Pos> &p, double prob,
                               const vector<int> &players, vector<int> numberOfMoves) :
    MMPursuitState(domain, p, prob, players, numberOfMoves, numberOfMoves[0], 0) {}

MMPursuitState::MMPursuitState(Domain *domain, const vector<Pos> &p, double prob,
                               const vector<int> &players, vector<int> numberOfMoves,
                               int currentNOM, int currentPlayer) :
    PursuitState(domain, p, prob), players_(players), numberOfMoves_(move(numberOfMoves)),
    currentNOM_(currentNOM), currentPlayer_(currentPlayer) {}

OutcomeDistribution
MMPursuitState::performActions(const vector<pair<int, shared_ptr<Action>>> &actions2) const {
  vector<PursuitAction*> actions(actions2.size());
  auto purDomain = static_cast<PursuitDomain *>(domain);
  for (auto &i : actions2) {
    actions[i.first] = dynamic_cast<PursuitAction*>(i.second.get());
  }
  // unsigned int count = 2;
  auto actionssize = static_cast<unsigned int>(actions.size());
  // number of all combinations
  OutcomeDistribution prob;
  prob.reserve(players_.size());
  for (auto &k : players_) {
    vector<shared_ptr<Observation>> observations(place_.size());
    vector<double> rewards(place_.size());
    double probability = prob_;
    // making moves and assigning probability
    vector<Pos> moves = place_;
    for (int i = 0; i < actionssize; ++i) {
      if (actions[i]->getId() > -1) {
        moves[i].x = pursuitMoves[actions[i]->GetMove()].x + place_[i].x;
        moves[i].y = pursuitMoves[actions[i]->GetMove()].y + place_[i].y;
      }
    }
    shared_ptr<MMPursuitState> s;
    if (currentNOM_ == 1) {
      int newPlayer = (currentPlayer_+1)%players_.size();
      s = make_shared<MMPursuitState>(domain, moves, probability, players_, numberOfMoves_,
          numberOfMoves_[newPlayer], newPlayer);
    } else {
      s = make_shared<MMPursuitState>(domain,
                                      moves,
                                      probability,
                                      players_,
                                      numberOfMoves_,
                                      currentNOM_ - 1,
                                      currentPlayer_);
     }
    auto size = static_cast<unsigned int>(s->place_.size());
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
    ob.reserve(size + 1);
    for (int m = 0, pom = 0; m < size; ++m, ++pom) {  // making observations
      if (actions[m]->getId() == -1 && currentNOM_ > 1) {
        observations[m] = std::make_shared<Observation>(-1);
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
        for (unsigned int l = 1; l < pursuitEightSurrounding.size(); ++l) {
          if ((s->place_[m].x + pursuitEightSurrounding[l].x) >= 0
              && (s->place_[m].x + pursuitEightSurrounding[l].x)
                  < purDomain->width &&
              (s->place_[m].y + pursuitEightSurrounding[l].y) >= 0
              && (s->place_[m].y + pursuitEightSurrounding[l].y)
                  < purDomain->height) {
            id2++;
          }

          if (s->place_[m].x + pursuitEightSurrounding[l].x == s->place_[i].x &&
              s->place_[m].y + pursuitEightSurrounding[l].y == s->place_[i].y) {
            index = l;
          }
        }
        ob.push_back(index);
        id += index * pow(id2, p);  // counting observation id
      }
      ob.push_back(1);
      observations[m] = std::make_shared<PursuitObservation>(id, ob);
      ob.clear();
    }
    DebugString(for (int j = 0; j < size; ++j) {
      s->strings_[j].append(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
          "  | OBS: " + observations[j]->toString());
    })
    double p2 = s->prob_ / prob_;
    Outcome p(move(s), observations, rewards);
    prob.emplace_back(move(p), p2);  // pair of an outcome and its probability
  }
  return prob;
}

bool MMPursuitState::operator==(const State &rhs) const {
  auto State = dynamic_cast<const MMPursuitState &>(rhs);

  return place_[0].x == State.place_[0].x &&
      place_[1].x == State.place_[1].x &&
      place_[0].y == State.place_[0].y &&
      place_[1].y == State.place_[1].y &&
      players_ == State.players_ &&
      prob_ == State.prob_ &&
      numberOfMoves_ == State.numberOfMoves_ &&
      currentNOM_ == State.currentNOM_ &&
      currentPlayer_ == State.currentPlayer_;
}

size_t MMPursuitState::getHash() const {
  size_t seed = 0;
  for (auto &i : place_) {
    boost::hash_combine(seed, i.x);
    boost::hash_combine(seed, i.y);
  }
  for (auto &i : players_) {
    boost::hash_combine(seed, i);
  }
  for (auto &i : numberOfMoves_) {
    boost::hash_combine(seed, i);
  }
  boost::hash_combine(seed, prob_);
  boost::hash_combine(seed, currentPlayer_);
  boost::hash_combine(seed, currentNOM_);
  return seed;
}

ObsPursuitState::ObsPursuitState(Domain *domain, const vector<Pos> &p) :
    PursuitState(domain, p) {}

ObsPursuitState::ObsPursuitState(Domain *domain, const vector<Pos> &p,
                                 double prob) : PursuitState(domain, p, prob) {}

OutcomeDistribution
ObsPursuitState::performActions(const vector<pair<int, shared_ptr<Action>>> &actions2) const {
  vector<PursuitAction*> actions(actions2.size());
  auto purDomain = static_cast<PursuitDomain *>(domain);
  for (auto &i : actions2) {
    actions[i.first] = dynamic_cast<PursuitAction*>(i.second.get());
  }
  // number of all combinations
  auto actionssize = static_cast<unsigned int>(actions.size());
  auto powsize = static_cast<unsigned int>(actions.size());
  for (auto &i : actions) {
    if (i->getId() == -1)
      powsize--;
  }
  int count = 1 << powsize;
  OutcomeDistribution prob;
  prob.reserve(static_cast<size_t>(count));
  for (int k = 0; k < count; ++k) {
    vector<shared_ptr<Observation>> observations(place_.size());
    vector<double> rewards(place_.size());
    double probability = prob_;
    // making moves and assigning probability
    vector<Pos> moves = place_;
    for (int i = 0; i < actionssize; ++i) {
      if (((k >> i) & 1) == 1) {
        if (actions[i]->getId() > -1) {
          moves[i].x = pursuitMoves[actions[i]->GetMove()].x + place_[i].x;
          moves[i].y = pursuitMoves[actions[i]->GetMove()].y + place_[i].y;
        }
        probability *= purDomain->probability[1];
      } else {
        probability *= purDomain->probability[0];
      }
    }

    shared_ptr<ObsPursuitState> s =
        make_shared<ObsPursuitState>(domain, moves, probability);
    auto size = static_cast<unsigned int>(s->place_.size());
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
        int id2 = purDomain->width * purDomain->height;

        ob.push_back({s->place_[i].y, s->place_[i].x});
        // counting observation id
        id += (s->place_[i].x + s->place_[i].y * purDomain->width) * pow(id2, p);
      }
      ob.push_back({((k >> m) & 1), -1});
      id += ((k >> m) & 1) * ob.size();
      observations[m] = std::make_shared<PursuitObservationLoc>(id, ob);
      ob.clear();
    }
    DebugString(for (int j = 0; j < size; ++j) {
      s->strings_[j].append(strings_[j] + "  ||  ACTION: " + actions[j]->toString() +
          "  | OBS: " + observations[j]->toString());
    })
    double p2 = s->prob_ / prob_;
    Outcome o(move(s), observations, rewards);
    prob.emplace_back(move(o), p2);  // pair of an outcome and its probability
  }
  return prob;
}

PursuitDomain::PursuitDomain(unsigned int max,
                             unsigned int numberOfPlayers,
                             const vector<Pos> &loc,
                             int height,
                             int width,
                             vector<double> probability) :
    Domain(max, numberOfPlayers), height(height), width(width), probability(move(probability)) {
  auto state = make_shared<PursuitState>(this, loc);
  vector<double> rewards(numberOfPlayers);
  vector<shared_ptr<Observation>> Obs;
  for (int j = 0; j < numberOfPlayers; ++j) {
    Obs.push_back(make_shared<Observation>(-1));
  }
  Outcome o(state, Obs, rewards);
  rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
}

string PursuitDomain::getInfo() const {
  return "Pursuit evasion\nDimensions: " + to_string(height) + " x " +
      to_string(width) + "\nMax depth: " +
      to_string(maxDepth) + "\nPlayers' starting location: " +
      rootStatesDistribution[0].first.state->toString();
}

PursuitDomain::PursuitDomain(unsigned int max, int height, int width) :
    PursuitDomain(max,
                  2,
                  vector<Pos>{{0, 0}, {height - 1,
                                       width - 1}},
                  height,
                  width,
                  vector<double>{0.1, 0.9}) {}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const shared_ptr<MMPursuitState> &state,
                             int height, int width, vector<double> probability) :
    Domain(max, numberOfPlayers), height(height), width(width), probability(move(probability)) {
  vector<double> rewards(numberOfPlayers);
  vector<shared_ptr<Observation>> Obs;
  for (int j = 0; j < numberOfPlayers; ++j) {
    Obs.push_back(make_shared<Observation>(-1));
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
                             int height, int width, vector<double> probability) :
    Domain(max, numberOfPlayers), height(height), width(width), probability(move(probability)) {
  vector<double> rewards(numberOfPlayers);
  vector<shared_ptr<Observation>> Obs;
  for (int j = 0; j < numberOfPlayers; ++j) {
    Obs.push_back(make_shared<Observation>(-1));
  }
  Outcome o(state, Obs, rewards);
  rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers, const vector<Pos> &loc,
                             int height, int width) : PursuitDomain(max,
                                                                    numberOfPlayers,
                                                                    loc,
                                                                    height,
                                                                    width,
                                                                    vector<double>{0.1, 0.9}) {}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const shared_ptr<MMPursuitState> &state, int height, int width) :
    PursuitDomain(max, numberOfPlayers, state, height, width,
                  vector<double>{0.1, 0.9}) {}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const shared_ptr<ObsPursuitState> &state, int height, int width) :
    PursuitDomain(max, numberOfPlayers, state,
                  height, width, vector<double>{0.1, 0.9}) {}

PursuitDomainChance::PursuitDomainChance(unsigned int max,
                                         unsigned int numberOfPlayers,
                                         const vector<Pos> &firstPlayerLocation,
                                         const vector<Pos> &secondPlayerLocation,
                                         int height, int width, vector<double> probability) :
    PursuitDomain(max, numberOfPlayers, {firstPlayerLocation[0], secondPlayerLocation[0]},
                  height, width, move(probability)) {
  rootStatesDistribution.clear();
  for (int i = 0; i < firstPlayerLocation.size(); ++i) {
    auto state = make_shared<PursuitState>(this, vector<Pos>{firstPlayerLocation[i],
                                                             secondPlayerLocation[i]});
    vector<double> rewards(numberOfPlayers);
    vector<shared_ptr<Observation>> Obs;
    for (int j = 0; j < numberOfPlayers; ++j) {
      Obs.push_back(make_shared<Observation>(-1));
    }

    Outcome o(state, Obs, rewards);
    rootStatesDistribution
        .push_back(pair<Outcome, double>(move(o), 1.0 / firstPlayerLocation.size()));
  }
}

PursuitDomainChance::PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                                         const vector<Pos> &firstPlayerLocation,
                                         const vector<Pos> &secondPlayerLocation, int height,
                                         int width) : PursuitDomainChance(max,
                                                                          numberOfPlayers,
                                                                          firstPlayerLocation,
                                                                          secondPlayerLocation,
                                                                          height,
                                                                          width,
                                                                          vector<double>{0.1,
                                                                                         0.9}) {}

}  // namespace domains
}  // namespace GTLib2
#pragma clang diagnostic pop
