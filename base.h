//
// Created by rozliv on 07.08.2017.
//

#ifndef PURSUIT_BASE_H
#define PURSUIT_BASE_H

#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <ctime>


template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Action {
public:
    explicit Action(int id);

    virtual std::string ToString();

    inline int getID() const{
        return id_;
    }

    virtual int getMove() const = 0;

protected:
    int id_;
};

class Observation {
public:
    explicit Observation(int id);

   // Observation(Observation const &) = delete;
   // Observation &operator=(Observation const &) = delete;

    virtual std::string ToString();

    inline int getID() const{
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
    //Outcome(const std::vector<Observation> &ob, const std::vector<double> &rew);
    Outcome(std::unique_ptr<State> s, std::vector<std::unique_ptr<Observation>> ob, const std::vector<double> &rew);

//    Outcome(const Outcome& foo) :
//    st_(std::move(foo.st_)), ob_(std::move(foo.ob_)), rew_(std::move(foo.rew_)) {
//
//    };

    //Outcome &operator=(Outcome const &) = delete;

    inline std::unique_ptr<State> getState() {
        return std::move(st_);
    }

    inline std::vector<std::unique_ptr<Observation>> getObs() {
        return std::move(ob_);
    }

    inline const std::vector<double>& getReward() const {
        return rew_;
    }

private:
    std::vector<std::unique_ptr<Observation>> ob_;
    std::vector<double> rew_;
    std::unique_ptr<State> st_;
};


class ProbDistribution {
public:
    explicit ProbDistribution(std::vector<std::pair<Outcome,double>> pairs);

    Outcome GetRandom();

    std::vector<Outcome> GetOutcomes();

private:
    std::vector<std::pair<Outcome, double>> pairs_;
};


class State {
public:
    State();

    virtual std::vector<std::shared_ptr<Action>> getActions (int player) = 0;

    virtual void getActions(std::vector<std::shared_ptr<Action>>&list ,int player) const = 0; //TODO: problem v pristupu na getPlace stavu

    virtual ProbDistribution PerformAction(std::vector<std::shared_ptr<Action>> actions) = 0;

    virtual const std::vector<Pos>& getPlace() const = 0;
};


class Domain {
public:
    explicit Domain(const std::unique_ptr<State> &r);

    inline const std::unique_ptr<State> &getRoot()const {
        return root_;
    }

    virtual int getMaxDepth() const = 0;

    virtual std::string GetInfo();

private:
    const std::unique_ptr<State> &root_;
};


#endif //PURSUIT_BASE_H