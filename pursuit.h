//
// Created by rozliv on 02.08.2017.
//


#ifndef PURSUIT_PURSUIT_H
#define PURSUIT_PURSUIT_H

#include "base.h"

using std::string;
using std::cout;
using std::vector;


class PursuitAction: public Action {
public:
    explicit PursuitAction(int id): Action(id){move_ = 0;}

    PursuitAction(int id, int move);

    std::string ToString() final;

    inline int getMove() const {
        return move_;
    }
private:
    int move_;
};

class PursuitObservation: public Observation {
public:
    explicit PursuitObservation(int id, int move);

    std::string ToString() final;

    inline int getMove() const{
        return move_;
    }

private:
    int move_;
};


class PursuitState: public State {
public:
    PursuitState();
    explicit PursuitState(std::vector<Pos> &p);

    PursuitState(std::vector<Pos> &p, double prob);

    std::vector<std::shared_ptr<Action>> getActions (int player) final;

    void getActions(std::vector<std::shared_ptr<Action>>&list ,int player) const final;

    ProbDistribution PerformAction(std::vector<std::shared_ptr<Action>> actions)  final;// final; TODO: problem - musi byt v base

    const std::vector<Pos>& getPlace() const  {
        return place_;
    }

    const std::vector<double>& getProb() const {
        return probdis_;
    }

    inline double getPro() const {
        return prob_;
    }

private:
    std::vector<Pos> place_;
    double prob_ = 1;
    std::vector<Pos> eight = {{-2,-2},{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}, {0,0}};
    std::vector<std::string> desc = {"nowhere","top left", "top", "top right", "left", "right",
                                     "bottom left", "bottom", "bottom right", "same"};
    std::vector<Pos> m_ = {{0,0}, {1,0}, {0,1}, {-1,0}, {0,-1}};
    std::vector<std::string> des_ = {"stay", "right", "down", "left", "up"};
    std::vector<double> probdis_ = {0.1, 0.9}; //TODO: docasne
};


class PursuitDomain: public Domain{
public:
    PursuitDomain(int max,const std::unique_ptr<State> &r);

    int getMaxDepth() const final {
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

void pursuit(const Domain& domain,const std::unique_ptr<State> &state, int depth);

#endif //PURSUIT_PURSUIT_H