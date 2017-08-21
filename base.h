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



/**
 * Action is an abstract class that represents actions,
 * which are identified by their id.  */
class Action {
 public:
  // constructor
  explicit Action(int id);

  // destructor
  virtual ~Action() = default;

  virtual string ToString();

  // GetID returns action id
  inline int GetID() const {
    return id_;
  }

 protected:
  int id_;
};

/**
 * Observation is an abstract class that represents observations,
 * which are identified by their id.  */
class Observation {
 public:
  // constructor
  explicit Observation(int id);

  // destructor
  virtual ~Observation() = default;

  virtual string ToString();

  // GetID returns observation id
  inline int GetID() const {
    return id_;
  }

 protected:
  int id_;
};

struct Pos{
  int y;
  int x;
};

class State;  // Forward declaration

/**
 * Outcome is a class that represents outcomes,
 * which contain rewards, observations and new state.  */
class Outcome{
 public:
  Outcome(unique_ptr<State> s, vector<unique_ptr<Observation>> ob,
          const vector<double> &rew);

  inline unique_ptr<State> GetState() {
    return move(st_);
  }

  inline vector<unique_ptr<Observation>> GetObs() {
    return move(ob_);
  }

  inline const vector<double>& GetReward() const {
    return rew_;
  }

 private:
  vector<unique_ptr<Observation>> ob_;
  vector<double> rew_;
  unique_ptr<State> st_;
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
 * InfSet is a class that represent information sets,
 *  which are identified by their hash code.  */
class InfSet {  // TODO(rozlijak): make it abstract
 public:
  explicit InfSet(const vector<int>& hist):aohistory_(hist) {}
  inline std::size_t GetIS()const {
      std::size_t seed = aohistory_.size();
      for (int i : aohistory_) {
        seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
  }

 private:
  const vector<int>& aohistory_;
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

  // GetPlayers returns number of players who can play in this state.
  virtual const int GetPlayers() const = 0;

  // SetPlayers sets number of players who can play in this state.
  virtual void SetPlayers(unsigned int number) = 0;

  // GetAOH returns action-observation histories of all players.
  virtual const vector<vector<int>>& GetAOH() const = 0;

  // SetAOH sets action-observation histories of all players.
  virtual void SetAOH(const vector<vector<int>>& list) = 0;

 protected:
  unsigned int numplayers_ = 0;
};


/**
 * Domain is an abstract class that represent domain,
 * contains a root state.  */
class Domain {
 public:
  // constructor
  Domain(int maxplayers, int max);

  // destructor
  virtual ~Domain() = default;

  const shared_ptr<State> &GetRoot() {
    return root_;
  }

  // GetMaxPlayers returns max players in a game.
  inline int GetMaxPlayers() const {
    return maxplayers_;
  }

  // GetMaxDepth returns maximal depth of algorithm
  int GetMaxDepth() const {
    return maxdepth_;
  }

  // TODO(rozlijak): virtual vector<Strategy*> ComputeUtility() = 0;

  // GetInfo returns string containing domain information.
  virtual string GetInfo() = 0;

  static int depth_;

 protected:
  int maxplayers_;
  int maxdepth_;
  shared_ptr<State> root_;
};

// Domain independent treewalk algorithm
void Treewalk(const unique_ptr<Domain>& domain, State *state,
              int depth, int players);


#endif  // BASE_H_
