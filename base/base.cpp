//
// Created by rozliv on 07.08.2017.
//


#include <cassert>
#include "base.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {

    Action::Action(int id) : id(id) {}

    bool Action::operator==(Action &that) {
        return id == that.id;
    }

    string Action::toString() const {
        if (id == -1)
            return "NoA";
        return to_string(id);
    }

    int Action::getId() const {
        return id;
    }

    size_t Action::getHash() const {
        std::hash<int> h;
        return h(id);
    }

    Observation::Observation(int id) : id_(id) {}

    string Observation::toString() const {
        if (id_ == -1) {
            return "NoOb;";
        }
        return to_string(id_);
    }

    int Observation::getId() const {
        return id_;
    }

    Outcome::Outcome(shared_ptr<State> s, vector<shared_ptr<Observation>> ob,
                     vector<double> rew) :
            state(move(s)), ob_(move(ob)), rew_(move(rew)) {}

    Outcome::Outcome(shared_ptr<State> s, unordered_map<int, shared_ptr<Observation>> observations,
                     unordered_map<int, double> rewards) {
        state = s;
        this->observations = observations;
        this->rewards = rewards;

    }

    ProbDistribution::ProbDistribution(vector<pair<Outcome, double>> pairs) :
            distribution(move(pairs)) {}

    vector<Outcome> ProbDistribution::GetOutcomes() {
        vector<Outcome> list;
        for (auto &pair : distribution) {
            list.push_back(pair.first);
        }
        return list;
    }

    vector<double> ProbDistribution::GetProb() {
        vector<double> list;
        for (auto &pair : distribution) {
            list.push_back(pair.second);
        }
        return list;
    }

    ProbDistribution::ProbDistribution() {

    }

    AOH::AOH(int player, const vector<int> &hist) : player_(player) {
        for (int i = 0; i < hist.size(); i++) {
            int actionId = hist[i];
            i++;
            int observationId = hist[i];
            aoh.push_back(tuple<int, int>(actionId, observationId));
        }
        this->initialObservationId = -1;
    }


    size_t AOH::getHash() {
        if (seed_ == 0) {
            for (auto actionObservation : aoh) {
                seed_ ^= (std::get<0>(actionObservation) + std::get<1>(actionObservation)) + 0x9e3779b9 + (seed_ << 6) +
                         (seed_ >> 2);
            }
            seed_ ^= player_ + 0x9e3779b9 + (seed_ << 6) + (seed_ >> 2);
        }
        return seed_;
    }

    AOH::AOH(int player, int initialObservation, const vector<tuple<int, int>> &aoHistory) {
        aoh = aoHistory;
        player_ = player;
        this->initialObservationId = initialObservation;

    }

    bool AOH::operator==(InformationSet &other2) {
        auto *other = dynamic_cast<AOH *>(&other2);
        if (other != nullptr) {
            if (player_ != other->player_ ||
                getHash() != other->getHash() ||
                aoh.size() != other->aoh.size() ||
                initialObservationId != other->initialObservationId) {
                return false;
            }
            for (int i = 0; i < aoh.size(); i++) {
                if (std::get<0>(aoh[i]) != std::get<0>(other->aoh[i]) ||
                    std::get<1>(aoh[i]) != std::get<1>(other->aoh[i])) {
                    return false;
                }
            }
        }
        return true;
    }

    State::State() = default;

    Domain::Domain(int maxDepth, int numberOfPlayers) :
            maxDepth(maxDepth), numberOfPlayers(numberOfPlayers) {}

    double Domain::CalculateUtility(const vector<PureStrategy> &pure_strategies) {
        vector<Outcome> outcomes = rootStatesDistributionPtr->GetOutcomes();
        vector<double> p = rootStatesDistributionPtr->GetProb();
        double suma = 0;
        int z = 0;
        for (Outcome &o : outcomes) {
            if (!o.GetReward().empty())
                suma += o.GetReward()[0] * p[z];
            suma += ComputeUtility(o.GetState().get(), maxDepth, numberOfPlayers,
                                   pure_strategies, vector<vector<int>>(pure_strategies.size())) * p[z];
            ++z;
        }
        return suma;
    }

    double Domain::ComputeUtility(State *state, unsigned int depth,
                                  unsigned int players,
                                  const vector<PureStrategy> &pure_strategies,
                                  const vector<vector<int>> &aoh) {
        if (state == nullptr) {
            throw ("State is NULL");
        }

        if (depth == 0) {
            return 0;
        }

        auto search = vector<shared_ptr<Action>>(aoh.size());
        auto player = state->GetPlayers();
        for (int i = 0; i < aoh.size(); ++i) {
            if (player[i]) {
                auto ptr = make_shared<AOH>(i, aoh[i]);
                auto action = pure_strategies[i].Find(ptr);
                if (action != pure_strategies[i].End())
                    search[i] = action->second[0].second;
                else
                    throw ("No action found!");
            } else {
                search[i] = make_shared<Action>(NoA);
            }
        }
        ProbDistribution prob = state->PerformAction(search);
        vector<Outcome> outcomes = prob.GetOutcomes();
        vector<double> p = prob.GetProb();
        double suma = 0;
        int z = 0;
        for (Outcome &o : outcomes) {
            vector<vector<int>> locallist = aoh;
            for (int i = 0; i < search.size(); ++i) {
                locallist[i].push_back(search[i]->getId());
                locallist[i].push_back(o.GetObs()[i]->getId());
            }
            if (!o.GetReward().empty())
                suma += o.GetReward()[0] * p[z];
            suma += ComputeUtility(o.GetState().get(), depth - 1, players,
                                   pure_strategies, locallist) * p[z];
            ++z;
        }
        return suma;
    }

    vector<int> Domain::getPlayers() const {
        vector<int> players;
        for (int i = 0; i< getNumberOfPlayers(); ++i){
            players.push_back(i);
        }
        return players;
    }


    unordered_set<int> State::getPlayersSet() const {
        unordered_set<int> set;
        vector<bool> players = GetPlayers();
        for (int i = 0; i < players.size(); i++) {
            if (players[i]) {
                set.insert(i);
            }
        }
        return set;
    }

    void State::GetActions(vector<shared_ptr<Action>> &list, int player) const {
        assert(false);
    }

    ProbDistribution State::PerformAction(const vector<shared_ptr<Action>> &actions) {
        assert(false);
        return ProbDistribution(vector<pair<Outcome, double>>());
    }

    const vector<bool> &State::GetPlayers() const {
        assert(false);
        return vector<bool>();
    }

    void State::AddString(const string &s, int player) {
        assert(false);

    }

//ProbDistribution State::performActions(unordered_map<int, shared_ptr<Action>> &actions) {
//
//    vector<shared_ptr<Action>> actionsVector;
//    for (int i = 0; i< GetNumPlayers(); i++) {
//        if (actions.count(i) > 0 ) {
//            actionsVector.push_back(actions[i]);
//        } else {
//            actionsVector.push_back(make_shared<Action>(NoA));
//        }
//    }
//
//    auto probDistr = PerformAction(actionsVector);
//
//    return probDistr;
//}
    int countStates; // TODO: Remove this

}

int countStates; // TODO: Remove this

#pragma clang diagnostic pop