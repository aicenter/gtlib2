//
// Created by rozliv on 07.08.2017.
//

#include "base.h"

Action::Action(int id): id_(id) {}

std::string Action::ToString() {
    return std::string();
}

//Action::Action(int t_id,  const std::string& t_s): id(t_id), s(t_s){}


Observation::Observation(int id): id_(id){}

Observation::Observation(int id,  const std::string& s):id_(id), s_(s){}


State::State() = default;

std::vector<Action> State::getActions(int player) {
    return std::vector<Action>();
};

void State::getActions(std::vector<Action> &list, int player) const {}

//ProbDistribution State::PerformAction(std::vector<Action> &actions) const {
//    return ProbDistribution(std::vector<DOut>());
//}


Outcome::Outcome( const std::vector<Observation> &ob, const std::vector<double> &rew):  ob_(ob), rew_(rew){}


Domain::Domain(State &r): root_(r) {}

std::string Domain::GetInfo() {}

