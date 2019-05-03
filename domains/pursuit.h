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
#ifndef DOMAINS_PURSUIT_H_
#define DOMAINS_PURSUIT_H_

#include "base/base.h"
#include <utility>
#include <numeric>
#include <vector>
#include <string>

namespace GTLib2::domains {

struct Pos {
    int y;
    int x;

    inline bool operator==(const Pos &rhs) const {
        return x == rhs.x && y == rhs.y;
    }
    inline HashType getHash() const { return hashCombine(12315461278641, x, y); }
};

// Moore neighborhood for observations
static array<Pos, 10> pursuitEightSurrounding = {{{-2, -2}, {-1, -1}, {0, -1}, {1, -1},
                                                       {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1},
                                                       {0, 0}}};

// eight surrounding description
static array<string, 10> eightdes_ = {"nowhere",
                                           "top left", "top", "top right",
                                           "left", "right",
                                           "bottom left", "bottom", "bottom right",
                                           "same"};

// moves
static array<Pos, 5> pursuitMoves = {{{0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}}};

// moves description
static array<string, 5> movedes_ = {"stay", "right", "down", "left", "up"};

/**
 * PursuitAction is a class that represents pursuit actions,
 * which are identified by their id and contain where to move.
 */
class PursuitAction: public Action {
 public:
    PursuitAction(ActionId id, int move);
    inline string toString() const final {
        if (id_ == NO_ACTION) return "NoA";
        return movedes_[move_];
    }
    // Returns index to array, which describes where to move.
    inline int GetMove() const { return move_; }

    bool operator==(const Action &that) const override;
    HashType getHash() const override;

 private:
    const int move_;
};

/**
 * PursuitObservation is a class that represents pursuit observation,
 * which are identified by their id and
 * contain vector of mini-observations to others.
 */
class PursuitObservation: public Observation {
 public:
    PursuitObservation(int id, vector<int> values);
    string toString() const final;
    // Returns vector of mini-observations to others.
    inline const vector<int> &GetValues() const { return values_; }

 private:
    const vector<int> values_;
};

/**
 * PursuitObservationLoc is a class that represents pursuit observation,
 * which are identified by their id and contain vector of others' locations.
 */
class PursuitObservationLoc: public Observation {
 public:
    PursuitObservationLoc(int id, vector <Pos> values);
    string toString() const final;
    // Returns vector of mini-observations to others (others' locations).
    inline const vector <Pos> &GetValues() const { return values_; }

 private:
    const vector <Pos> values_;
};

/**
 * PursuitState is a class that represents pursuit states,
 * which contains eight surrounding, moves,
 * vector of locations of all players and state probability.
 */
class PursuitState: public State {
 public:
    explicit PursuitState(const Domain *domain, const vector <Pos> &place);
    PursuitState(const Domain *domain, const vector <Pos> &place, double prob);
    ~PursuitState() override = default;

    unsigned long countAvailableActionsFor(Player player) const override;
    vector <shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector <PlayerAction> &actions) const override;
    inline vector <Player> getPlayers() const final { return players_; }
    inline bool isTerminal() const override { return players_.empty(); };

    inline string toString() const override;
    bool operator==(const State &rhs) const override;

 protected:
    const vector <Pos> place_;  // locations of all players
    // eight surrounding
    vector <Player> players_;
    const double prob_ = 1;  // state probability
};

/**
 * MMPursuitState is a class that represents multiple-move pursuit states,
 * which contains vector of all players and a move count of player on turn.
 */
class MMPursuitState: public PursuitState {
 public:
    MMPursuitState(const Domain *domain, const vector <Pos> &p, const vector <Player> &players,
                   vector<int> numberOfMoves);

    MMPursuitState(const Domain *domain, const vector <Pos> &p, const vector <Player> &players,
                   vector<int> numberOfMoves, int currentNOM, int currentPlayer);

    MMPursuitState(const Domain *domain, const vector <Pos> &p, double prob,
                   const vector <Player> &players, vector<int> numberOfMoves);

    MMPursuitState(const Domain *domain, const vector <Pos> &p, double prob,
                   const vector <Player> &players, vector<int> numberOfMoves,
                   int currentNOM, int currentPlayer);

    OutcomeDistribution performActions(const vector <PlayerAction> &actions) const override;

    bool operator==(const State &rhs) const override;

 private:
    const vector<int> numberOfMoves_;
    const vector <Player> players_;
    const int currentNOM_;
    const int currentPlayer_;
};

/**
 * ObsPursuitState is a class that represents pursuit states,
 * which contains eight surrounding, moves,
 * vector of locations of all players and state probability.
 * A difference is that it uses PursuitObservationLoc.
 */
class ObsPursuitState: public PursuitState {
 public:
    explicit ObsPursuitState(const Domain *domain, const vector <Pos> &p);
    ObsPursuitState(const Domain *domain, const vector <Pos> &p, double prob);
    OutcomeDistribution performActions(const vector <PlayerAction> &actions) const override;
};

/**
 * PursuitDomain is a class that represents pursuit domain,
 * which contain static height and static width.
 */
class PursuitDomain: public Domain {
 public:
    PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                  const vector <Pos> &loc, int height, int width);

    PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                  const vector <Pos> &loc, int height, int width, vector<double> probability);

    PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                  const shared_ptr <MMPursuitState> &state, int height, int width);

    PursuitDomain(unsigned int max,
                  unsigned int numberOfPlayers,
                  const shared_ptr <MMPursuitState> &state,
                  int height,
                  int width,
                  vector<double> probability);

    PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                  const shared_ptr <ObsPursuitState> &state, int height, int width);
    PursuitDomain(unsigned int max,
                  unsigned int numberOfPlayers,
                  const shared_ptr <ObsPursuitState> &state,
                  int height,
                  int width,
                  vector<double> probability);

    PursuitDomain(unsigned int max, int height, int width);

    ~PursuitDomain() override = default;

    string getInfo() const final;
    vector <Player> getPlayers() const final;

    vector<double> probability_;  // probability of stay or move
    int height_;
    int width_;
};

/**
 * PursuitDomainChance is a class that represents pursuit domain,
 * it starts with a ChanceNode, so it can have more first states.
 */
class PursuitDomainChance: public PursuitDomain {
 public:
    PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                        const vector <Pos> &firstPlayerLocation,
                        const vector <Pos> &secondPlayerLocation,
                        int height, int width);

    PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                        const vector <Pos> &firstPlayerLocation,
                        const vector <Pos> &secondPlayerLocation,
                        int height, int width, vector<double> probability);

    PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                        const shared_ptr <MMPursuitState> &state,
                        int height, int width);

    PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                        const shared_ptr <MMPursuitState> &state,
                        int height, int width, vector<double> probability);
};
}  // namespace GTLib2

#endif  // DOMAINS_PURSUIT_H_

#pragma clang diagnostic pop


