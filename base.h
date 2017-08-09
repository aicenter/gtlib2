//
// Created by rozliv on 07.08.2017.
//

#ifndef PURSUIT_BASE_H
#define PURSUIT_BASE_H

#include <iostream>
#include <vector>
#include <cmath>
#include <memory>


class Action {
public:
    explicit Action(int id);

//    Action(int t_id, const std::string& t_s);
//
//    inline const std::string& getDesc() const{
//        return s;
//    }

    virtual std::string ToString();

    virtual int getInfo(){return 0; }

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

struct Pos{
    int y;
    int x;
};

class State;

class Outcome{
public:
    //Outcome(const std::vector<Observation> &ob, const std::vector<double> &rew);
    Outcome(const std::shared_ptr<State> &s, const std::vector<Observation> &ob, const std::vector<double> &rew);

    inline std::shared_ptr<State> getState() {
        return st_;
    }

    inline const std::vector<Observation>& getObs() const {
        return ob_;
    }

    inline const std::vector<double>& getReward() const {
        return rew_;
    }

private:
    std::vector<Observation> ob_;
    std::vector<double> rew_;
    std::shared_ptr<State> st_;
};


class ProbDistribution {
public:
    explicit ProbDistribution(const std::vector<std::pair<Outcome,double>>& pairs);
    Outcome GetRandom();

    std::vector<Outcome> GetOutcomes();

private:
    std::vector<std::pair<Outcome, double>> pairs_;
};


class State {
public:
    State();

    virtual std::vector<Action> getActions (int player) = 0;

    virtual void getActions(std::vector<Action>&list ,int player) const = 0; //TODO: problem v pristupu na getPlace stavu

    virtual ProbDistribution PerformAction(std::vector<Action>& actions) const = 0;

    virtual const std::vector<Pos>& getPlace() const = 0;

    virtual const std::vector<double>& getProb() const = 0;

    virtual double getPro() const = 0;
};


class Domain {
public:
    explicit Domain(const std::shared_ptr<State> &r);

    inline std::shared_ptr<State> getRoot(){
        return root_;
    }

    virtual int getMaxDepth() const = 0;

    virtual std::string GetInfo();

private:
    std::shared_ptr<State> root_;
};





#endif //PURSUIT_BASE_H
