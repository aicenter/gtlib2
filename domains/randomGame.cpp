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

RandomGameAction::RandomGameAction(ActionId id) : Action(id) {}

bool RandomGameAction::operator==(const Action &other) const {
    if (typeid(*this) != typeid(other)) {
        return false;
    }
    auto otherAction = dynamic_cast<const RandomGameAction &>(other);
    return this->id_ == otherAction.id_;
}

size_t RandomGameAction::getHash() const {
    return id_;
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
    maxRewardModification_(settings.maxRewardModification),
    utilityCorrelation_(settings.utilityCorrelation) {

    assert(maxDepth > 0);
    assert(maxBranchingFactor > 1);
    assert(maxDifferentObservations > 0);
    assert(maxCenterModification > 0);

    if (settings.utilityCorrelation) {
        if (binaryUtility_) {
            maxUtility_ = 1.0;
        } else {
            maxUtility_ = maxRewardModification_ * maxDepth_;
        }
    } else {
        maxUtility_ = settings.maxUtility;
    }

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
           "\nMax depth: " + to_string(maxDepth_) +
        "\nSeed: " + to_string(seed_) +
        "\nMax branching factor: " + to_string(maxBranchingFactor_) +
        "\nMax different observations: " + to_string(maxDifferentObservations_) +
        "\nMax reward modification: " + to_string(maxRewardModification_) +
        "\nMax utility: " + to_string(maxUtility_) +
        "\nUtility correlation: " + (utilityCorrelation_ ? "True" : "False") +
        "\nFixed branching factor: " + (fixedBranchingFactor_ ? "True" : "False") +
        "\nBinary utility: " + (binaryUtility_ ? "True" : "False") + "\n";
}

RandomGameState::RandomGameState(Domain *domain,
                                 int stateSeed,
                                 vector<long> histories,
                                 double cumulativeReward,
                                 unsigned int depth) :
    State(domain), stateSeed_(stateSeed), cumulativeReward_(cumulativeReward), depth_(depth) {

    playerHistories_ = move(histories);
}

vector<shared_ptr<Action>> RandomGameState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;
    auto possibleMoves = countAvailableActionsFor(player);
//    cout << "Player: " << (player == 1) << ", histories: " << playerHistories_[player]
//         << ", moves: " << possibleMoves << ", depth: "
//         << this->depth_ << endl;
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

    auto pubObs = make_shared<RandomGameObservation>(NO_OBSERVATION);
    auto player0Obs = p1Action->getId() % RGdomain->getMaxDifferentObservations();
    auto player1Obs = p0Action->getId() % RGdomain->getMaxDifferentObservations();
    vector<shared_ptr<Observation>> observations{make_shared<RandomGameObservation>(player0Obs),
                                                 make_shared<RandomGameObservation>(player1Obs)};
    vector<long> newHistories{
        playerHistories_[0] + (p0Action->getId() + 1) * 31 + (player0Obs + 1) * 17,
        playerHistories_[1] + (p1Action->getId() + 1) * 31 + (player1Obs + 1) * 17
    };

    long newStateSeed = (stateSeed_ + p0Action->getId() + p1Action->getId()) * 31 + 17;
    std::mt19937 generator(newStateSeed);
    std::uniform_real_distribution<double> cumulativeRewardDistribution
        (-RGdomain->getMaxRewardModification(), RGdomain->getMaxRewardModification());
    double newCumulativeReward = cumulativeReward_ + cumulativeRewardDistribution(generator);

    auto newState =
        make_shared<RandomGameState>(domain_,
                                     newStateSeed,
                                     newHistories,
                                     newCumulativeReward,
                                     depth_ + 1);

    if (RGdomain->isUtilityCorrelation()) {
        if (RGdomain->isBinaryUtility()) {
            // signum(newCumulativeReward)
            newCumulativeReward = (double) (0 < newCumulativeReward) - (newCumulativeReward < 0);
        }
    } else {
        std::uniform_real_distribution<double>
            rewardDistribution(RGdomain->getMinUtility(), RGdomain->getMaxUtility());
        newCumulativeReward = rewardDistribution(generator);
    }
    vector<double> rewards{newCumulativeReward, -newCumulativeReward};

    Outcome outcome(newState, observations, pubObs, rewards);
    OutcomeDistribution dist;
    dist.emplace_back(outcome, 1.0);

    return dist;
}

size_t RandomGameState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, stateSeed_);
    boost::hash_combine(seed, cumulativeReward_);
    return seed;
}

vector<Player> RandomGameState::getPlayers() const {
    auto RGdomain = dynamic_cast<RandomGameDomain *>(domain_);
    if (this->depth_ == RGdomain->getMaxDepth()) return {};
    return {0, 1};
}

string RandomGameState::toString() const {
    return "stateId: " + to_string(stateSeed_) +
        "\ncenter: " + to_string(cumulativeReward_) +
        "\ndepth: " + to_string(depth_) + '\n';
}

bool RandomGameState::operator==(const State &other) const {
    if (typeid(*this) != typeid(other)) {
        return false;
    }
    auto otherState = dynamic_cast<const RandomGameState &>(other);
    return stateSeed_ == otherState.stateSeed_ &&
        cumulativeReward_ == otherState.cumulativeReward_ &&
        depth_ == otherState.depth_;
}

RandomGameObservation::RandomGameObservation(ObservationId id) : Observation(id) {}
}