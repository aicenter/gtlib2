//
// Created by rozliv on 07.08.2017.
//

#ifndef PURSUIT_BASE_H
#define PURSUIT_BASE_H

#include <memory>
#include <iostream>
#include <vector>
#include <cmath>


struct Pos{ //TODO: To tady byt nemuze
    int y;
    int x;
};


class Action {
public:
    explicit Action(int id);

//    Action(int t_id, const std::string& t_s);
//
//    inline const std::string& getDesc() const{
//        return s;
//    }
    virtual int getInfo(){return 0; }

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


class ProbDistribution;

class State {
public:
    explicit State();

    State(const State&) = delete;

    State& operator=(const State&) = delete;

    virtual ~State() = default;

    virtual std::vector<Action> getActions (int player);

    virtual void getActions(std::vector<Action>&list ,int player) const; //TODO: problem v pristupu na getPlace stavu

    virtual ProbDistribution PerformAction(std::vector<Action>& actions) const;// final;

    virtual const std::vector<Pos>& getPlace() const = 0;

    virtual const std::vector<double>& getProb() const = 0;


    virtual double getPro() const = 0;

};

class Outcome{
public:
    explicit Outcome() = default;

    //Outcome(const Outcome&) = delete;

  //  Outcome& operator=(const Outcome&) = delete;

    virtual ~Outcome() = default;
     //Outcome(const std::vector<Observation> &ob, const std::vector<double> &rew);
    Outcome(std::unique_ptr<State>& st, const std::vector<Observation> &ob, const std::vector<double> &rew);
    inline const std::vector<Observation>& getObs() const {
        return ob_;
    }

    inline const std::vector<double>& getReward() const {
        return rew_;
    }

    inline const std::unique_ptr<State> & getState() const {
        return st_;
    }

private:
    std::vector<Observation> ob_;
    std::vector<double> rew_;
    std::unique_ptr<State> st_;
};

class Domain {
public:
    explicit Domain(std::unique_ptr<State>& r);

    inline const std::unique_ptr<State>& getRoot() const {
        return root_;
    }

    virtual std::string GetInfo();

private:
    std::unique_ptr<State> root_;
};

class ProbDistribution {
public:
    explicit ProbDistribution(const std::vector<std::pair<Outcome&,double>>& pairs);

    const Outcome& GetRandom();

    std::vector<Outcome> GetOutcomes();

    std::vector<std::pair<Outcome&, double>> pairs_;
};

#endif //PURSUIT_BASE_H
