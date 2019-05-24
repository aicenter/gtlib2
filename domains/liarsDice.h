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

constexpr int PLAYER_1 = 0;
constexpr int PLAYER_2 = 1;

namespace GTLib2::domains {

struct LiarsDiceSettings {
    vector<int> playersDice;
    int faces;
};

class LiarsDiceDomain : public Domain {
 public:
    LiarsDiceDomain(LiarsDiceSettings settings);
    string getInfo() const override;
    vector<Player> getPlayers() const;
    void initRootStates();
    void addToRootStates(vector<int> rolls, double baseProbability);

    inline const int getPlayerNDice(int n) const {
        return playersDice_[n];
    }

    inline const int getSumDice() const {
        return playersDice_[PLAYER_1] + playersDice_[PLAYER_2];
    }

    inline const int getFaces() const {
        return faces_;
    }

    inline const int getMaxBid() const {
        return maxBid_;
    }

 private:
    const vector<int> playersDice_;
    const int faces_;
    const int maxBid_;

    int comb(int n, int m) const;
    double calculateProbabilityForRolls(double baseProbability, vector<vector<int>> rolls) const;
};

class LiarsDiceAction : public Action {
 public:
    inline LiarsDiceAction() : Action(), roll_(false), value_(-1) {}
    explicit LiarsDiceAction(ActionId id, bool roll, int value);
    bool operator==(const Action &that) const override;
    size_t getHash() const override;
    string toString() const;

    inline int getValue() const {
        return value_;
    }

    inline bool isRoll() const {
        return roll_;
    }

 private:
    bool roll_;
    int value_;
};

class LiarsDiceState : public State {
 public:
    inline LiarsDiceState(const Domain *domain, int currentBid, int previousBid, int round,
                          int currentPlayerIndex, vector<int> rolls) :
        State(domain, hashCombine(0, currentBid, previousBid, round, currentPlayerIndex, rolls)),
        currentBid_(currentBid),
        previousBid_(previousBid),
        round_(round),
        currentPlayerIndex_(currentPlayerIndex) {
        this->rolls_ = move(rolls);
    }

    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    unsigned long countAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector<shared_ptr<Action>> &actions) const override;
    vector<Player> getPlayers() const override;
    bool isTerminal() const override;
    string toString() const override;
    bool operator==(const State &rhs) const override;
    bool isBluffCallSuccessful() const;

 private:
    int currentBid_;
    int previousBid_;
    int round_;
    int currentPlayerIndex_;
    vector<int> rolls_;
};

class LiarsDiceObservation : public Observation {
 public:
    inline LiarsDiceObservation() : Observation(), isRoll_(false), rolls_({}), bid_(-1) {}
    explicit LiarsDiceObservation(bool isRoll, vector<int> rolls, int bid);

 private:
    bool isRoll_;
    vector<int> rolls_;
    int bid_;
};

}

#endif //GTLIB2_LIARSDICE_H