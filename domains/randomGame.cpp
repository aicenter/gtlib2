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

RandomGameDomain::RandomGameDomain(unsigned int maxDepth, long seed, int maxBranchingFactor,
                                   int maxDifferentObservations, bool binaryUtility,
                                   bool fixedBranchingFactor, int maxCenterModification) :
       Domain(maxDepth, 2), seed_(seed), maxBranchingFactor_(maxBranchingFactor), maxDifferentObservations_(maxDifferentObservations),
       binary_utility_(binaryUtility), fixedBranchingFactor_(fixedBranchingFactor), maxCenterModification_(maxCenterModification) {

    random_ = std::mt19937(seed);
    auto rootState = make_shared<RandomGameState>(this, randomInt(seed_), 0, 0);
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
    State(domain), ID_(id), center_(center), depth_(depth) {}


vector<shared_ptr<Action>> RandomGameState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;
    const auto RGdomain = static_cast<RandomGameDomain *>(domain_);
    int possibleMoves = RGdomain->getMaxBranchingFactor();
    if (!RGdomain->isFixedBranchingFactor()) possibleMoves = randomIntBound(this->getHash()*player, RGdomain->getMaxBranchingFactor()-1)+2;
//    std::cout <<"Player " << std::to_string(player) << ": "  << possibleMoves << std::endl;
    for (int i = 0; i < possibleMoves; ++i) {
        actions.push_back(make_shared<RandomGameAction>(i));
    }
    return actions;
}

OutcomeDistribution RandomGameState::performActions(const vector<PlayerAction> &actions) const {
    auto RGdomain = static_cast<RandomGameDomain *>(domain_);
//    auto p1Action = dynamic_cast<RandomGameAction *>(actions[0].second.get());
//    auto p2Action = dynamic_cast<RandomGameAction *>(actions[1].second.get());

    int newID = randomInt(ID_);
    double newCenter = center_ + randomIntBound(newID, RGdomain->getMaxCenterModification()*2 + 1) - RGdomain->getMaxCenterModification(); // TODO: range maxCenterModification
    std::cout << newCenter << std::endl;
    vector<double> rewards{newCenter, -newCenter};
    auto newState = make_shared<RandomGameState>(domain_, newID, newCenter, depth_+1);
    int observation = randomIntBound(ID_, RGdomain->getMaxDifferentObservations());
    vector<shared_ptr<Observation>> observations{make_shared<RandomGameObservation>(observation),
                                                 make_shared<RandomGameObservation>(observation)};

    if (RGdomain->getMaxDepth() == newState->depth_){ // endGame
        double reward = 0.0;
        if (RGdomain->isBinaryUtility()){
            reward = newCenter / std::abs(newCenter);
            rewards[0] = reward;
            rewards[1] = -reward;
        }
//        std::cout << rewards[0] << ":" << rewards[1] << std::endl;
    }
    Outcome outcome(newState, observations, rewards);
    OutcomeDistribution distribution;
    distribution.emplace_back(outcome, 1.0);
    return distribution;
}

size_t RandomGameState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, ID_);
    boost::hash_combine(seed, center_);
    return seed;
}

unsigned long RandomGameState::countAvailableActionsFor(Player player) const {
    return 0;
}

vector<Player> RandomGameState::getPlayers() const {
    return {0, 1};
}


RandomGameObservation::RandomGameObservation(ObservationId id) : Observation(id) {}
}
}
