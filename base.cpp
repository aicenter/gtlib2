//
// Created by rozliv on 07.08.2017.
//

#include "base.h"

Action::Action(int id): id_(id) {}

std::string Action::ToString() {
    return std::string();
}

Observation::Observation(int id): id_(id){}

std::string Observation::ToString() {
    return std::string();
}

State::State() = default;

Domain::Domain(const std::unique_ptr<State> &r):root_(r) {}

std::string Domain::GetInfo() {}

Outcome::Outcome(std::unique_ptr<State> s, std::vector<std::unique_ptr<Observation>> ob, const std::vector<double> &rew):
        st_(std::move(s)), ob_(std::move(ob)), rew_(rew){}

ProbDistribution::ProbDistribution(std::vector<std::pair<Outcome,double>> pairs): pairs_(std::move(pairs)){}

//Outcome ProbDistribution::GetRandom(){ //TODO
//    return std::move(pairs_[3].first);
//}

std::vector<Outcome>  ProbDistribution::GetOutcomes() {
    std::vector<Outcome> list;// = std::vector<PursuitOutcome>(pairs_.size());
    for (auto &pair : pairs_) {
        Outcome o (pair.first.getState(), pair.first.getObs(), pair.first.getReward());
        list.push_back(std::move(o));
    }
    return list;
}