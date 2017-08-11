//
// Created by rozliv on 02.08.2017.
//


#ifndef PURSUIT_H_
#define PURSUIT_H_

#include "base.h"




class PursuitAction: public Action {
 public:
    PursuitAction(int id, int move);

    string ToString() final;

    inline int getMove() const {
        return move_;
    }
 private:
    int move_;
};

class PursuitObservation: public Observation {
 public:
    explicit PursuitObservation(int id, int move);

    string ToString() final;

    inline int getMove() const {
        return move_;
    }

 private:
    int move_;
};


class PursuitState: public State {
 public:
    PursuitState();
    explicit PursuitState(const vector<Pos> &p);

    PursuitState(const vector<Pos> &p, double prob);

    vector<shared_ptr<Action>> getActions(int player) final;

    void getActions(vector<shared_ptr<Action>>& list, int player) const final;

    ProbDistribution PerformAction(vector<shared_ptr<Action>> actions) final;

    const vector<Pos>& getPlace() const final {
        return place_;
    }

    const vector<double>& getProb() const {
        return probdis_;
    }

    inline double getPro() const {
        return prob_;
    }

 private:
    vector<Pos> place_;
    double prob_ = 1;
    vector<Pos> eight = {{-2, -2}, {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                         {0, 1}, {1, -1}, {1, 0}, {1, 1}, {0, 0}};
    vector<string> desc = {"nowhere", "top left", "top", "top right", "left",
                           "right", "bottom left", "bottom", "bottom right", "same"};
    vector<Pos> m_ = {{0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    vector<string> des_ = {"stay", "right", "down", "left", "up"};
    vector<double> probdis_ = {0.1, 0.9};  // TODO(rozlijak): docasne
};


class PursuitDomain: public Domain{
 public:
    PursuitDomain(int max, const unique_ptr<State> &r, int maxPlayers);

    int getMaxDepth() const {
        return maxdepth_;
    }

    string GetInfo() final;

    static int height_;
    static int width_;

 private:
    int maxdepth_;
};


extern int count;
extern vector<double> rewards;

void pursuit(const PursuitDomain& domain, const unique_ptr<State> &state, int depth);

#endif  // PURSUIT_H_
