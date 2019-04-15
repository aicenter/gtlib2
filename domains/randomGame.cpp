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

#include <random>
#include "domains/randomGame.h"

namespace GTLib2 {
namespace domains {

RandomGameAction::RandomGameAction(ActionId id) : Action(id) {}

bool RandomGameAction::operator==(const Action &other) const {
    if (typeid(*this) != typeid(other)) {
        return false;
    }
    auto otherAction = dynamic_cast<const RandomGameAction &>(other);
    return this->id_ == otherAction.id_;
}

size_t RandomGameAction::getHash() const {
    return Action::getHash();
}

RandomGameDomain::RandomGameDomain(unsigned int maxDepth, long seed, int maxBranchingFactor,
                                   int maxDifferentObservations, bool binaryUtility,
                                   bool fixedBranchingFactor, int maxCenterModification) :
       Domain(maxDepth, 2), seed_(seed), maxBranchingFactor_(maxBranchingFactor), maxDifferentObservations_(maxDifferentObservations),
       binary_utility_(binaryUtility), fixedBranchingFactor_(fixedBranchingFactor), maxCenterModification_(maxCenterModification) {

    assert(maxDepth > 0);
    assert(maxBranchingFactor > 1);
    assert(maxDifferentObservations > 0);
    assert(maxCenterModification > 0);

    std::mt19937 generator(seed);
    auto rootState = make_shared<RandomGameState>(this, generator(), 0, 0);
    vector<shared_ptr<Observation>> observations{make_shared<RandomGameObservation>(NO_OBSERVATION),
                                                 make_shared<RandomGameObservation>(NO_OBSERVATION)};
    Outcome outcome(rootState, observations, {0.0, 0.0});
    rootStatesDistribution_.emplace_back(outcome, 1.0);
}

string RandomGameDomain::getInfo() const {
    return "Random Game"
           "\nMax depth: " + to_string(maxDepth_) +
           "\nSeed: " + to_string(seed_) +
           "\nMax branching factor: " + to_string(maxBranchingFactor_) +
           "\nFixed branching factor: " + to_string(fixedBranchingFactor_) +
           "\nMax different observations: " + to_string(maxDifferentObservations_) +
           "\nBinary utility: " + to_string(binary_utility_) + "\n";
}


RandomGameState::RandomGameState(Domain *domain, int id, int center, int depth) :
    State(domain), stateId_(id), center_(center), depth_(depth) {}


vector<shared_ptr<Action>> RandomGameState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;
    const auto RGdomain = dynamic_cast<RandomGameDomain *>(domain_);
    int possibleMoves = RGdomain->getMaxBranchingFactor();
    if (!RGdomain->isFixedBranchingFactor()) {
        std::uniform_int_distribution<int> distribution(2, RGdomain->getMaxBranchingFactor());
//        std::cout << "this->getHash: " << this->getHash() << std::endl;
        std::mt19937 generator((stateId_ + center_ + depth_ + player)*31);
        possibleMoves = distribution(generator);
    }
    std::cout <<"Player " << std::to_string(player) << ": "  << possibleMoves << std::endl;
    for (int i = 0; i < possibleMoves; ++i) {
        actions.push_back(make_shared<RandomGameAction>(i));
    }
    return actions;
}

OutcomeDistribution RandomGameState::performActions(const vector<PlayerAction> &actions) const {
    auto RGdomain = dynamic_cast<RandomGameDomain *>(domain_);
    auto p1Action = dynamic_cast<RandomGameAction *>(actions[0].second.get());
    auto p2Action = dynamic_cast<RandomGameAction *>(actions[1].second.get());

    int newID = (stateId_ + p1Action->getId() + p2Action->getId()) * 31 + 17;

    std::mt19937 generator(newID);
    std::uniform_int_distribution<int> centerDistribution(-RGdomain->getMaxCenterModification(), RGdomain->getMaxCenterModification());
    int newCenter = center_ + centerDistribution(generator);

//    std::cout << "newCenter: " << newCenter << " newID: " << newID << std::endl;

    vector<double> rewards{0.0, 0.0};
    auto newState = make_shared<RandomGameState>(domain_, newID, newCenter, depth_+1);
    std::uniform_int_distribution<int> observationDistribution(RGdomain->getMaxDifferentObservations()-1);
    int observation = observationDistribution(generator);

    vector<shared_ptr<Observation>> observations{make_shared<RandomGameObservation>(observation),
                                                 make_shared<RandomGameObservation>(observation)};
    double reward = newCenter;
    if (RGdomain->getMaxDepth() == newState->depth_ && RGdomain->isBinaryUtility() && newCenter != 0){ // endGame
        reward = newCenter / std::abs(newCenter);
//        std::cout << rewards[0] << ":" << rewards[1] << std::endl;
    }
    rewards[0] = reward;
    rewards[1] = -reward;
    Outcome outcome(newState, observations, rewards);
    OutcomeDistribution dist;
    dist.emplace_back(outcome, 1.0);

    return dist;
}

size_t RandomGameState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, stateId_);
    boost::hash_combine(seed, center_);
    return seed;
}

unsigned long RandomGameState::countAvailableActionsFor(Player player) const {
    return 0;
}

vector<Player> RandomGameState::getPlayers() const {
    auto RGdomain = dynamic_cast<RandomGameDomain *>(domain_);
    if(this->depth_ == RGdomain->getMaxDepth()) return {};
    return {0, 1};
}

string RandomGameState::toString() const {
    return  "stateId: " + to_string(stateId_) +
            "\ncenter: " + to_string(center_) +
            "\ndepth: " + to_string(depth_) + '\n';
}

bool RandomGameState::operator==(const State &other) const {
    if (typeid(*this) != typeid(other)) {
        return false;
    }
    auto otherState = dynamic_cast<const RandomGameState &>(other);
    return stateId_ == otherState.stateId_ &&
           center_ == otherState.center_ &&
           depth_ == otherState.depth_;
}


RandomGameObservation::RandomGameObservation(ObservationId id) : Observation(id) {}
}
}
