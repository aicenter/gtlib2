//
// Created by rozliv on 07.08.2017.
//

#ifndef BASE_H_
#define BASE_H_

#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <utility>
#include <ctime>
#include <string>
#include "utils/utils.h"

using std::string;
using std::cout;
using std::vector;
using std::move;
using std::unique_ptr;
using std::shared_ptr;
using std::to_string;
using std::make_shared;



/**
 * Action is an abstract class that represents actions,
 * which are identified by their id.  */
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
 * which are identified by their id.  */
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
 * which contain rewards, observations and a new state.  */
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
 * pairs of outcomes and their probability.  */
class ProbDistribution {
 public:
  // constructor
  explicit ProbDistribution(vector<std::pair<Outcome, double>> pairs);

  // GetRandom returns a random Outcome from vector.
  Outcome GetRandom();

  // GetOutcomes returns a vector of all outcomes.
  vector<Outcome> GetOutcomes();

 private:
  vector<std::pair<Outcome, double>> pairs_;
};


/**
 * InfSet is an abstract class that represent information sets,
 *  which are identified by their hash code.  */
class InfSet {
 public:
  InfSet() = default;

  virtual bool operator==(InfSet& other) = 0;

  // GetHash returns hash code.
  virtual size_t GetHash() = 0;
};


/**
 * AOH is a class that represents action-observation history,
 * which contains player id and vector of action-observation history,
 * hash code for faster comparing and has overloaded operator '=='  */
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
 * State is an abstract class that represent states  */
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

  // GetAOH returns action-observation histories of all players.
  virtual const vector<vector<int>>& GetAOH() const = 0;

  // SetAOH sets action-observation histories of all players.
  virtual void SetAOH(const vector<vector<int>>& list) = 0;

  // AddString adds string s to a string in vector of strings.
  virtual void AddString(const string &s, int player) = 0;

  // GetString returns a string from vector.
  virtual const string& GetString(int player) const = 0;

  // ToString returns state description
  virtual string ToString(int player) = 0;
};


/**
 * Domain is an abstract class that represent domain,
 * contains a probability distribution over root states.  */
class Domain {
 public:
  // constructor
  Domain(unsigned int max, unsigned int maxplayers);

  // destructor
  virtual ~Domain() = default;

  // GetRoot returns ProbDistribution over first state.
  const shared_ptr<ProbDistribution>& GetRoot() {
    return root_;
  }

  // GetMaxPlayers returns max players in a game.
  inline unsigned int GetMaxPlayers() const {
    return maxplayers_;
  }

  // GetMaxDepth returns maximal depth of algorithm.
  unsigned int GetMaxDepth() const {
    return maxdepth_;
  }

  // GetInfo returns string containing domain information.
  virtual string GetInfo() = 0;

  static int depth_;

 protected:
  unsigned int maxdepth_;
  unsigned int maxplayers_;
  shared_ptr<ProbDistribution> root_;
};

// Domain independent treewalk algorithm
void Treewalk(const unique_ptr<Domain>& domain, State *state,
              int depth, int players);

// Start method for domain independent treewalk algorithm
void TreewalkStart(const unique_ptr<Domain>& domain, int depth = 0);

#endif  // BASE_H_
