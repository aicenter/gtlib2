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


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef BASE_BASE_H_
#define BASE_BASE_H_

#include <functional>
#include <utility>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include "utils/utils.h"

using std::string;
using std::vector;
using std::move;
using std::unique_ptr;
using std::shared_ptr;
using std::to_string;
using std::unordered_map;
using std::make_shared;
using std::make_unique;
using std::pair;
using std::dynamic_pointer_cast;

namespace GTLib2 {

/**
 * Action is an abstract class that represents actions,
 * which are identified by their id. Different actions have to have different ids,
 * the same actions have to have the same id.
 */
class Action {
 public:
  explicit Action(int id);

  virtual ~Action() = default;

  virtual string toString() const;

  int getId() const;

  virtual bool operator==(const Action &that) const;

  virtual size_t getHash() const;

 protected:
  int id;
};

/**
 * Observation is an abstract class that represents observations,
 * which are identified by their id. Different observations have to have different ids,
 * the same observations have to have the same id.
 */
class Observation {
 public:
  explicit Observation(int id);

  virtual ~Observation() = default;

  // Returns description
  virtual string toString() const;

  // Returns observation id
  int getId() const;

  virtual bool operator==(const Observation &rhs) const;

  virtual size_t getHash() const;

 protected:
  int id;
};

class State;
class Domain;

/**
 * Outcome is a class that represents outcomes,
 * which contain rewards for each player, observations for each player and a new state.
 */
class Outcome {
 public:
  Outcome(shared_ptr<State> s, vector<shared_ptr<Observation>> observations,
          vector<double> rewards);

  shared_ptr<State> state;
  vector<shared_ptr<Observation>> observations;
  vector<double> rewards;

  size_t getHash() const;

  bool operator==(const Outcome &rhs) const;
};

typedef pair<Outcome, double> distributionEntry;
typedef vector<distributionEntry> OutcomeDistribution;

/**
 * InformationSet is an abstract class that represents information sets.
 */
class InformationSet {
 public:
  InformationSet() = default;

  virtual bool operator==(const InformationSet &rhs) const = 0;

  // GetHash returns hash code.
  virtual size_t getHash() const = 0;

  virtual string toString() const = 0;
};

/**
 * AOH is a class that represents action-observation history,
 * which contains player id and vector of action-observation history,
 * hash code for faster comparing and has overloaded operator '=='.
 */
class AOH : public InformationSet {
 public:
  AOH(int player, const vector<pair<int, int>> &aoHistory);

  int getNumberOfActions() const;

  // GetHash returns hash code.
  inline size_t getHash() const final {
    return hashValue;
  }

  // Overloaded for comparing two AOHs
  bool operator==(const InformationSet &rhs) const override;

  inline int getPlayer() const {
    return player;
  }
  inline int getInitialObservationId() const {
    return aoh.front().second;
  }
  inline vector<pair<int, int>> getAOHistory() const {
    return aoh;
  }

  string toString() const override;

 private:
  size_t computeHash() const;
  // Vector of pairs. First coordinate is action id, the second is observation id.
  vector<pair<int, int>> aoh;
  size_t hashValue;
  int player;
};

/**
 * State is an abstract class that represents states
 */
class State {
 public:
  explicit State(Domain *domain);

  virtual ~State() = default;

  // Returns possible actions for a player in the state.
  virtual vector<shared_ptr<Action>> getAvailableActionsFor(int player) const = 0;

  // Performs actions given by player->action map
  virtual OutcomeDistribution performActions(
      const vector<pair<int, shared_ptr<Action>>> &actions) const = 0;

  // Gets players that can play in this state

  virtual vector<int> getPlayers() const = 0;

  // GetNumPlayers returns number of players who can play in this state.
  virtual int getNumberOfPlayers() const;

  // ToString returns state description
  virtual string toString() const;

  virtual bool operator==(const State &rhs) const;

  virtual size_t getHash() const;

  inline Domain *getDomain() const {
    return domain;
  }

 protected:
  Domain *domain;
};

typedef unordered_map<shared_ptr<InformationSet>,
                      unordered_map<shared_ptr<Action>, double>> BehavioralStrategy;

typedef vector<pair<shared_ptr<InformationSet>, shared_ptr<Action>>> ActionSequence;

typedef unordered_map<shared_ptr<ActionSequence>, double> RealizationPlan;

/**
* Domain is an abstract class that represents domain,
* contains a probability distribution over root states.
*/
class Domain {
 public:
  // constructor
  Domain(unsigned int maxDepth, unsigned int numberOfPlayers);

  // destructor
  virtual ~Domain() = default;

  // Returns distributions of the root states.
  const OutcomeDistribution &getRootStatesDistribution() const;

  virtual vector<int> getPlayers() const = 0;

  // Returns number of players in the game.
  inline unsigned int getNumberOfPlayers() const {
    return numberOfPlayers;
  }

  // Returns default maximal depth used in algorithms.
  inline unsigned int getMaxDepth() const {
    return maxDepth;
  }

  inline int getMaxUtility() const {
    return maxUtility;
  }

  // GetInfo returns string containing domain information.
  virtual string getInfo() const = 0;

 protected:
  OutcomeDistribution rootStatesDistribution;
  unsigned int maxDepth;
  unsigned int numberOfPlayers;
  int maxUtility;
};
}  // namespace GTLib2

namespace std {
using namespace GTLib2;

template<>
struct hash<shared_ptr<InformationSet>> {
  size_t operator()(shared_ptr<InformationSet> const &p) const {
    return p->getHash();
  }
};

template<>
struct equal_to<shared_ptr<InformationSet>> {
  bool operator()(shared_ptr<InformationSet> const &a,
                  shared_ptr<InformationSet> const &b) const {
    return *a == *b;
  }
};

template<>
struct hash<InformationSet *> {
  size_t operator()(InformationSet const *p) const {
    return p->getHash();
  }
};

template<>
struct equal_to<InformationSet *> {
  bool operator()(InformationSet *a,
                  InformationSet *b) const {
    return *a == *b;
  }
};

template<>
struct hash<shared_ptr<AOH>> {
  size_t operator()(shared_ptr<AOH> const &p) const {
    return p->getHash();
  }
};

template<>
struct equal_to<shared_ptr<AOH>> {
  bool operator()(shared_ptr<AOH> const &a,
                  shared_ptr<AOH> const &b) const {
    return *a == *b;
  }
};

template<>
struct hash<shared_ptr<Action>> {
  size_t operator()(const shared_ptr<Action> &p) const {
    return p->getHash();
  }
};

template<>
struct equal_to<shared_ptr<Action>> {
  bool operator()(const shared_ptr<Action> &a,
                  const shared_ptr<Action> &b) const {
    return *a == *b;
  }
};

template<>
struct hash<shared_ptr<ActionSequence>> {
  size_t operator()(const shared_ptr<ActionSequence> &seq) const {
    size_t seed = 0;
    for (const auto &action : (*seq)) {
      boost::hash_combine(seed, action.first);
      boost::hash_combine(seed, action.second);
    }
    return seed;
  }
};

template<>
struct equal_to<shared_ptr<ActionSequence>> {
  bool operator()(const shared_ptr<ActionSequence> &a,
                  const shared_ptr<ActionSequence> &b) const {
    hash<shared_ptr<ActionSequence>> hasher;
    if (hasher(a) != hasher(b) || (*a).size() != (*b).size()) {
      return false;
    }
    for (int i = 0; i <= (*a).size(); i++) {
      if ((*a)[i] != (*b)[i]) {
        return false;
      }
    }
    return true;
  }
};

template<>
struct hash<ActionSequence> {
  size_t operator()(const ActionSequence &seq) const {
    size_t seed = 0;
    hash<shared_ptr<InformationSet>> hasher;
    hash<shared_ptr<Action>> hasher2;
    for (const auto &action : seq) {
      boost::hash_combine(seed, hasher(action.first));
      boost::hash_combine(seed, hasher2(action.second));
    }
    return seed;
  }
};

template<>
struct equal_to<ActionSequence> {
  bool operator()(const ActionSequence &a,
                  const ActionSequence &b) const {
    hash<ActionSequence> hasher;
    if (hasher(a) != hasher(b) || a.size() != b.size()) {
      return false;
    }
    for (int i = 0; i < a.size(); i++) {
      if (!a[i].first->operator==(*b[i].first.get()) ||
          !a[i].second->operator==(*b[i].second.get())) {
        return false;
      }
    }
    return true;
  }
};

template<>
struct hash<shared_ptr<Observation>> {
  size_t operator()(const shared_ptr<Observation> &p) const {
    return p->getHash();
  }
};

template<>
struct equal_to<shared_ptr<Observation>> {
  bool operator()(const shared_ptr<Observation> &a,
                  const shared_ptr<Observation> &b) const {
    return *a == *b;
  }
};

template<>
struct hash<shared_ptr<State>> {
  size_t operator()(const shared_ptr<State> &p) const {
    return p->getHash();
  }
};

template<>
struct equal_to<shared_ptr<State>> {
  bool operator()(const shared_ptr<State> &a,
                  const shared_ptr<State> &b) const {
    return *a == *b;
  }
};

template<>
struct hash<Outcome> {
  size_t operator()(const Outcome &p) const {
    return p.getHash();
  }
};

template<>
struct equal_to<Outcome> {
  bool operator()(const Outcome &a,
                  const Outcome &b) const {
    return a == b;
  }
};
}  // namespace std


#endif  // BASE_BASE_H_

#pragma clang diagnostic pop
