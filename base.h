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

class Action {
 public:
    explicit Action(int id);

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

    virtual ProbDistribution PerformAction(vector<shared_ptr<Action>> actions) = 0;

    virtual const vector<Pos>& getPlace() const = 0;
};


class Domain {
 public:
    explicit Domain(const unique_ptr<State> &r, int maxPlayers);

    inline const unique_ptr<State> &getRoot()const {
        return root_;
    }

    inline int getMaxPlayers() const {
        // TODO(rozlijak): pro kolik to je nebo kolik jich muze
        return maxPlayers_;
    }

    virtual string GetInfo();

 private:
    const unique_ptr<State> &root_;
    int maxPlayers_;
};
#endif  // BASE_H_
