//
// Created by rozliv on 07.08.2017.
//

#include "base.h"

Action::Action(int id): id_(id) {}

std::string Action::ToString() {
    return std::string();
}

Observation::Observation(int id): id_(id){}

Observation::Observation(int id,  const std::string& s):id_(id), s_(s){}

State::State() = default;

Domain::Domain(const std::shared_ptr<State> &r):root_(r) {}

std::string Domain::GetInfo() {}

Outcome::Outcome(const std::shared_ptr<State> &s, const std::vector<Observation> &ob, const std::vector<double> &rew):st_(s), ob_(ob), rew_(rew){}

ProbDistribution::ProbDistribution(const std::vector<std::pair<Outcome,double>>& pairs): pairs_(pairs){}

Outcome ProbDistribution::GetRandom(){ //TODO
    return pairs_[3].first;
}

std::vector<Outcome>  ProbDistribution::GetOutcomes() {
    std::vector<Outcome> list;// = std::vector<PursuitOutcome>(pairs_.size());
    for (auto &pair : pairs_) {
        list.push_back(pair.first);
    }
    return list;
}



