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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {

Outcome::Outcome(shared_ptr<State> _state,
                 vector<shared_ptr<Observation>> _privateObservations,
                 shared_ptr<Observation> _publicObservation,
                 vector<double> _rewards) :
    state(move(_state)),
    privateObservations(move(_privateObservations)),
    publicObservation(move(_publicObservation)),
    rewards(move(_rewards)),
    hash(hashCombine(1984534684564L, state, privateObservations, publicObservation)) {}


bool Outcome::operator==(const Outcome &rhs) const {
    return hash == rhs.hash
        && state == rhs.state
        && privateObservations == rhs.privateObservations
        && publicObservation == rhs.publicObservation
        && rewards == rhs.rewards;
}


AOH::AOH(Player player, bool isPlayerActing, vector<ActionObservationIds> aoHistory)
    : player_(player), isPlayerActing_(isPlayerActing), aoh_(move(aoHistory)),
      hash_(hashCombine(5645138468, aoh_, player_)) {}

bool AOH::operator==(const InformationSet &rhs) const {
    // cheap alternative to dynamic_cast,
    // this should be safe because we do not need any intermediate types
    // todo: Kuba please finish comment with better explanation :)
    if (typeid(rhs) != typeid(*this)) {
        return false;
    }

    const auto rhsAOH = static_cast<const AOH *>(&rhs);
    if (hash_ != rhsAOH->hash_
        || player_ != rhsAOH->player_
        || aoh_.size() != rhsAOH->aoh_.size()
        || isPlayerActing_ != rhsAOH->isPlayerActing_) {
        return false;
    }
    for (int i = 0; i < aoh_.size(); ++i) {
        if (aoh_[i] != rhsAOH->aoh_[i]) {
            return false;
        }
    }
    return true;
}

string AOH::toString() const {
    std::stringstream ss;
    ss << " Pl" << int(player_) << " " << aoh_;
    return ss.str();
}

bool ActionSequence::operator==(const ActionSequence &rhs) const {
    if (hash_ != rhs.hash_) return false;
    if (sequence_.size() != rhs.sequence_.size()) return false;
    for (int i = 0; i < sequence_.size(); ++i) {
        if (sequence_[i].getHash() != rhs.sequence_[i].getHash()
            || (sequence_[i] != rhs.sequence_[i])) {
            return false;
        }
    }
    return true;
}
std::ostream &operator<<(std::ostream &os, const ActionSequence &sequence) {
    os << sequence.sequence_;
    return os;
}

State::State(const Domain *domain, HashType hash) : domain_(domain), hash_(hash) {}

OutcomeDistribution State::performPartialActions(const vector<PlayerAction> &plActions) const {
    // no padding is necessary (but we don't check if the players are indeed the correct ones!)
    auto actions = vector<shared_ptr<Action>>();
    actions.reserve(domain_->getNumberOfPlayers());

    for (int i = 0; i < domain_->getNumberOfPlayers(); ++i) {
        shared_ptr<Action> actionFound;
        for (const auto&[requestedPlayer, requestedAction] : plActions) {
            if (Player(i) == requestedPlayer) {
                actionFound = requestedAction;
                break;
            }
        }

        actions.emplace_back(actionFound ? actionFound : domain_->getNoAction());
    }

    return performActions(actions);
}

// todo: explicit max utility!!!!
Domain::Domain(unsigned int maxStateDepth, unsigned int numberOfPlayers, bool isZeroSum,
               shared_ptr<Action> noAction, shared_ptr<Observation> noObservation) :
    maxStateDepth_(maxStateDepth), numberOfPlayers_(numberOfPlayers), isZeroSum_(isZeroSum),
    maxUtility_(0), noAction_(move(noAction)), noObservation_(move(noObservation)) {
    assert(noAction_->getId() == NO_ACTION);
    assert(noObservation_->getId() == NO_OBSERVATION);
}

const OutcomeDistribution &Domain::getRootStatesDistribution() const {
#ifndef NDEBUG // equivalent to assert
    double sum = 0.;
    for (const auto&[_, prob] : rootStatesDistribution_) {
        sum += prob;
    }
    assert(sum > (1 - 1e-6));
    assert(sum < (1 + 1e-6));
#endif
    return rootStatesDistribution_;
}

bool State::isPlayerMakingMove(Player pl) const {
    for (const auto movingPl : getPlayers()) {
        if (movingPl == pl) return true;
    }
    return false;
}

string State::toString() const {
    return std::string("not implemented");
}

bool ActionObservationIds::operator==(const ActionObservationIds &rhs) const {
    return action == rhs.action
        && observation == rhs.observation;
}
bool ActionObservationIds::operator!=(const ActionObservationIds &rhs) const {
    return !(rhs == *this);
}
}  // namespace GTLib2

#pragma clang diagnostic pop
