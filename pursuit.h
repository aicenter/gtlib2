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


class PursuitOutcome;

class PursuitState: public State {
    std::vector<Pos> place;
    std::vector<Pos> m = {{0,0}, {1,0}, {0,1}, {-1,0}, {0,-1}};
    std::vector<std::string> des = {"stay", "right", "down", "left", "up"};
    std::vector<double> probDis = {0.1, 0.9}; //TODO: docasne
public:
    explicit PursuitState(std::vector<Pos> &p);

    PursuitOutcome PerformAction(std::vector<Action>& actions)const;

    void getActions(std::vector<Action>&list ,int player, int t_i, int t_j)const;

    void setProb(); //TODO: docasne

    inline const std::vector<Pos>& getPlace() const{
        return place;
    }
};

class PursuitOutcome: public Outcome{
    PursuitState st;
public:
    PursuitOutcome(const PursuitState &s, const std::vector<Observation> &t_ob, const std::vector<int> &t_rew);

    inline const PursuitState & getState() const {
        return st;
    }
};


class PursuitDomain: public Domain{
    int maxdepth;
public:
    PursuitDomain(int w, int h,int max, State &r);

    int getMaxDepth() const;
};

extern int count;
extern std::vector<int> rewards;

void pursuit(PursuitDomain& domain, const PursuitState &state, int depth);

#endif //PURSUIT_PURSUIT_H