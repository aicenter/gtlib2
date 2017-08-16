//
// Created by rozliv on 07.08.2017.
//

#ifndef BASE_H_
#define BASE_H_

#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <ctime>
#include <string>
#include "utils.h"

using std::string;
using std::cout;
using std::vector;
using std::move;
using std::unique_ptr;
using std::shared_ptr;
using std::to_string;



class Action {
 public:
  explicit Action(int id);

  virtual ~Action() = default;

  virtual string ToString();

  inline int GetID() const {
    return id_;
  }

 protected:
  int id_;
};


class Observation {
 public:
  explicit Observation(int id);

  virtual ~Observation() = default;

  virtual string ToString();

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

class State;

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


class ProbDistribution {
 public:
  explicit ProbDistribution(vector<std::pair<Outcome, double>> pairs);

  Outcome GetRandom();

  vector<Outcome> GetOutcomes();

 private:
  vector<std::pair<Outcome, double>> pairs_;
};


class State {
 public:
  State();

  virtual ~State() = default;

  virtual vector<shared_ptr<Action>> GetActions(int player) = 0;

  virtual void GetActions(vector<shared_ptr<Action>>& list, int player) const = 0;

  virtual ProbDistribution PerformAction(const vector<shared_ptr<Action>> &actions) = 0;

  virtual const vector<Pos>& GetPlace() const = 0;
};


class Domain {
 public:
  Domain(int maxplayers, int max);

  virtual ~Domain() = default;

  const unique_ptr<State> &GetRoot() {
    return root_;
  }

  inline int GetMaxPlayers() const {
    return maxplayers_;
  }

  int GetMaxDepth() const {
    return maxdepth_;
  }

  virtual string GetInfo();

  static int depth_;

 protected:
  int maxplayers_;
  int maxdepth_;
  unique_ptr<State> root_;
};

void Treewalk(const unique_ptr<Domain>& domain, const unique_ptr<State> &state,
              int depth, int players);



#endif  // BASE_H_
