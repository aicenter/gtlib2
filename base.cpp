//
// Created by rozliv on 07.08.2017.
//

#include "base.h"

Action::Action(int id): id_(id) {}

string Action::ToString() {
    return string();
}

Observation::Observation(int id): id_(id) {}

string Observation::ToString() {
    return string();
}

State::State() = default;

Domain::Domain(const unique_ptr<State> &r, int maxPlayers):
        root_(r), maxPlayers_(maxPlayers) {}

string Domain::GetInfo() {}

Outcome::Outcome(unique_ptr<State> s, vector<unique_ptr<Observation>> ob,
                 const vector<double> &rew):
        st_(move(s)), ob_(move(ob)), rew_(rew) {}

ProbDistribution::ProbDistribution(vector<std::pair<Outcome, double>> pairs):
        pairs_(move(pairs)) {}

Outcome ProbDistribution::GetRandom() {
    int r = rand() % pairs_.size();
    return move(pairs_[r].first);
}

vector<Outcome>  ProbDistribution::GetOutcomes() {
    vector<Outcome> list;
    for (auto &pair : pairs_) {
        Outcome o(pair.first.getState(), pair.first.getObs(), pair.first.getReward());
        list.push_back(move(o));
    }
    return list;
}
