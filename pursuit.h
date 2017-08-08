//
// Created by rozliv on 02.08.2017.
//


#ifndef PURSUIT_PURSUIT_H
#define PURSUIT_PURSUIT_H

#include "base.h"

using std::string;
using std::cout;
using std::vector;


struct Pos{
    int y;
    int x;
};

class PursuitAction {
public:
    explicit PursuitAction(int id);

//    Action(int t_id, const std::string& t_s);
//
//    inline const std::string& getDesc() const{
//        return s;
//    }

    virtual std::string ToString() final;

private:
};



class PursuitOutcome;

class PursuitState: public State {
public:
    explicit PursuitState(std::vector<Pos> &p);

    PursuitState(std::vector<Pos> &p, double prob);

    std::vector<Action> getActions (int player) final;

    void getActions(std::vector<Action>&list ,int player) const final;

    PursuitOutcome PerformAction(std::vector<Action>& actions) const;

    inline const std::vector<Pos>& getPlace() const{
        return place_;
    }

    inline const std::vector<double>& getProb() const{
        return probdis_;
    }

    inline double getPro() const{
        return prob_;
    }

private:
    double prob_ = 1;
    std::vector<Pos> place_;
    std::vector<Pos> m_ = {{0,0}, {1,0}, {0,1}, {-1,0}, {0,-1}};
    std::vector<std::string> des_ = {"stay", "right", "down", "left", "up"};
    std::vector<double> probdis_ = {0.1, 0.9}; //TODO: docasne
};

class PursuitOutcome: public Outcome{
public:
    PursuitOutcome(const PursuitState &s, const std::vector<Observation> &ob, const std::vector<double> &rew);

    inline const PursuitState & getState() const {
        return st_;
    }

private:
    PursuitState st_;
};


class PursuitDomain: public Domain{
public:
    PursuitDomain(int max, State &r);

    inline int getMaxDepth() {
        return maxdepth_;
    }

    std::string GetInfo() final;

    static int height_;
    static int width_;

private:
    int maxdepth_;
};

extern int count;
extern std::vector<double> rewards;

void pursuit(PursuitDomain& domain, const PursuitState &state, int depth);

#endif //PURSUIT_PURSUIT_H