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

using std::string;
using std::cout;
using std::vector;
using std::move;
using std::unique_ptr;
using std::shared_ptr;
using std::to_string;

template<typename T, typename ...Args>
unique_ptr<T> make_unique(Args&&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename U>
vector<shared_ptr<U>> Cast(const vector<shared_ptr<T>>& actions2) {
  vector<shared_ptr<U>> actions = vector<shared_ptr<U>>();
  for (const auto &j : actions2) {
    actions.push_back(std::dynamic_pointer_cast<U>(j));
  }
  return actions;
}



class Action {
 public:
  explicit Action(int id);

  virtual ~Action() = default;

  virtual string ToString();

  inline int getID() const {
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

  inline int getID() const {
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

  inline unique_ptr<State> getState() {
    return move(st_);
  }

  inline vector<unique_ptr<Observation>> getObs() {
    return move(ob_);
  }

  inline const vector<double>& getReward() const {
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

  virtual vector<shared_ptr<Action>> getActions(int player) = 0;

  virtual void getActions(vector<shared_ptr<Action>>& list, int player) const = 0;

  virtual ProbDistribution PerformAction(const vector<shared_ptr<Action>> &actions) = 0;

  virtual const vector<Pos>& getPlace() const = 0;
};


class Domain {
 public:
  Domain(const unique_ptr<State> &r, int maxPlayers);

  virtual ~Domain() = default;

  inline const unique_ptr<State> &getRoot()const {
    return root_;
  }

  inline int getMaxPlayers() const {
    // TODO(rozlijak): pro kolik to je nebo kolik jich muze
    return maxPlayers_;
  }

  virtual int getMaxDepth() const = 0;

  virtual string GetInfo();

 private:
  const unique_ptr<State> &root_;
  int maxPlayers_;
};

void Treewalk(const unique_ptr<Domain>& domain, const unique_ptr<State> &state, int depth);

#endif  // BASE_H_
