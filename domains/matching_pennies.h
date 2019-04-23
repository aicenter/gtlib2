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
#ifndef DOMAINS_MATCHING_PENNIES_H_
#define DOMAINS_MATCHING_PENNIES_H_

#include "base/base.h"
#include <utility>
#include <vector>
#include <string>

namespace GTLib2::domains {

/**
 * Should the States be represented as both players moving (SimultaneousMoves)
 * or there should be one new state per player's move? (AlternatingMoves)
 */
enum MatchingPenniesVariant { SimultaneousMoves, AlternatingMoves };

class MatchingPenniesDomain: public Domain {
 public:
    MatchingPenniesDomain(MatchingPenniesVariant variant);
    vector <Player> getPlayers() const override { return {0, 1}; };
    string getInfo() const override {
        return variant_ == SimultaneousMoves
               ? "Simultaneous matching pennies"
               : "Alternating matching pennies";
    }
    const MatchingPenniesVariant variant_;

};

typedef ActionId Move;
// actions
constexpr Move Heads = 0;
constexpr Move Tails = 1;
// priv obs
constexpr ObservationId OtherHeads = 0;
constexpr ObservationId OtherTails = 1;
// pub obs
constexpr ObservationId Pl0Wins = 1;
constexpr ObservationId Pl1Wins = 0;

class MatchingPenniesAction: public Action {
 public:
    explicit MatchingPenniesAction(Move moveParm);
    string toString() const override;
    bool operator==(const Action &that) const override;
    size_t getHash() const override;
    Move move_;
};

class MatchingPenniesObservation: public Observation {
 public:
    explicit MatchingPenniesObservation(ObservationId id);
};

class MatchingPenniesState: public State {
 public:
    MatchingPenniesState(Domain *domain, array<Move, 2> moves);

    unsigned long countAvailableActionsFor(Player player) const override;
    vector <shared_ptr<Action>> getAvailableActionsFor(Player pl) const override;

    OutcomeDistribution performActions(const vector <PlayerAction> &actions) const override;

    inline int getNumberOfPlayers() const override { return int(players_.size()); }
    inline vector <Player> getPlayers() const override { return players_; };
    bool operator==(const State &rhs) const override;
    size_t getHash() const override;

    vector <Player> players_;
    array<Move, 2> moves_;
    MatchingPenniesVariant variant_;

    string toString() const override;
};

}  // namespace GTLib2
#endif  // DOMAINS_MATCHING_PENNIES_H_

#pragma clang diagnostic pop
