//
// Created by Jakub Rozlivek on 07.08.2017.
//


#include <cassert>
#include "base.h"
#include <utility>

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

  Outcome::Outcome(shared_ptr<State> s, vector<shared_ptr<Observation>> observations,
                   vector<double> rewards)
          : state(move(s)), rewards(move(rewards)), observations(move(observations)) {}

    size_t Outcome::getHash() const {
        size_t seed = state->getHash();
        for (const auto &playerObservation : observations) {
            boost::hash_combine(seed, playerObservation);
        }
        for (const auto &playerReward : rewards) {
            boost::hash_combine(seed, playerReward);
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
        if(rewards != rhs.rewards) {
            return false;
        }
        return !(observations != rhs.observations);
    }

    size_t AOH::computeHash() const {
        size_t seed = 0;
        for (auto actionObservation : aoh) {
//            seed ^=  std::get<0>(actionObservation) + 0x9e3779b9 + (seed<<6) + (seed>>2);
//            seed ^=  std::get<1>(actionObservation) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            boost::hash_combine(seed, std::get<0>(actionObservation));
            boost::hash_combine(seed, std::get<1>(actionObservation));
            }
        return seed;
    }

    AOH::AOH(int player, int initialObservation, const vector<pair<int, int>> &aoHistory) {
        aoh = aoHistory;
        this->player = player;
        this->initialObservationId = initialObservation;
        hashValue = computeHash();
    }

    bool AOH::operator==(const InformationSet &rhs) const {
        const auto rhsAOH = dynamic_cast<const AOH *>(&rhs);
        if (rhsAOH != nullptr) {
            if (player != rhsAOH->player ||
                hashValue != rhsAOH->hashValue ||
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

  string AOH::toString() const {
      string s = "Player: " + to_string(player) + ",  init observation:" +
              to_string(initialObservationId) + ", hash value: " +
              to_string(hashValue) + "\n";
    for (const auto &i : aoh) {
      s+= "Action: " + to_string(std::get<0>(i)) + ", Obs: " + to_string(std::get<1>(i)) + " | ";
    }
    return s;
  }


  State::State(Domain* domain):domain(domain) {};

    Domain::Domain(int maxDepth, unsigned int numberOfPlayers) :
            maxDepth(maxDepth), numberOfPlayers(numberOfPlayers) {}


    OutcomeDistribution Domain::getRootStatesDistribution() const {
        return rootStatesDistribution;
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

    string State::toString() const {
        return std::string();
    }
}


#pragma clang diagnostic pop