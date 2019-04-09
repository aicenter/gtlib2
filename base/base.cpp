/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#include "base/base.h"
#include <cassert>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {

Action::Action(ActionId id) : id_(id) {}

bool Action::operator==(const Action &that) const {
    return id_ == that.id_;
}

string Action::toString() const {
    if (id_ == NO_ACTION) return "NoA";
    return to_string(id_);
}

ActionId Action::getId() const {
    return id_;
}

size_t Action::getHash() const {
    std::hash<size_t> h;
    return h(id_);
}

Observation::Observation(ObservationId id) : id_(id) {}

string Observation::toString() const {
    if (id_ == NO_OBSERVATION) {
        return "NoOb;";
    }
    return to_string(id_);
}

ObservationId Observation::getId() const {
    return id_;
}

bool Observation::operator==(const Observation &rhs) const {
    return id_ == rhs.id_;
}

size_t Observation::getHash() const {
    std::hash<size_t> h;
    return h(id_);
}

Outcome::Outcome(shared_ptr<State> s, vector<shared_ptr<Observation>> observations,
                 vector<double> rewards)
    : state_(move(s)), rewards_(move(rewards)), observations_(move(observations)) {}

size_t Outcome::getHash() const {
    size_t seed = state_->getHash();
    for (const auto &playerObservation : observations_) {
        boost::hash_combine(seed, playerObservation);
    }
    for (const auto &playerReward : rewards_) {
        boost::hash_combine(seed, playerReward);
    }
    return seed;
}

bool Outcome::operator==(const Outcome &rhs) const {
    if (observations_.size() != rhs.observations_.size()) {
        return false;
    }
    if (rewards_.size() != rhs.rewards_.size()) {
        return false;
    }
    if (!(state_ == rhs.state_)) {
        return false;
    }
    if (rewards_ != rhs.rewards_) {
        return false;
    }
    return observations_ == rhs.observations_;
}

size_t AOH::computeHash() const {
    size_t seed = 0;
    for (auto actionObservation : aoh_) {
        boost::hash_combine(seed, std::get<0>(actionObservation));
        boost::hash_combine(seed, std::get<1>(actionObservation));
    }
    boost::hash_combine(seed, player_);
    return seed;
}

AOH::AOH(Player player, const vector<ActionObservation> &aoHistory) {
    aoh_ = aoHistory;
    player_ = player;
    hashValue_ = computeHash();
}

bool AOH::operator==(const InformationSet &rhs) const {
    // cheap alternative to dynamic_cast,
    // this should be safe because we do not need any intermediate types
    // todo: Kuba please finish comment with better explanation :)
    if (typeid(rhs) == typeid(*this)) {
        const auto rhsAOH = static_cast<const AOH *>(&rhs);
        if (player_ != rhsAOH->player_ ||
            hashValue_ != rhsAOH->hashValue_ ||
            aoh_.size() != rhsAOH->aoh_.size()) {
            return false;
        }
        for (int i = 0; i < aoh_.size(); ++i) {
            if (aoh_[i] != rhsAOH->aoh_[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

string AOH::toString() const {
    string s = "Player: " + to_string(player_) + ",  init observation:" +
        to_string(aoh_.front().second) + ", hash value: " +
        to_string(hashValue_) + "\n";
    for (const auto &i : aoh_) {
        s += "Action: " + to_string(std::get<0>(i)) + ", Obs: " + to_string(std::get<1>(i)) + " | ";
    }
    return s;
}

State::State(Domain *domain) : domain_(domain) {}

Domain::Domain(unsigned int maxDepth, unsigned int numberOfPlayers) :
    maxDepth_(maxDepth), numberOfPlayers_(numberOfPlayers), maxUtility_(0) {}

const OutcomeDistribution &Domain::getRootStatesDistribution() const {
    return rootStatesDistribution_;
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
    return static_cast<int> (getPlayers().size());
}

string State::toString() const {
    return std::string();
}
}  // namespace GTLib2

#pragma clang diagnostic pop
