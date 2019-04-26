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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2::domains {

LiarsDiceAction::LiarsDiceAction(GTLib2::ActionId id, bool roll, int value) : Action(id),
                                                                              roll_(roll),
                                                                              value_(value) {}

bool LiarsDiceAction::operator==(const Action &that) const {
    if (typeid(*this) != typeid(that)) {
        return false;
    }
    const auto otherAction = static_cast<const LiarsDiceAction *>(&that);

    return (this->getValue() == otherAction->getValue() && this->isRoll() == otherAction->isRoll()
        && this->id_ == otherAction->id_);
}

string LiarsDiceAction::toString() const {
    return "id: " + std::to_string(id_) +
        " roll?:" + std::to_string(roll_) +
        " value: " + std::to_string(value_);
}

size_t LiarsDiceAction::getHash() const {
    std::hash<size_t> h;
    return h(id_);
}

LiarsDiceDomain::LiarsDiceDomain(vector<int> playersDice, int faces) :
    playersDice_(playersDice),
    Domain(static_cast<unsigned int>(((getSumDice()) * faces) + 1), 2),
    faces_(faces),
    maxBid_((getSumDice()) * faces + 1) {

    assert(getSumDice() >= 1);
    assert(faces >= 2);
    maxUtility_ = 1.0;

    maxUtility_ = 1.0;

    initRootStates();
}

vector<shared_ptr<Observation>> LiarsDiceDomain::createInitialObservations(std::vector<int> rolls) const {
    vector<vector<int>> playerRolls(2);

    for(int i = 0 ; i < getPlayerNDice(PLAYER_1); i++){
        playerRolls[PLAYER_1].push_back(rolls[i]);
    }
    for(int j = getPlayerNDice(PLAYER_1) ; j < getSumDice(); j++){
        playerRolls[PLAYER_2].push_back(rolls[j]);
    }
    vector<shared_ptr<Observation>> observations{make_shared<LiarsDiceObservation>(true, playerRolls[PLAYER_1], -1),
                                                 make_shared<LiarsDiceObservation>(true, playerRolls[PLAYER_2], -1)};
    return observations;
}

void LiarsDiceDomain::initRootStates() {
    double probability = 1 / (faces_ ^ getSumDice());

    function<void(int, vector<int>)> backtrack = [&](int depth, vector<int> rolls) -> void {
        if (depth == 0) {
            auto newState = make_shared<LiarsDiceState>(this, 0, 0, 0, PLAYER_1, rolls);
            Outcome outcome(newState, createInitialObservations(rolls), shared_ptr<Observation>(), {0.0, 0.0});
            rootStatesDistribution_.emplace_back(outcome, probability);
        } else {
            for (int i = 0; i < depth; i++) {
                vector<int> appendedRolls(rolls);
                appendedRolls.push_back(i);
                backtrack(depth - 1, appendedRolls);
            }
        }
    };
}

string LiarsDiceDomain::getInfo() const {
    return "Liars Dice"
           "\nPlayer 1 Dice: " + std::to_string(playersDice_[PLAYER_1]) +
        "\nPlayer 2 Dice: " + std::to_string(playersDice_[PLAYER_2]) +
        "\nDice faces: " + std::to_string(faces_) +
        "\nMax depth: " + std::to_string(maxDepth_) + '\n';
}

vector<Player> LiarsDiceDomain::getPlayers() const {
    return {PLAYER_1, PLAYER_2};
}

LiarsDiceState::LiarsDiceState(Domain *domain, Player player) :
    LiarsDiceState(domain, 0, 0, 0, player, {}) {}

LiarsDiceState::LiarsDiceState(Domain *domain, int currentBid, int previousBid, int round,
                               int currentPlayerIndex, vector<int> rolls) :
    State(domain),
    currentBid_(currentBid),
    previousBid_(previousBid),
    round_(round),
    currentPlayerIndex_(currentPlayerIndex) {
    this->rolls_ = move(rolls);
}

vector<shared_ptr<Action>> LiarsDiceState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;

    unsigned int id = 0;

    const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);

    if (player == this->currentPlayerIndex_) {
        if (this->currentBid_ == 0) {
            // if first move of first turn, don't allow calling bluff
            for (int i = 1; i <= (LDdomain->getMaxBid() - 1); i++) {
                actions.push_back(make_shared<LiarsDiceAction>(id++, false, i));
            }
        } else {
            // otherwise start at next bid up to calling bluff
            for (int i = this->currentBid_ + 1; i <= LDdomain->getMaxBid(); i++) {
                actions.push_back(make_shared<LiarsDiceAction>(id++, false, i));
            }
        }
    }
    return actions;
}

unsigned long LiarsDiceState::countAvailableActionsFor(Player player) const {
    const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);
    if (!(player == currentPlayerIndex_)) {
        return 0;
    }

    if (currentBid_ == 0) {
        return LDdomain->getMaxBid() - 1;
    } else {
        return LDdomain->getMaxBid() - currentBid_;
    }
}

OutcomeDistribution LiarsDiceState::performActions(const std::vector<GTLib2::PlayerAction> &actions) const {
    auto p1Action = dynamic_cast<LiarsDiceAction *>(actions[0].second.get());
    auto p2Action = dynamic_cast<LiarsDiceAction *>(actions[1].second.get());

    const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);

    OutcomeDistribution newOutcome;

    assert(p1Action == nullptr | p2Action == nullptr);
    assert(p1Action != nullptr
               | p2Action != nullptr); //exactly one player performs action each move

    auto currentPlayerAction =
        dynamic_cast<LiarsDiceAction *>(actions[currentPlayerIndex_].second.get());

    int newPlayer = currentPlayerIndex_ == PLAYER_1 ? PLAYER_2 : PLAYER_1;

    int newBid = currentPlayerAction->getValue();

    const auto newState = make_shared<LiarsDiceState>(LDdomain,
                                                      newBid,
                                                      currentBid_,
                                                      round_ + 1,
                                                      newPlayer,
                                                      rolls_); // TODO: ask if appropriate to pass vector object from other State object

    const auto publicObs = make_shared<LiarsDiceObservation>(false, vector<int>() , newBid);

    vector<double> rewards;

    if (newState->isGameOver()) {
        if (isBluffCallSuccessful()) {
            if (currentPlayerIndex_ == PLAYER_1) {
                rewards = {1.0, -1.0, 0.0};
            } else {
                rewards = {-1.0, 1.0, 0.0};
            }
        } else {
            if (currentPlayerIndex_ == PLAYER_1) {
                rewards = {-1.0, 1.0};
            } else {
                rewards = {1.0, -1.0};
            }
        }
    } else {
        rewards = {0.0, 0.0};
    }

    const auto
        outcome = Outcome(newState, {publicObs, publicObs}, publicObs, rewards);
    newOutcome.emplace_back(outcome, 1.0);

    return newOutcome;

}

bool LiarsDiceState::isBluffCallSuccessful() const {
    const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);

    int desiredDiceValue = (previousBid_ - 1) % LDdomain->getFaces();
    int desiredDiceAmount = 1 + ((previousBid_ - 1) / LDdomain->getFaces());

    int actualDiceAmount = 0;

    for (int value : rolls_) {
        if (value == desiredDiceValue) {
            actualDiceAmount++;
        }
    }

    return actualDiceAmount >= desiredDiceAmount;
}

vector<Player> LiarsDiceState::getPlayers() const {
    if (isGameOver()) {
        return {};
    }
    vector<Player> player;
    player.push_back(currentPlayerIndex_);
    return player;
}

bool LiarsDiceState::isGameOver() const {
    const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);
    return (currentBid_ == LDdomain->getMaxBid());
}

string LiarsDiceState::toString() const {
    string ret;
    ret.append("Current bid: " + to_string(currentBid_) + "\n");
    ret.append("Previous bid: " + to_string(previousBid_) + "\n");
    ret.append("Round: " + to_string(round_) + "\n");
    ret.append("Current player : " + to_string(currentPlayerIndex_) + "\n");
    ret.append("Rolls: ");
    for (int i = 0; i < rolls_.size(); i++) {
        ret.append("roll #" + to_string(i + 1) + " = " + to_string(rolls_[i]) + " ");
    }
    ret.append("\n");
    return ret;
}

bool LiarsDiceState::operator==(const GTLib2::State &rhs) const {
    const auto otherState = static_cast<const LiarsDiceState *>(&rhs);
    return (currentBid_ == otherState->currentBid_
        & previousBid_ == otherState->previousBid_
        & round_ == otherState->round_
        & currentPlayerIndex_ == otherState->currentPlayerIndex_
        & rolls_ == otherState->rolls_);
}

size_t LiarsDiceState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, currentBid_);
    boost::hash_combine(seed, previousBid_);
    boost::hash_combine(seed, round_);
    boost::hash_combine(seed, currentPlayerIndex_);
    boost::hash_combine(seed, rolls_);
    return seed;
}

LiarsDiceObservation::LiarsDiceObservation(bool isRoll, vector<int> rolls, int bid) :
    Observation(),
    isRoll_(isRoll),
    rolls_(rolls),
    bid_(bid) {

    if (isRoll_) {
        int shift = 1;
        int idTemp = 1;
        for (vector<int>::iterator it = rolls_.begin(); it != rolls_.end(); it++) {
            idTemp += (*it << shift++);
        }
        id_ = idTemp;
    } else {
        id_ = bid << 1;
    }
}

}