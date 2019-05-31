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

#include "domains/randomGame.h"

namespace GTLib2::domains {

bool RandomGameAction::operator==(const Action &other) const {
    if (typeid(*this) != typeid(other)) {
        return false;
    }
    auto otherAction = dynamic_cast<const RandomGameAction &>(other);
    return this->id_ == otherAction.id_;
}

RandomGameDomain::RandomGameDomain(RandomGameSettings settings) :
    Domain(settings.maxDepth,
           2,
           make_shared<RandomGameAction>(),
           make_shared<RandomGameObservation>()),
    seed_(settings.seed),
    maxBranchingFactor_(settings.maxBranchingFactor),
    maxDifferentObservations_(settings.maxDifferentObservations),
    binaryUtility_(settings.binaryUtility),
    fixedBranchingFactor_(settings.fixedBranchingFactor),
    maxRewardModification_(settings.maxRewardModification),
    utilityCorrelation_(settings.utilityCorrelation) {

    assert(maxBranchingFactor_ > 1);
    assert(maxDifferentObservations_ > 0);
    assert(maxRewardModification_ > 0);

    maxUtility_ = settings.utilityCorrelation
                  ? binaryUtility_
                    ? 1.0 : maxRewardModification_ * maxStateDepth_
                  : settings.maxUtility;

    std::mt19937 generator(seed_);
    long initValue = generator();
    vector<long> histories = {initValue, initValue};

    auto rootState = make_shared<RandomGameState>(this, initValue, histories, 0, 0);
    auto pubObs = make_shared<RandomGameObservation>(NO_OBSERVATION);
    auto p0Obs = make_shared<RandomGameObservation>(*pubObs);
    auto p1Obs = make_shared<RandomGameObservation>(*pubObs);

    Outcome outcome(rootState, {p0Obs, p1Obs}, pubObs, {0.0, 0.0});
    rootStatesDistribution_.emplace_back(outcome, 1.0);
}

string RandomGameDomain::getInfo() const {
    return "Random Game"
           "\nMax depth: " + to_string(maxStateDepth_) +
        "\nSeed: " + to_string(seed_) +
        "\nMax branching factor: " + to_string(maxBranchingFactor_) +
        "\nMax different observations: " + to_string(maxDifferentObservations_) +
        "\nMax reward modification: " + to_string(maxRewardModification_) +
        "\nMax utility: " + to_string(maxUtility_) +
        "\nUtility correlation: " + boolToString(utilityCorrelation_) +
        "\nFixed branching factor: " + boolToString(fixedBranchingFactor_) +
        "\nBinary utility: " + boolToString(binaryUtility_) + "\n";
}

vector<shared_ptr<Action>> RandomGameState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;
    auto possibleMoves = countAvailableActionsFor(player);
    for (int i = 0; i < possibleMoves; ++i) {
        actions.push_back(make_shared<RandomGameAction>(i));
    }
    return actions;
}

unsigned long RandomGameState::countAvailableActionsFor(Player player) const {
    auto RGdomain = dynamic_cast<const RandomGameDomain *>(domain_);
    unsigned long possibleMoves = RGdomain->getMaxBranchingFactor();
    if (!RGdomain->isFixedBranchingFactor()) {
        std::uniform_int_distribution<long> distribution(2, RGdomain->getMaxBranchingFactor());
        std::mt19937 generator(playerActionSeeds_[player]);
        possibleMoves = distribution(generator);
    }
    return possibleMoves;
}

OutcomeDistribution
RandomGameState::performActions(const vector<shared_ptr<Action>> &actions) const {
    auto RGdomain = dynamic_cast<const RandomGameDomain *>(domain_);
    auto p0Action = dynamic_cast<RandomGameAction &>(*actions[0]);
    auto p1Action = dynamic_cast<RandomGameAction &>(*actions[1]);

    auto pubObs = make_shared<RandomGameObservation>(NO_OBSERVATION);
    auto player0Obs = p1Action.getId() % RGdomain->getMaxDifferentObservations();
    auto player1Obs = p0Action.getId() % RGdomain->getMaxDifferentObservations();
    assert(player0Obs >= 0);
    assert(player1Obs >= 0);
    vector<shared_ptr<Observation>> observations{make_shared<RandomGameObservation>(player0Obs),
                                                 make_shared<RandomGameObservation>(player1Obs)};
    // simple hashed AOH history
    vector<long> newPlayerSeeds{
        playerActionSeeds_[0] + (p0Action.getId() + 1) * 31 + (player0Obs + 1) * 17,
        playerActionSeeds_[1] + (p1Action.getId() + 1) * 31 + (player1Obs + 1) * 17
    };

    long newStateSeed = (stateSeed_ + p0Action.getId() + p1Action.getId()) * 31 + 17;
    std::mt19937 generator(newStateSeed);

    double newReward;
    if (RGdomain->isUtilityCorrelation()) {
        std::uniform_real_distribution<double> cumulativeRewardDistribution
            (-RGdomain->getMaxRewardModification(), RGdomain->getMaxRewardModification());
        newReward = cumulativeRewardDistribution(generator);
    } else {
        std::uniform_real_distribution<double>
            rewardDistribution(RGdomain->getMinUtility() / RGdomain->getMaxStateDepth(),
                               RGdomain->getMaxUtility() / RGdomain->getMaxStateDepth());
        newReward = rewardDistribution(generator);
    }
    auto newState = make_shared<RandomGameState>(domain_,
                                                 newStateSeed,
                                                 newPlayerSeeds,
                                                 newReward + cumulativeReward_,
                                                 depth_ + 1);
    // if binary utility, give reward only in leaf node
    if (RGdomain->isBinaryUtility()) {
        newReward = newState->isTerminal() ? signum(newState->cumulativeReward_) : 0.0;
    }
    vector<double> rewards{newReward, -newReward};

    Outcome outcome(newState, observations, pubObs, rewards);
    OutcomeDistribution dist;
    dist.emplace_back(outcome);
    return dist;
}

string RandomGameState::toString() const {
    return "stateSeed: " + to_string(stateSeed_) +
        "\ncumulativeReward: " + to_string(cumulativeReward_) +
        "\ndepth: " + to_string(depth_) + '\n';
}

bool RandomGameState::operator==(const State &other) const {
    if (typeid(*this) != typeid(other)) {
        return false;
    }
    auto otherState = dynamic_cast<const RandomGameState &>(other);
    return hash_ == other.getHash()
        && stateSeed_ == otherState.stateSeed_
        && cumulativeReward_ == otherState.cumulativeReward_
        && depth_ == otherState.depth_;
}
bool RandomGameState::isTerminal() const {
    auto RGdomain = dynamic_cast<const RandomGameDomain *>(domain_);
    return this->depth_ == RGdomain->getMaxStateDepth();
}
}