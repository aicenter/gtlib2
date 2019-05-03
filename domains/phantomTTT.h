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
#ifndef DOMAINS_PHANTOMTTT_H_
#define DOMAINS_PHANTOMTTT_H_

#include "base/base.h"
#include <utility>
#include <vector>
#include <string>

namespace GTLib2::domains {

const array<string, 9> phantomMoves = {"top left", "top", "top right",
                                       "left", "center", "right",
                                       "bottom left", "bottom", "bottom right"};

/**
 * PhantomTTTAction is a class that represents PhantomTTT actions,
 * which are identified by their id and contain where to play.
 */
class PhantomTTTAction: public Action {
 public:
    PhantomTTTAction(ActionId id, int move);
    inline string toString() const final {
        if (id_ == NO_ACTION) return "NoA";
        return phantomMoves[move_];
    }

    bool operator==(const Action &that) const override;
    HashType getHash() const override;

    /**
     * Returns index to 3x3 array (represented as array of 9 elements),
     * which describes where to move.
     */
    inline int GetMove() const { return move_; }

 private:
    int move_;
};

/**
 * PhantomTTTObservation is a class that represents PhantomTTT observations,
 * which are identified by their id and contain an integer value
 * indicating if an action was successful(1) or not (0).
 */
class PhantomTTTObservation: public Observation {
 public:
    explicit PhantomTTTObservation(int id);
    inline string toString() const final {
        if (id_ == 1) return "Success";
        return "Failure";
    };
};

/**
 * PhantomTTTState is a class that represents PhantomTTT states,
 * which contains players' board - what they can see,
 * and who can play in the turn.
 */
class PhantomTTTState: public State {
 public:
    inline PhantomTTTState(Domain *domain, vector <vector<int>> board, vector <Player> players)
        : State(domain, hashCombine(324816846515, board, players)),
          board_(move(board)),
          players_(move(players)) {}
    ~PhantomTTTState() override = default;

    unsigned long countAvailableActionsFor(Player player) const override;
    vector <shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector <PlayerAction> &actions) const override;
    inline vector <Player> getPlayers() const final { return players_; }

    bool operator==(const State &rhs) const override;
    inline string toString() const override;

 protected:
    const vector <vector<int>> board_;
    const vector <Player> players_;
};

/**
 * PhantomTTTDomain is a class that represents PhantomTTT domain,
 * which contain static height and static width.
 */
class PhantomTTTDomain: public Domain {
 public:
    explicit PhantomTTTDomain(unsigned int max);
    ~PhantomTTTDomain() override = default;
    string getInfo() const final;
    inline vector <Player> getPlayers() const final { return {Player(0), Player(1)}; }
};
}
#endif  // DOMAINS_PHANTOMTTT_H_

#pragma clang diagnostic pop
