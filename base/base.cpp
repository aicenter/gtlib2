//
// Created by rozliv on 07.08.2017.
//


#include <cassert>
#include "base.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {

    Action::Action(int id) : id(id) {}

    bool Action::operator==(const Action &that) const{
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

    Observation::Observation(int id) : id(id) {}

    string Observation::toString() const {
        if (id == -1) {
            return "NoOb;";
        }
        return to_string(id);
    }

    int Observation::getId() const {
        return id;
    }

    bool Observation::operator==(const Observation &rhs) const {
        return id == rhs.id;
    }

    size_t Observation::getHash() const {
        std::hash<int> h;
        return h(id);
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

    size_t Outcome::getHash() const {
        size_t seed = state->getHash();
        for (const auto &playerObservation : observations) {
            boost::hash_combine(seed, std::get<0>(playerObservation));
            boost::hash_combine(seed, std::get<1>(playerObservation));
        }
        for (const auto &playerReward : rewards) {
            boost::hash_combine(seed, std::get<0>(playerReward));
            boost::hash_combine(seed, std::get<1>(playerReward));
        }
        return seed;
    }

    bool Outcome::operator==(const Outcome &rhs) const {
        if (observations.size() != rhs.observations.size()) {
            return false;
        }
        if (rewards.size() != rhs.rewards.size()) {
            return false;
        }
        if (!(state == rhs.state)) {
            return false;
        }
        for (const auto &playerReward : rewards) {
            auto player = std::get<0>(playerReward);
            auto reward = std::get<1>(playerReward);
            if (rhs.rewards.find(player) == rhs.rewards.end() ||
                    rhs.rewards.at(player) != reward) {
                return false;
            }
        }
        for (const auto &playerObservation : observations) {
            auto player = std::get<0>(playerObservation);
            auto observation = std::get<1>(playerObservation);
            if (rhs.observations.find(player) == rhs.observations.end() ||
                rhs.observations.at(player) != observation) {
                return false;
            }
        }
        return true;
    }

    OutcomeDistributionOld::OutcomeDistributionOld(vector<pair<Outcome, double>> pairs) :
            distribution(move(pairs)) {}

    vector<Outcome> OutcomeDistributionOld::GetOutcomes() {
        vector<Outcome> list;
        for (auto &pair : distribution) {
            list.push_back(pair.first);
        }
        return list;
    }

    vector<double> OutcomeDistributionOld::GetProb() {
        vector<double> list;
        for (auto &pair : distribution) {
            list.push_back(pair.second);
        }
        return list;
    }

    OutcomeDistributionOld::OutcomeDistributionOld() {

    }

    AOH::AOH(int player, const vector<int> &hist) : player_(player) {
        for (int i = 0; i < hist.size(); i++) {
            int actionId = hist[i];
            i++;
            int observationId = hist[i];
            aoh.push_back(tuple<int, int>(actionId, observationId));
        }
        this->initialObservationId = -1;
        hashValue = computeHash();
    }

    size_t AOH::computeHash() const {
        size_t seed = 0;
        for (auto actionObservation : aoh) {
            boost::hash_combine(seed, std::get<0>(actionObservation));
            boost::hash_combine(seed, std::get<1>(actionObservation));
            }
        return seed;
    }

    size_t AOH::getHash() const {
        return hashValue;
    }

    AOH::AOH(int player, int initialObservation, const vector<pair<int, int>> &aoHistory) {
        aoh = aoHistory;
        player_ = player;
        this->initialObservationId = initialObservation;
        hashValue = computeHash();
    }

    bool AOH::operator==(const InformationSet &rhs) const {
        const auto rhsAOH = dynamic_cast<const AOH*>(&rhs);

        if (rhsAOH != nullptr) {
            if (player_ != rhsAOH->player_ ||
                getHash() != rhsAOH->getHash() ||
                aoh.size() != rhsAOH->aoh.size() ||
                initialObservationId != rhsAOH->initialObservationId) {
                return false;
            }
            for (int i = 0; i < aoh.size(); i++) {
                if (std::get<0>(aoh[i]) != std::get<0>(rhsAOH->aoh[i]) ||
                    std::get<1>(aoh[i]) != std::get<1>(rhsAOH->aoh[i])) {
                    return false;
                }
            }
        }
        return true;
    }

    int AOH::getNumberOfActions() const {
        return (int) aoh.size();
    }

    vector<pair<int, int>> AOH::getAOHistory() const {
        return aoh;
    }

    int AOH::getInitialObservationId() const {
        return initialObservationId;
    }

    int AOH::getPlayer() const {
        return player_;
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
        OutcomeDistributionOld prob = state->PerformAction(search);
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
        //TODO: Implement this in phantom and pursuits domains and remove this and make this method abstract.
        assert(("domain::getPlayers not implemented!",false));
        vector<int> players;
        for (int i = 0; i< getNumberOfPlayers(); ++i){
            players.push_back(i);
        }
        return players;
    }

    OutcomeDistribution Domain::getRootStatesDistribution() const {
        return rootStatesDistribution;
    }


    unordered_set<int> State::getPlayersSet() const {
        //TODO: Implement this in phantom and pursuits domains and remove this and make this method abstract.
        assert(("getPlayers not implemented", false));
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

    OutcomeDistributionOld State::PerformAction(const vector<shared_ptr<Action>> &actions) {
        assert(false);
        return OutcomeDistributionOld(vector<pair<Outcome, double>>());
    }

    const vector<bool> &State::GetPlayers() const {
        assert(false);
        return vector<bool>();
    }

    void State::AddString(const string &s, int player) {
        assert(false);

    }

    bool State::operator==(const State &rhs) const {
        assert(("operator == is not implemented", false));
        return false;
    }

    size_t State::getHash() const {
        assert(("getHash is not implemented", false));
        return 0;
    }

    int State::getNumberOfPlayers() const {
        return (int) getPlayersSet().size();
    }

    string State::toString(int player) const {
        return std::string();
    }

    string State::toString() const {
        return std::string();
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
//    auto probDistr = OldPerformAction(actionsVector);
//
//    return probDistr;
//}
    int countStates; // TODO: Remove this

}

int countStates; // TODO: Remove this

#pragma clang diagnostic pop