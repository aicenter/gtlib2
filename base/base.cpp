//
// Created by Jakub Rozlivek on 07.08.2017.
//


//#include <cassert>
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


    AOH::AOH(int player, const vector<int> &hist) : player(player) {
        for (int i = 0; i < hist.size(); i++) {
            int actionId = hist[i];
            i++;
            int observationId = hist[i];
            aoh.push_back(pair<int, int>(actionId, observationId));
        }
        this->initialObservationId = 1;
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
        this->player = player;
        this->initialObservationId = initialObservation;
        hashValue = computeHash();
    }

    bool AOH::operator==(const InformationSet &rhs) const {
        const auto rhsAOH = dynamic_cast<const AOH*>(&rhs);

        if (rhsAOH != nullptr) {
            if (player != rhsAOH->player ||
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
        return player;
    }


    State::State() = default;

    Domain::Domain(int maxDepth, int numberOfPlayers) :
            maxDepth(maxDepth), numberOfPlayers(numberOfPlayers) {}


    OutcomeDistribution Domain::getRootStatesDistribution() const {
        return rootStatesDistribution;
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
        return (int) getPlayers().size();
    }

    string State::toString(int player) const {
        return std::string();
    }

    string State::toString() const {
        return std::string();
    }
}


#pragma clang diagnostic pop