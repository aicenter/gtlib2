//
// Created by rozliv on 07.08.2017.
//

#ifndef PURSUIT_BASE_H
#define PURSUIT_BASE_H

#include <iostream>
#include <vector>
#include <cmath>



class Action {
public:
    explicit Action(int id);

//    Action(int t_id, const std::string& t_s);
//
//    inline const std::string& getDesc() const{
//        return s;
//    }

    virtual std::string ToString();

    inline int getID() const{
        return id_;
    }

private:
    int id_;
};

class Observation {
public:
    explicit Observation(int id);

    Observation(int id, const std::string& s);

    inline const std::string& getDesc() const{
        return s_;
    }

    inline int getID() const{
        return id_;
    }

private:
    int id_;
    std::string s_;
};


class State {
public:
    State();

    virtual std::vector<Action> getActions (int player);

    virtual void getActions(std::vector<Action>&list ,int player) const; //TODO: problem v pristupu na getPlace stavu

  //  virtual ProbDistribution PerformAction(std::vector<Action>& actions) const;
};


class Outcome{
public:
    Outcome(const std::vector<Observation> &ob, const std::vector<double> &rew);

    inline const std::vector<Observation>& getObs() const {
        return ob_;
    }

    inline const std::vector<double>& getReward() const {
        return rew_;
    }

private:
    std::vector<Observation> ob_;
    std::vector<double> rew_;
};

class Domain {
public:
    explicit Domain(State &r);

    inline const State& getRoot() const {
        return root_;
    }

    virtual std::string GetInfo();

private:
    State root_;
};



#endif //PURSUIT_BASE_H
