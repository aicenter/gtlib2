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

namespace GTLib2::domains {

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

string RandomGameAction::toString() const {
    return "Action ID " + to_string(id_);
}

RandomGameDomain::RandomGameDomain(RandomGameSettings settings) :
    Domain(settings.maximalDepth, 2),
    seed_(settings.seed),
    maxBranchingFactor_(settings.maxBranchingFactor),
    maxDifferentObservations_(settings.maxDifferentObservations),
    binaryUtility_(settings.binaryUtility),
    fixedBranchingFactor_(settings.fixedBranchingFactor),
    maxCenterModification_(settings.maxCenterModification) {

    assert(maxDepth > 0);
    assert(maxBranchingFactor > 1);
    assert(maxDifferentObservations > 0);
    assert(maxCenterModification > 0);

    if (binaryUtility_) {
        maxUtility_ = 1.0;
    } else {
        maxUtility_ = maxCenterModification_ * maxDepth_;
    }
    std::mt19937 generator(seed_);
    vector<long> histories = {0L, 0L};

    auto rootState = make_shared<RandomGameState>(this, generator(), histories, 0, 0);
    auto pubObs = make_shared<RandomGameObservation>(NO_OBSERVATION);
    auto p0Obs = make_shared<RandomGameObservation>(*pubObs);
    auto p1Obs = make_shared<RandomGameObservation>(*pubObs);

    Outcome outcome(rootState, {p0Obs, p1Obs}, pubObs, {0.0, 0.0});
    rootStatesDistribution_.emplace_back(outcome, 1.0);
}

string RandomGameDomain::getInfo() const {
    return "Random Game"
           "\nMax depth: " + to_string(maxDepth_) +
        "\nSeed: " + to_string(seed_) +
        "\nMax branching factor: " + to_string(maxBranchingFactor_) +
        "\nFixed branching factor: " + to_string(fixedBranchingFactor_) +
        "\nMax different observations: " + to_string(maxDifferentObservations_) +
        "\nBinary utility: " + to_string(binaryUtility_) + "\n";
}

RandomGameState::RandomGameState(Domain *domain,
                                 int id,
                                 vector<long> histories,
                                 int center,
                                 int depth) :
    State(domain), stateId_(id), center_(center), depth_(depth) {

    playerHistories_ = move(histories);
}

vector<shared_ptr<Action>> RandomGameState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;
    auto possibleMoves = countAvailableActionsFor(player);
    cout << "Player: " << (player == 1) << ", moves: " << possibleMoves << ", depth: "
         << this->depth_ << endl;
    for (int i = 0; i < possibleMoves; ++i) {
        actions.push_back(make_shared<RandomGameAction>(i));
    }
    return actions;
}

unsigned long RandomGameState::countAvailableActionsFor(Player player) const {
    auto RGdomain = dynamic_cast<RandomGameDomain *>(domain_);
    unsigned long possibleMoves = RGdomain->getMaxBranchingFactor();
    if (!RGdomain->isFixedBranchingFactor()) {
        std::uniform_int_distribution<long> distribution(2, RGdomain->getMaxBranchingFactor());
        std::mt19937 generator(playerHistories_[player]);
        possibleMoves = distribution(generator);
    }
    return possibleMoves;
}

OutcomeDistribution RandomGameState::performActions(const vector<PlayerAction> &actions) const {
    auto RGdomain = dynamic_cast<RandomGameDomain *>(domain_);
    auto p0Action = dynamic_cast<RandomGameAction *>(actions[0].second.get());
    auto p1Action = dynamic_cast<RandomGameAction *>(actions[1].second.get());

    int newID = (stateId_ + p0Action->getId() + p1Action->getId()) * 31 + 17;

    std::mt19937 generator(newID);
    std::uniform_int_distribution<int> centerDistribution
        (-RGdomain->getMaxCenterModification(), RGdomain->getMaxCenterModification());
    int newCenter = center_ + centerDistribution(generator);

//    std::cout << "newCenter: " << newCenter << " newID: " << newID << std::endl;

    vector<double> rewards{0.0, 0.0};
    vector<long> newHistories(playerHistories_);

    std::uniform_int_distribution<int>
        observationDistribution(RGdomain->getMaxDifferentObservations() - 1);
    int observation = observationDistribution(generator);

    auto pubObs = make_shared<RandomGameObservation>(observation);
    auto player0Obs = p1Action->getId() % RGdomain->getMaxDifferentObservations();
    auto player1Obs = p0Action->getId() % RGdomain->getMaxDifferentObservations();
    vector<shared_ptr<Observation>> observations{make_shared<RandomGameObservation>(player0Obs),
                                                 make_shared<RandomGameObservation>(player1Obs)};

    newHistories[0] += p0Action->getId() * 31 + player0Obs * 17;
    newHistories[1] += p1Action->getId() * 31 + player0Obs * 17;

    double reward = newCenter;
    if (RGdomain->isBinaryUtility()) {
        reward = (0 < newCenter) - (newCenter < 0);
//        std::cout << rewards[0] << ":" << rewards[1] << std::endl;
    }
    rewards[0] = reward;
    rewards[1] = -reward;

    auto newState =
        make_shared<RandomGameState>(domain_, newID, newHistories, newCenter, depth_ + 1);
    Outcome outcome(newState, observations, pubObs, rewards);
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


vector<Player> RandomGameState::getPlayers() const {
    auto RGdomain = dynamic_cast<RandomGameDomain *>(domain_);
    if (this->depth_ == RGdomain->getMaxDepth()) return {};
    return {0, 1};
}

string RandomGameState::toString() const {
    return "stateId: " + to_string(stateId_) +
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