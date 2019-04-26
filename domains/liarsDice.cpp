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

    LiarsDiceDomain::LiarsDiceDomain(int p1Dice, int p2Dice, int faces) :
            Domain(static_cast<unsigned int>(((p1Dice + p2Dice) * (1 + faces)) + 1), 3),
            p1Dice_(p1Dice),
            p2Dice_(p2Dice),
            faces_(faces),
            maxBid_((p1Dice + p2Dice) * faces + 1) {

        assert(p1Dice + p2Dice >= 1);
        assert(faces >= 2);
        maxUtility_ = 1.0;

         maxUtility_ = 1.0;
         auto rootState = make_shared<LiarsDiceState>(this);
         vector<shared_ptr<Observation>> observations{make_shared<Observation>(),
                                                      make_shared<Observation>(),
                                                      make_shared<Observation>()};
         vector<double> rewards{0.0, 0.0, 0.0};
         Outcome outcome(rootState, observations, shared_ptr<Observation>(), rewards);

         rootStatesDistribution_.emplace_back(outcome, 1.0);

    }

    string LiarsDiceDomain::getInfo() const {
        return "Liars Dice"
               "\nPlayer 1 Dice: " + std::to_string(p1Dice_) +
               "\nPlayer 2 Dice: " + std::to_string(p2Dice_) +
               "\nDice faces: " + std::to_string(faces_) +
               "\nMax depth: " + std::to_string(maxDepth_) + '\n';
    }

    vector<Player> LiarsDiceDomain::getPlayers() const {
        return {PLAYER_1, PLAYER_2, NATURE};
    }


    LiarsDiceState::LiarsDiceState(Domain *domain) :
            LiarsDiceState(domain, 0, 0, 0, NATURE, {}) {}


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
        if (player == NATURE) { //NATURE
            if (round_ < LDdomain->getNDice()) {
                for (int i = 0; i < LDdomain->getFaces(); i++) { // adds all possible rolls
                    actions.push_back(make_shared<LiarsDiceAction>(id++, true, i));
                }
            }
        } else if (player == this->currentPlayerIndex_) { // PLAYERS
            if (round_ >= LDdomain->getNDice()) {
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
        }
        return actions;
    }

    unsigned long LiarsDiceState::countAvailableActionsFor(Player player) const {
        const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);
        if(!(player == currentPlayerIndex_)){
            return 0;
        }

        if (player == NATURE) {
            return LDdomain->getFaces();
        } else {
            if (currentBid_ == 0) {
                return LDdomain->getMaxBid() - 1;
            } else {
                return LDdomain->getMaxBid() - currentBid_;
            }
        }
    }

    OutcomeDistribution LiarsDiceState::performActions(const std::vector<GTLib2::PlayerAction> &actions) const {
        auto p1Action = dynamic_cast<LiarsDiceAction *>(actions[0].second.get());
        auto p2Action = dynamic_cast<LiarsDiceAction *>(actions[1].second.get());
        auto natureAction = dynamic_cast<LiarsDiceAction *>(actions[2].second.get());

        const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);

        OutcomeDistribution newOutcome;
        shared_ptr<Observation> noObs = make_shared<Observation>();
        shared_ptr<LiarsDiceObservation>publicObs;

        if(currentPlayerIndex_ == NATURE){
            assert(natureAction != nullptr & p1Action == nullptr & p2Action == nullptr);

            int newPlayer = round_ + 1 < LDdomain->getNDice() ? NATURE : PLAYER_1;

            int rollValue = natureAction->getValue();
            vector<int> newRolls(rolls_);
            newRolls.push_back(rollValue);

            const auto newState = make_shared<LiarsDiceState>(LDdomain, 0, 0, round_ + 1, newPlayer, newRolls);

            shared_ptr<LiarsDiceObservation> playerObs = make_shared<LiarsDiceObservation>(true, rollValue);

            vector<shared_ptr<Observation>> privateObservations;

            if(round_ < LDdomain->getP1Dice()){
                privateObservations = {playerObs, noObs, noObs}; //TODO: ask if objects are copied or its only object reference in the vector
            }
            else{
               privateObservations = {noObs, playerObs, noObs};
            }

            const auto outcome = Outcome(newState, privateObservations , noObs, {0.0, 0.0, 0.0});
            newOutcome.emplace_back(outcome, 1.0);
        }

        else {
            assert(natureAction == nullptr);
            assert(p1Action == nullptr | p2Action == nullptr);
            assert(p1Action != nullptr | p2Action != nullptr); //exactly one player performs action each move

            auto currentPlayerAction = dynamic_cast<LiarsDiceAction *>(actions[currentPlayerIndex_].second.get());

            int newPlayer = currentPlayerIndex_ == PLAYER_1 ? PLAYER_2 : PLAYER_1;

            int newBid = currentPlayerAction->getValue();

            const auto newState = make_shared<LiarsDiceState>(LDdomain, newBid, currentBid_, round_ + 1, newPlayer, rolls_); // TODO: ask if appropriate to pass vector object from other State object

            publicObs = make_shared<LiarsDiceObservation>(false, newBid);

            vector<double> rewards;

            if(newState->isGameOver()){
                if(isBluffCallSuccessful()){
                    if(currentPlayerIndex_ == PLAYER_1){
                        rewards = {1.0, -1.0, 0.0};
                    }
                    else{
                        rewards = {-1.0, 1.0, 0.0};
                    }
                }
                else{
                    if(currentPlayerIndex_ == PLAYER_1){
                        rewards = {-1.0, 1.0, 0.0};
                    }
                    else{
                        rewards = {1.0, -1.0, 0.0};
                    }
                }
            }
            else{
                rewards = {0.0, 0.0, 0.0};
            }

            const auto outcome = Outcome(newState, {publicObs, publicObs, publicObs} , publicObs, rewards);
            newOutcome.emplace_back(outcome, 1.0);
        }

        return newOutcome;

    }

    bool LiarsDiceState::isBluffCallSuccessful() const {
        const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);

        int desiredDiceValue = (previousBid_ - 1) % LDdomain->getFaces();
        int desiredDiceAmount = 1 + ((previousBid_ - 1) / LDdomain->getFaces());

        int actualDiceAmount = 0;

        for(int value : rolls_){
            if(value == desiredDiceValue){
                actualDiceAmount++;
            }
        }

        return actualDiceAmount >= desiredDiceAmount;
    }

    vector<Player> LiarsDiceState::getPlayers() const {
        if(isGameOver()){
            return {};
        }
        vector<Player > player;
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
        for(int i = 0; i < rolls_.size(); i++){
            ret.append("roll #" + to_string(i + 1) + " = " + to_string(rolls_[i]) + " ");
        }
        ret.append("\n");
        return  ret;
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

    LiarsDiceObservation::LiarsDiceObservation(bool roll, int value) :
            Observation(),
            roll_(roll),
            value_(value) {

        id_ = roll_ ? 1 | value << 1 : value << 1;
    }

}