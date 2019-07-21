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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

#ifndef GTLIB2_LIARSDICE_H
#define GTLIB2_LIARSDICE_H

#include "base/base.h"

namespace GTLib2::domains {

/**
 * Liar’s Dice LD({D1,D2},F) is a dice-bidding game.
 * Each die has faces 0 to F − 1. Each player i rolls Di of these dice
 * without showing them to their opponent.
 *
 * Each round, players alternate by bidding on the outcome
 * of all dice in play until one player "calls liar”,
 * i.e. claims that their opponent’s latest bid does not hold.
 *
 * If the bid holds, the calling player loses; otherwise, she wins.
 *
 * A bid consists of a quantity of dice and a face value.
 * To bid, the player must increase either the quantity or face value of the current bid (or both).
 * All actions in this game are public. The only hiddeninformation is caused
 * by chance at the beginning of the game.
 * Therefore, the size of all information sets is identical.
 */
class LiarsDiceDomain : public Domain {

 public:
    LiarsDiceDomain(vector<int> playersDice, int faces);
    string getInfo() const override;
    vector<Player> getPlayers() const;

    inline const int getPlayerDice(Player pl) const {
        return playersDice_[pl];
    }

    inline const int getSumDice() const {
        return playersDice_[0] + playersDice_[1];
    }

    inline const int getFaces() const {
        return faces_;
    }

    inline const int getMaxBid() const {
        return maxBid_;
    }

 private:
    void initRootStates();
    void addToRootStates(vector<int> rolls, double baseProbability);

    const vector<int> playersDice_;
    const int faces_;
    const int maxBid_;

    double calculateProbabilityForRolls(double baseProbability, vector<vector<int>> rolls) const;
};

class LiarsDiceAction : public Action {
 public:
    inline LiarsDiceAction() : Action(), roll_(false), value_(-1) {}
    inline LiarsDiceAction(GTLib2::ActionId id, bool roll, int value) : Action(id),
                                                                        roll_(roll),
                                                                        value_(value) {}
    bool operator==(const Action &that) const override;
    string toString() const override;

    inline int getValue() const {
        return value_;
    }

    inline bool isRoll() const {
        return roll_;
    }

 private:
    const bool roll_;
    const int value_;
};

class LiarsDiceState : public State {
 public:
    inline LiarsDiceState(const Domain *domain, int currentBid, int previousBid, int round,
                          Player currentPlayer, vector<int> rolls) :
        State(domain, hashCombine(0, currentBid, previousBid, round, currentPlayer, rolls)),
        currentBid_(currentBid),
        previousBid_(previousBid),
        round_(round),
        currentPlayer_(currentPlayer),
        rolls_(move(rolls)) {}

    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    unsigned long countAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector<shared_ptr<Action>> &actions) const override;
    vector<Player> getPlayers() const override;
    bool isTerminal() const override;
    string toString() const override;
    bool operator==(const State &rhs) const override;

 private:
    bool isBluffCallSuccessful() const;

    const int currentBid_;
    const int previousBid_;
    const int round_;
    const int currentPlayer_;
    const vector<int> rolls_;
};

class LiarsDiceObservation : public Observation {
    /*
     * id_ supports domains that satisfy faces^rolls.size() < 2^31
     */
 public:
    inline LiarsDiceObservation() : Observation(), rolls_({}), bid_(-1) {}
    inline LiarsDiceObservation(vector<int> rolls, int faces, int bid) :
        Observation(),
        rolls_(move(rolls)),
        bid_(bid) {

        if (!rolls_.empty()) {
            unsigned int idTemp = 0;
            unsigned int jump = 1;
            for (int i = 0; i < rolls_.size(); i++) {
                idTemp += rolls_[i] * jump;
                jump *= faces;
            }
            id_ = 1 + (idTemp << 1);
        } else {
            id_ = bid_ << 1;
        }
    }
 private:
    const vector<int> rolls_;
    const int bid_;
};

}

#endif //GTLIB2_LIARSDICE_H
