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



#include "liarsDice.h"
#include "utils/combinatorics.h"


namespace GTLib2::domains {

bool LiarsDiceAction::operator==(const Action &that) const {
    if (typeid(*this) != typeid(that)) {
        return false;
    }
    const auto otherAction = static_cast<const LiarsDiceAction *>(&that);

    if (getHash() != otherAction->getHash()) {
        return false;
    }

    return getValue() == otherAction->getValue()
        && isRoll() == otherAction->isRoll()
        && id_ == otherAction->id_;
}

string LiarsDiceAction::toString() const {
    return " roll?:" + std::to_string(roll_) +
        " value: " + std::to_string(value_);
}

LiarsDiceDomain::LiarsDiceDomain(vector<int> playersDice, int faces) :
    Domain(static_cast<unsigned int>(((playersDice[0] + playersDice[1]) * faces) + 2),
           2, true,
           make_shared<LiarsDiceAction>(),
           make_shared<LiarsDiceObservation>()),
    playersDice_(playersDice),
    faces_(faces),
    maxBid_((playersDice[0] + playersDice[1]) * faces + 1) {

    assert(getSumDice() >= 1);
    assert(faces_ >= 2);
    maxUtility_ = 1.0;
    initRootStates();
}

double LiarsDiceDomain::calculateProbabilityForRolls(double baseProbability,
                                                     std::vector<std::vector<int>> rolls) const {

    sort(rolls[0].begin(), rolls[0].end());
    sort(rolls[1].begin(), rolls[1].end());

    vector<int> combinations = {1, 1};

    int currentRoll;
    int streak;
    int remaining;

    for (int i = 0; i < 2; i++) {
        if (getPlayerDice(i) == 0) {
            combinations[i] = 1;
        } else {
            currentRoll = rolls[i][0];
            streak = 0;
            remaining = getPlayerDice(i);
            for (int roll : rolls[i]) {
                if (roll == currentRoll) {
                    streak++;
                } else {
                    combinations[i] *= utils::binomial_coefficient(remaining, streak);
                    remaining -= streak;
                    streak = 1;
                    currentRoll = roll;
                }
            }
        }
    }

    return baseProbability * combinations[0] * combinations[1];
}

void LiarsDiceDomain::addToRootStates(std::vector<int> rolls, double baseProbability) {
    vector<vector<int>> playerRolls(2);

    for (int i = 0; i < getPlayerDice(0); i++) {
        playerRolls[0].push_back(rolls[i]);
    }
    for (int j = getPlayerDice(0); j < getSumDice(); j++) {
        playerRolls[1].push_back(rolls[j]);
    }

    auto obsPl1 = playerRolls[0].empty()
                  ? noObservation_ : make_shared<LiarsDiceObservation>(playerRolls[0], faces_, -1);
    auto obsPl2 = playerRolls[1].empty()
                  ? noObservation_ : make_shared<LiarsDiceObservation>(playerRolls[1], faces_, -1);

    auto newState = make_shared<LiarsDiceState>(this, 0, 0, 0, 0, rolls);
    Outcome outcome(newState, {obsPl1, obsPl2}, noObservation_, {0.0, 0.0});
    rootStatesDistribution_.emplace_back(OutcomeEntry(outcome,
                                                      calculateProbabilityForRolls(baseProbability,
                                                                                   playerRolls)));
}

void LiarsDiceDomain::initRootStates() {
    double baseProbability = 1.0 / pow(double(faces_), double(getSumDice()));

    function<void(int, int, vector<int>)>
        backtrack = [&](int depth, int limit, vector<int> rolls) -> void {
        if (depth == getSumDice()) {
            addToRootStates(rolls, baseProbability);
        } else {
            for (int i = 0; i <= limit; i++) {
                vector<int> appendedRolls(rolls);
                appendedRolls.push_back(i);
                if (depth + 1 == getPlayerDice(0)) {
                    backtrack(depth + 1, faces_ - 1, appendedRolls);
                } else {
                    backtrack(depth + 1, i, appendedRolls);
                }
            }
        }
    };

    backtrack(0, faces_ - 1, {});
}

string LiarsDiceDomain::getInfo() const {
    return "Liars Dice"
           "\nPlayer 1 Dice: " + std::to_string(playersDice_[0]) +
        "\nPlayer 2 Dice: " + std::to_string(playersDice_[1]) +
        "\nDice faces: " + std::to_string(faces_) +
        "\nMax depth: " + std::to_string(maxStateDepth_) + '\n';
}

vector<Player> LiarsDiceDomain::getPlayers() const {
    return {0, 1};
}

vector<shared_ptr<Action>> LiarsDiceState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;

    unsigned int id = 0;
    const auto LDdomain = static_cast<const LiarsDiceDomain *>(domain_);

    if (player == currentPlayer_) {
        if (currentBid_ == 0) {
            // if first move of first turn, don't allow calling bluff
            for (int i = 1; i <= (LDdomain->getMaxBid() - 1); i++) {
                actions.push_back(make_shared<LiarsDiceAction>(id++, false, i));
            }
        } else {
            // otherwise start at next bid up to calling bluff
            for (int i = currentBid_ + 1; i <= LDdomain->getMaxBid(); i++) {
                actions.push_back(make_shared<LiarsDiceAction>(id++, false, i));
            }
        }
    }
    return actions;
}

unsigned long LiarsDiceState::countAvailableActionsFor(Player player) const {
    const auto LDdomain = static_cast<const LiarsDiceDomain *>(domain_);
    if (player != currentPlayer_) return 0;
    if (currentBid_ == 0) return LDdomain->getMaxBid() - 1;
    return LDdomain->getMaxBid() - currentBid_;
}

OutcomeDistribution
LiarsDiceState::performActions(const vector<shared_ptr<Action>> &actions) const {
    const auto LDdomain = static_cast<const LiarsDiceDomain *>(domain_);
    auto currentPlayerAction =
        dynamic_cast<LiarsDiceAction &>(*actions[currentPlayer_]);
    int newPlayer = currentPlayer_ == 0 ? 1 : 0;
    int newBid = currentPlayerAction.getValue();
    const auto newState = make_shared<LiarsDiceState>(
        LDdomain, newBid, currentBid_, round_ + 1, newPlayer, rolls_);

    const auto publicObs = make_shared<LiarsDiceObservation>(vector<int>(), -1, newBid);
    vector<double> rewards(2);

    if (newState->isTerminal()) {
        if (isBluffCallSuccessful()) {
            if (currentPlayer_ == 0) {
                rewards = {1.0, -1.0};
            } else {
                rewards = {-1.0, 1.0};
            }
        } else {
            if (currentPlayer_ == 0) {
                rewards = {-1.0, 1.0};
            } else {
                rewards = {1.0, -1.0};
            }
        }
    }

    return {OutcomeEntry(Outcome(newState, {publicObs, publicObs}, publicObs, rewards))};
}

bool LiarsDiceState::isBluffCallSuccessful() const {
    const auto LDdomain = static_cast<const LiarsDiceDomain *>(domain_);

    int desiredDiceValue = (currentBid_ - 1) % LDdomain->getFaces();
    int desiredDiceAmount = 1 + ((currentBid_ - 1) / LDdomain->getFaces());

    int actualDiceAmount = 0;

    for (int value : rolls_) {
        if (value == desiredDiceValue) {
            actualDiceAmount++;
        }
    }

    return actualDiceAmount < desiredDiceAmount;
}

vector<Player> LiarsDiceState::getPlayers() const {
    if (isTerminal()) {
        return {};
    }
    vector<Player> player;
    player.push_back(currentPlayer_);
    return player;
}

bool LiarsDiceState::isTerminal() const {
    const auto LDdomain = static_cast<const LiarsDiceDomain *>(domain_);
    return currentBid_ == LDdomain->getMaxBid();
}

string LiarsDiceState::toString() const {
    std::stringstream ret;
    ret << "Current bid: " << currentBid_ << "\n";
    ret << "Previous bid: " << previousBid_ << "\n";
    ret << "Round: " << round_ << "\n";
    ret << "Current player : " << currentPlayer_ << "\n";
    ret << "Rolls: " << rolls_ << "\n";
    return ret.str();
}

bool LiarsDiceState::operator==(const GTLib2::State &rhs) const {
    const auto otherState = static_cast<const LiarsDiceState *>(&rhs);

    return getHash() == otherState->getHash()
        & currentBid_ == otherState->currentBid_
        & previousBid_ == otherState->previousBid_
        & round_ == otherState->round_
        & currentPlayer_ == otherState->currentPlayer_
        & rolls_ == otherState->rolls_;
}
}
