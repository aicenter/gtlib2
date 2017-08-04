//
// Created by rozliv on 02.08.2017.
//
#include <iostream>
#include <vector>
#include <random>
#include <cstdlib>
#include <ctime>

#ifndef PURSUIT_H
#define PURSUIT_H

struct Pos{
    int y;
    int x;
};

class Random{
    private:
        static Random* instance;
        std::mt19937 gen;
        std::uniform_real_distribution<> dis{0, 1};

        Random();

    public:
        static Random* getInstance();
        ~Random() = default;
        double getRandom();
};

class Action {
    int id;
    std::string s;
public:
    explicit Action(int t_id);

    Action(int t_id, const std::string& t_s);

    inline const std::string& getDesc() const{
        return s;
    }

    inline int getID() const{
        return id;
    }
};

class Observation {
    int id;
    std::string s;
public:
    explicit Observation(int t_id);

    Observation(int t_id, const std::string& t_s);

    inline const std::string& getDesc() const{
        return s;
    }

    inline int getID() const{
        return id;
    }
};

class PursuitOutcome;
class Outcome;

class State {
public:

    explicit State();

    virtual void getActions(std::vector<Action>&list ,int player);
};

class PursuitState: public State {
    std::vector<Pos> place;
    std::vector<Pos> m = {{0,0}, {1,0}, {0,1}, {-1,0}, {0,-1}};
    std::vector<double> probDis = {0.1, 0.9}; //TODO: docasne
public:
    explicit PursuitState(std::vector<Pos> &p);

    PursuitOutcome PerformAction(std::vector<Action>& actions);

    void getActions(std::vector<Action>&list ,int player, int t_i, int t_j);

    void setProb(); //TODO: docasne

    inline const std::vector<Pos>& getPlace() const{
        return place;
    }
};

class Domain {
    int width;
    int height;
    State root;
public:
    explicit Domain(int w, int h, State &r);

    inline int getHeight() const{
        return height;
    }

    inline int getWidth() const{
        return  width;
    }
};

class PursuitDomain: public Domain{
    int maxdepth;
public:
    PursuitDomain(int w, int h,int max, State &r);

    int getMaxDepth() const;
};

class Outcome{
    std::vector<Observation> ob;
    std::vector<int> rew;
public:
    Outcome(const std::vector<Observation> &t_ob, const std::vector<int> &t_rew);

    inline const std::vector<Observation>& getObs() const {
        return ob;
    }

    inline const std::vector<int>& getReward() const {
        return rew;
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

class Chance: public PursuitOutcome{ //TODO: nejak to tam dostat
    std::vector<double> probDis = {0.1, 0.9};
public:
    Chance(const PursuitState &s, const std::vector<Observation> &t_ob, const std::vector<int> &t_rew);
};
#endif //PURSUIT_H