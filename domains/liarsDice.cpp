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

        //TODO : create LD state and LD observations and implement rest of function

    }

    string LiarsDiceDomain::getInfo() const {
        return "Liars Dice"
               "\nPlayer 1 Dice: " + std::to_string(p1Dice_) +
               "\nPlayer 2 Dice: " + std::to_string(p2Dice_) +
               "\nDice faces: " + std::to_string(faces_) +
               "\nMax depth: " + std::to_string(maxDepth_) + '\n';
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
        const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);
        if (player == NATURE) { //NATURE
            if (round_ < LDdomain->getP1Dice() + LDdomain->getP2Dice()) {
                for (int i = 0; i < LDdomain->getFaces(); i++) { // adds all possible rolls
                    actions.push_back(make_shared<LiarsDiceAction>(LDdomain->getMaxBid() + i, true, i));
                }
            }
        } else if (player == this->currentPlayerIndex_) { // PLAYERS
            if (round_ >= LDdomain->getP1Dice() + LDdomain->getP2Dice()) {
                if (this->currentBid_ == 0) {
                    // if first move of first turn, don't allow calling bluff
                    for (int i = 1; i <= (LDdomain->getMaxBid() - 1); i++) {
                        actions.push_back(make_shared<LiarsDiceAction>(i - 1, false, i));
                    }
                } else {
                    // otherwise start at next bid up to calling bluff
                    for (int i = this->currentBid_ + 1; i <= LDdomain->getMaxBid(); i++) {
                        actions.push_back(make_shared<LiarsDiceAction>(i - 1, false, i));
                    }
                }
            }
        }
        return actions;
    }

    unsigned long LiarsDiceState::countAvailableActionsFor(Player player) const {
        const auto LDdomain = static_cast<LiarsDiceDomain *>(domain_);
        if (player == NATURE) {
            return LDdomain->getFaces();
        }
        if (this->currentBid_ == 0) {
            return LDdomain->getMaxBid() - 1;
        } else {
            return LDdomain->getMaxBid() - this->currentBid_;
        }
    }

    LiarsDiceObservation::LiarsDiceObservation(int id, bool roll, int value) :
            Observation(id),
            roll_(roll),
            value_(value) {}

}