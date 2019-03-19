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

#include <utility>
#include <numeric>
#include <vector>
#include <string>
#include "base/base.h"

namespace GTLib2 {
namespace domains {
struct Pos {
  int y;
  int x;
};

// Moore neighborhood for observations
static std::array<Pos, 10> pursuitEightSurrounding = {{{-2, -2}, {-1, -1}, {0, -1}, {1, -1},
                                                       {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1},
                                                       {0, 0}}};

// eight surrounding description
static std::array<string, 10> eightdes_ = {"nowhere", "top left", "top", "top right",
                                           "left", "right", "bottom left", "bottom",
                                           "bottom right", "same"};

// moves
static std::array<Pos, 5> pursuitMoves = {{{0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}}};

// moves description
static std::array<string, 5> movedes_ = {"stay", "right", "down", "left", "up"};

/**
 * PursuitAction is a class that represents pursuit actions,
 * which are identified by their id and contain where to move.
 */
class PursuitAction : public Action {
 public:
  // constructor
  PursuitAction(ActionId id, int move);

  // Returns move description.
  inline string toString() const final {
    if (id == NO_ACTION)
      return "NoA";
    return movedes_[move_];
  }

  // Returns index to array, which describes where to move.
  inline int GetMove() const {
    return move_;
  }

  bool operator==(const Action &that) const override;
  size_t getHash() const override;

 private:
  int move_;
};

/**
 * PursuitObservation is a class that represents pursuit observation,
 * which are identified by their id and
 * contain vector of mini-observations to others.
 */
class PursuitObservation : public Observation {
 public:
  // constructor
  PursuitObservation(int id, vector<int> values);

  // Returns description.
  string toString() const final;

  // Returns vector of mini-observations to others.
  inline const vector<int> &GetValues() const {
    return values_;
  }

 private:
  vector<int> values_;
};

/**
 * PursuitObservationLoc is a class that represents pursuit observation,
 * which are identified by their id and contain vector of others' locations.
 */
class PursuitObservationLoc : public Observation {
 public:
  // constructor
  PursuitObservationLoc(int id, vector<Pos> values);

  // Returns description.
  string toString() const final;

  // Returns vector of mini-observations to others (others' locations).
  inline const vector<Pos> &GetValues() const {
    return values_;
  }

 private:
  vector<Pos> values_;
};

/**
 * PursuitState is a class that represents pursuit states,
 * which contains eight surrounding, moves,
 * vector of locations of all players and state probability.
 */
class PursuitState : public State {
 public:
  // Constructor
  explicit PursuitState(Domain *domain, const vector<Pos> &p);

  // Constructor
  PursuitState(Domain *domain, const vector<Pos> &p, double prob);

  // Destructor
  ~PursuitState() override = default;

  // GetActions returns possible actions for a player in the state.
  vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;

  OutcomeDistribution
  performActions(const vector<PlayerAction> &actions2) const override;

  inline vector<Player> getPlayers() const final {
    return players_;
  }

  // ToString returns state description.
  inline string toString() const override {
    string s;
    for (auto player = 0; player < place_.size(); ++player) {
      s += "player: " + to_string(player) + ", location: " +
          to_string(place_[player].x) + " " + to_string(place_[player].y) +
          strings_[player] + "\n";
    }
    return s;
  }

  bool operator==(const State &rhs) const override;

  size_t getHash() const override;

 protected:
  vector<Pos> place_;  // locations of all players
  // eight surrounding
  vector<string> strings_;
  vector<Player> players_;
  double prob_ = 1;  // state probability
};

/**
 * MMPursuitState is a class that represents multiple-move pursuit states,
 * which contains vector of all players and a move count of player on turn.
 */
class MMPursuitState : public PursuitState {
 public:
  // Constructor
  MMPursuitState(Domain *domain, const vector<Pos> &p, const vector<Player> &players,
                 vector<int> numberOfMoves);

  // Constructor
  MMPursuitState(Domain *domain, const vector<Pos> &p, const vector<Player> &players,
                 vector<int> numberOfMoves, int currentNOM, int currentPlayer);

  // Constructor
  MMPursuitState(Domain *domain, const vector<Pos> &p, double prob,
                 const vector<Player> &players, vector<int> numberOfMoves);

  // Constructor
  MMPursuitState(Domain *domain, const vector<Pos> &p, double prob, const vector<Player> &players,
                 vector<int> numberOfMoves, int currentNOM, int currentPlayer);

  OutcomeDistribution
  performActions(const vector<PlayerAction> &actions2) const override;

  bool operator==(const State &rhs) const override;

  size_t getHash() const override;

 private:
  vector<int> numberOfMoves_;
  vector<Player> players_;
  int currentNOM_;
  int currentPlayer_;
};

/**
 * ObsPursuitState is a class that represents pursuit states,
 * which contains eight surrounding, moves,
 * vector of locations of all players and state probability.
 * A difference is that it uses PursuitObservationLoc.
 */
class ObsPursuitState : public PursuitState {
 public:
  // Constructor
  explicit ObsPursuitState(Domain *domain, const vector<Pos> &p);

  // Constructor
  ObsPursuitState(Domain *domain, const vector<Pos> &p, double prob);

  OutcomeDistribution
  performActions(const vector<PlayerAction> &actions2) const override;
};

/**
 * PursuitDomain is a class that represents pursuit domain,
 * which contain static height and static width.
 */
class PursuitDomain : public Domain {
 public:
  // constructor
  PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                const vector<Pos> &loc, int height, int width);

  PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                const vector<Pos> &loc, int height, int width, vector<double> probability);

  // constructor
  PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                const shared_ptr<MMPursuitState> &state, int height, int width);

  PursuitDomain(unsigned int max,
                unsigned int numberOfPlayers,
                const shared_ptr<MMPursuitState> &state,
                int height,
                int width,
                vector<double> probability);

  PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                const shared_ptr<ObsPursuitState> &state, int height, int width);
  PursuitDomain(unsigned int max,
                unsigned int numberOfPlayers,
                const shared_ptr<ObsPursuitState> &state,
                int height,
                int width,
                vector<double> probability);

  // constructor
  PursuitDomain(unsigned int max, int height, int width);

  // destructor
  ~PursuitDomain() override = default;

  // GetInfo returns string containing domain information.
  string getInfo() const final;

  vector<Player> getPlayers() const final;

  vector<double> probability;  // probability of stay or move
  int height;
  int width;
};

/**
 * PursuitDomainChance is a class that represents pursuit domain,
 * it starts with a ChanceNode, so it can have more first states.
 */
class PursuitDomainChance : public PursuitDomain {
 public:
  // constructor
  PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                      const vector<Pos> &firstPlayerLocation,
                      const vector<Pos> &secondPlayerLocation,
                      int height, int width);

  PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                      const vector<Pos> &firstPlayerLocation,
                      const vector<Pos> &secondPlayerLocation,
                      int height, int width, vector<double> probability);

  // constructor
  PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                      const shared_ptr<MMPursuitState> &state,
                      int height, int width);

  PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                      const shared_ptr<MMPursuitState> &state,
                      int height, int width, vector<double> probability);
};
}  // namespace domains
}  // namespace GTLib2

#endif  // DOMAINS_PURSUIT_H_

#pragma clang diagnostic pop


