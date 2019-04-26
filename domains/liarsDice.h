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
constexpr int NATURE = 2;

namespace GTLib2::domains {
    class LiarsDiceDomain : public Domain {
    public:
        LiarsDiceDomain(int p1Dice, int p2Dice, int faces);
        string getInfo() const override;

        inline const int getP1Dice() const {
            return p1Dice_;
        }

        inline const int getP2Dice() const {
            return p2Dice_;
        }

        inline  const int getNDice() const {
            return  p1Dice_ + p2Dice_;
        }

        inline const int getFaces() const {
            return faces_;
        }

        inline const int getMaxBid() const {
            return maxBid_;
        }

    private:
        const int p1Dice_;
        const int p2Dice_;
        const int faces_;
        const int maxBid_;
    };

    class LiarsDiceAction : public Action {
    public:
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
        LiarsDiceState(Domain *domain);
        LiarsDiceState(Domain *domain, int currentBid, int previousBid, int round,
                int currentPlayerIndex, vector<int> rolls);
        vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
        unsigned long countAvailableActionsFor(Player player) const override;
        OutcomeDistribution performActions(const vector<PlayerAction> &actions) const override;
        vector<Player> getPlayers() const override;
        bool isGameEnd() const;
        string toString() const override;
        bool operator==(const State &rhs) const override;
        size_t getHash() const override;

    private:
        int currentBid_;
        int previousBid_;
        int round_;
        int currentPlayerIndex_;
        vector<int> rolls_;
    };

    class LiarsDiceObservation : public Observation {
    public:
        explicit LiarsDiceObservation(bool roll, int value);

        inline bool isRoll() const {
            return roll_;
        }

        inline int getValue() const {
            return value_;
        }

    private:
        bool roll_;
        int value_;
    };

}

#endif //GTLIB2_LIARSDICE_H
