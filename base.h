//
// Created by rozliv on 07.08.2017.
//

#ifndef BASE_H_
#define BASE_H_

#include <functional>
#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <ctime>
#include <string>
#include <unordered_map>
#include "utils/utils.h"

using std::string;
using std::cout;
using std::vector;
using std::move;
using std::unique_ptr;
using std::shared_ptr;
using std::to_string;
using std::unordered_map;
using std::make_shared;
using std::pair;



/**
 * Action is an abstract class that represents actions,
 * which are identified by their id.
 */
class Action {
 public:
  // constructor
  explicit Action(int id);

  // destructor
  virtual ~Action() = default;

  // Returns description
  inline virtual string ToString() {
    if (id_ == -1)
      return "NoA";
    return to_string(id_);
  }

  // GetID returns action id
  inline int GetID() const {
    return id_;
  }

 protected:
  int id_;
};

static Action NoA(-1);  // No Action

/**
 * Observation is an abstract class that represents observations,
 * which are identified by their id.
 */
class Observation {
 public:
  // constructor
  explicit Observation(int id);

  // destructor
  virtual ~Observation() = default;

  // Returns description
  inline virtual string ToString() {
    if (id_ == -1) {
      return "NoOb;";
    }
    return to_string(id_);
  }

  // GetID returns observation id
  inline int GetID() const {
    return id_;
  }

 protected:
  int id_;
};

static Observation NoOb(-1);  // No Observation

struct Pos{
  int y;
  int x;
};

class State;  // Forward declaration

/**
 * Outcome is a class that represents outcomes,
 * which contain rewards, observations and a new state.
 */
class Outcome{
 public:
  Outcome(shared_ptr<State> s, vector<shared_ptr<Observation>> ob,
          vector<double> rew);

  // GetState returns a new state
  inline const shared_ptr<State>& GetState() const {
    return st_;
  }

  // GetObs returns vector of observations
  inline const vector<shared_ptr<Observation>>& GetObs() {
    return ob_;
  }

  // GetReward returns vector of rewards for each player.
  inline const vector<double>& GetReward() const {
    return rew_;
  }

 private:
  shared_ptr<State> st_;
  vector<shared_ptr<Observation>> ob_;
  vector<double> rew_;
};

/**
 * ProbDistribution is a class that represent
 * pairs of outcomes and their probability.
 */
class ProbDistribution {
 public:
  // constructor
  explicit ProbDistribution(vector<pair<Outcome, double>> pairs);

  // GetOutcomes returns a vector of all outcomes.
  vector<Outcome> GetOutcomes();

  // GetProb returns vector of probabilities over outcomes.
  vector<double> GetProb();

 private:
  vector<pair<Outcome, double>> pairs_;
};


/**
 * InfSet is an abstract class that represent information sets,
 *  which are identified by their hash code. Contains two structs
 *  used for hashing this class.
 */
class InfSet {
 public:
  InfSet() = default;

  virtual bool operator==(InfSet& other) = 0;

  // GetHash returns hash code.
  virtual size_t GetHash() = 0;

  // Hash is struct that is used as hash function for InfSet shared pointer.
  struct Hash {
    size_t operator() (shared_ptr<InfSet> const &p) const {
      return p->GetHash();
    }
  };

  // Compare is used as compare function for InfSet shared pointers.
  struct Compare {
    bool operator() (shared_ptr<InfSet> const &a,
                     shared_ptr<InfSet> const &b) const {
      return *a == *b;
    }
  };
};


/**
 * AOH is a class that represents action-observation history,
 * which contains player id and vector of action-observation history,
 * hash code for faster comparing and has overloaded operator '=='.
 */
class AOH: public InfSet {
 public:
  // constructor
  AOH(int player, const vector<int>& hist);

  // GetHash returns hash code.
  size_t GetHash() final;

  // Overloaded for comparing two AOHs
  bool operator==(InfSet& other2) override {
    auto * other = dynamic_cast<AOH*>(&other2);
    if (other != nullptr) {
      return player_ == other->player_ && GetHash() == other->GetHash() &&
             aohistory_ == other->aohistory_;
    }
    return false;
  }
 private:
  int player_;
  vector<int> aohistory_;
  size_t seed_ = 0;
};



/**
 * Strategy is a class that represents a player's (behavioral) strategy.
 * Contains unordered_map with shared pointer to InfSet used as key and
 * probability distribution over actions as values. Prob. distribution is
 * represented by vector of pairs of double (0-1) and shared pointer to Action.
 */
class Strategy {
 public:
  // constructor
  Strategy() {
    purestrategies = unordered_map<shared_ptr<InfSet>,
        vector<pair<double, shared_ptr<Action>>>,
        InfSet::Hash, InfSet::Compare>();
  }

  // inserts new element into unordered_map
  inline void Add(const shared_ptr<InfSet>& key,
                  const vector<pair<double, shared_ptr<Action>>>& value) {
    purestrategies.emplace(key, value);
  }

  // returns element with  key equivalent to key
  inline const vector<pair<double, shared_ptr<Action>>>& Find(const shared_ptr<InfSet>& key) const {
    return purestrategies.find(key)->second;  // TODO(rozlijak): osetrit false
  }

  // returns all strategies
  inline virtual const unordered_map<shared_ptr<InfSet>,
      vector<pair<double, shared_ptr<Action>>>,
      InfSet::Hash, InfSet::Compare>& GetStrategies() {
    return purestrategies;
  }

 protected:
  unordered_map<shared_ptr<InfSet>, vector<pair<double, shared_ptr<Action>>>,
      InfSet::Hash, InfSet::Compare> purestrategies;
};


/**
 * PureStrategy is a class that represents a player's pure strategy.
 * Contains unordered_map with shared pointer to InfSet used as key and
 * probability distribution over actions as values. Prob. distribution is
 * represented by a pair of double (1) and shared pointer to Action.
 */
class PureStrategy: public Strategy {
 public:
  // constructor
  PureStrategy() : Strategy() {}

  // inserts new element into unordered_map
  inline void Add(const shared_ptr<InfSet> &key,
                  const shared_ptr<Action> &value) {
    purestrategies.emplace(key, vector<pair<double, shared_ptr<Action>>>
        {std::make_pair(1, value)});
  }

  // returns all pure strategies
  inline const unordered_map<shared_ptr<InfSet>,
      vector<pair<double, shared_ptr<Action>>>,
      InfSet::Hash, InfSet::Compare>& GetStrategies() override {
    return purestrategies;
  }

  // returns shared pointer to Action with key equivalent to key or returns NoA
  inline const shared_ptr<Action>& Find(const shared_ptr<InfSet>& key)const {
    auto search = purestrategies.find(key);
    if (search != purestrategies.end()) {
      return search->second[0].second;
    }
    return noa;
  }

 private:
  shared_ptr<Action> noa = make_shared<Action>(NoA);  // TODO(rozlijak): temp
};


/**
 * State is an abstract class that represent states
 */
class State {
 public:
  // constructor
  State();

  // destructor
  virtual ~State() = default;

  // GetActions returns possible actions for a player in the state.
  virtual vector<shared_ptr<Action>> GetActions(int player) = 0;

  // GetActions returns possible actions for a player in the state.
  virtual void GetActions(vector<shared_ptr<Action>>& list, int player) const = 0;

  // PerformAction performs actions for all players who can play in the state.
  virtual ProbDistribution PerformAction(const vector<shared_ptr<Action>> &actions) = 0;

  // GetNumPlayers returns number of players who can play in this state.
  virtual const int GetNumPlayers() const = 0;

  // GetPlayers returns who can play in this state.
  virtual const vector<bool> GetPlayers() const = 0;

  // AddString adds string s to a string in vector of strings.
  virtual void AddString(const string &s, int player) = 0;

  // ToString returns state description
  virtual string ToString(int player) = 0;
};


/**
 * Domain is an abstract class that represent domain,
 * contains a probability distribution over root states.
 */
class Domain {
 public:
  // constructor
  Domain(unsigned int max, unsigned int maxplayers);

  // destructor
  virtual ~Domain() = default;

  // GetRoot returns ProbDistribution over first state.
  inline const shared_ptr<ProbDistribution>& GetRoot() {
    return root_;
  }

  // GetMaxPlayers returns max players in a game.
  inline unsigned int GetMaxPlayers() const {
    return maxplayers_;
  }

  // GetMaxDepth returns maximal depth of algorithm.
  inline unsigned int GetMaxDepth() const {
    return maxdepth_;
  }

  // GetInfo returns string containing domain information.
  virtual string GetInfo() = 0;

  static int depth_;

  // Start function to calculate an expected value for a strategy profile
  virtual double CalculateUtility(const vector<PureStrategy>&pure_strategies);

  // Calculate an expected value for a strategy profile
  virtual double ComputeUtility(State* state, unsigned int depth,
                                unsigned int players,
                                const vector<PureStrategy>& pure_strategies,
                                const vector<vector<int>>& aoh);

 protected:
  unsigned int maxdepth_;
  unsigned int maxplayers_;
  shared_ptr<ProbDistribution> root_;
};






// Domain independent treewalk algorithm
void Treewalk(shared_ptr<Domain> domain, State *state,
              unsigned int depth, int players,
              std::function<void(State*)> FunctionForState);


void Treewalk(shared_ptr<Domain> domain, State *state,
              unsigned int depth, int players);

// Start method for domain independent treewalk algorithm
void TreewalkStart(const shared_ptr<Domain>& domain, unsigned int depth = 0);

#endif  // BASE_H_
