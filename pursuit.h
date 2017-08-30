//
// Created by rozliv on 02.08.2017.
//

#ifndef PURSUIT_H_
#define PURSUIT_H_

#include <utility>

#include "base.h"

// moves description
const vector<string> movedes_ = {"stay", "right", "down", "left", "up"};

// eight surrounding description
const vector<string> eightdes_ = {"nowhere", "top left", "top", "top right",
                                  "left", "right", "bottom left", "bottom",
                                  "bottom right", "same"};

/**
 * PursuitAction is a class that represents pursuit actions,
 * which are identified by their id and contain where to move.  */
class PursuitAction: public Action {
 public:
  // constructor
  PursuitAction(int id, int move);

  // Returns move description.
  inline string ToString() final {
    if (id_ == -1)
      return "NoA";
    return movedes_[move_];
  }

  // Returns index to array, which describes where to move.
  inline int GetMove() const {
    return move_;
  }
 private:
  int move_;
};


/**
 * PursuitObservation is a class that represents pursuit observation,
 * which are identified by their id and
 * contain vector of mini-observations to others.  */
class PursuitObservation: public Observation {
 public:
  // constructor
  PursuitObservation(int id, vector<int> values);

  // Returns description.
  string ToString() final;

  // Returns vector of mini-observations to others.
  inline const vector<int>& GetValues() const {
    return values_;
  }

 private:
  vector<int> values_;
};

/**
 * PursuitObservationLoc is a class that represents pursuit observation,
 * which are identified by their id and contain vector of others' locations. */
class PursuitObservationLoc: public Observation {
 public:
  // constructor
  PursuitObservationLoc(int id, vector<Pos> values);

  // Returns description.
  string ToString() final;

  // Returns vector of mini-observations to others (others' locations).
  inline const vector<Pos>& GetValues() const {
    return values_;
  }

 private:
  vector<Pos> values_;
};


/**
 * PursuitState is a class that represents pursuit states,
 * which contains eight surrounding, moves,
 * vector of locations of all players and state probability.  */
class PursuitState: public State {
 public:
  // Constructor
  explicit PursuitState(const vector<Pos> &p);

  // Constructor
  PursuitState(const vector<Pos> &p, double prob);

  // Destructor
  ~PursuitState() override = default;

  // GetActions returns possible actions for a player in the state.
  vector<shared_ptr<Action>> GetActions(int player) override;

  // GetActions returns possible actions for a player in the state.
  void GetActions(vector<shared_ptr<Action>>& list, int player) const override;

  // PerformAction performs actions for all players who can play in the state.
  ProbDistribution PerformAction(const vector<shared_ptr<Action>>& actions) override;

  // GetPlace returns locations of all players.
  inline const vector<Pos>& GetPlace() const {
    return place_;
  }

  // GetNumPlayers returns number of players who can play in this state.
  inline const int GetNumPlayers() const override {
    return numplayers_ == 0? place_.size() : numplayers_;
  }

  // GetPlayers returns who can play in this state.
  inline const vector<bool> GetPlayers() const override {
    return players_;
  }

  // AddString adds string s to a string in vector of strings.
  inline void AddString(const string& s, int player) override {
    strings_[player].append(s);
  }

  // ToString returns state description.
  inline string ToString(int player) override {
    return  "player: " + to_string(player) +  ", location: " +
        to_string(place_[player].x) + " " + to_string(place_[player].y) +
        strings_[player] + "\n";
  }

  // GetLast returns vector of actions' and observations' id from last state.
  inline const vector<int>& GetLast() const override {
    return obs_;
  }

 protected:
  vector<int> obs_;
  unsigned int numplayers_ = 0;
  vector<Pos> place_;  // locations of all players
  double prob_ = 1;  // state probability
  // eight surrounding
  vector<Pos> eight_ = {{-2, -2}, {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                        {0, 1}, {1, -1}, {1, 0}, {1, 1}, {0, 0}};
  vector<Pos> m_ = {{0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}};  // moves
  vector<double> probdis_ = {0.1, 0.9};  // TODO(rozlijak): temporary
  vector<string> strings_;
  vector<bool> players_;
};


/**
 * MMPursuitState is a class that represents multiple-move pursuit states,
 * which contains vector of all players and a move count of player on turn. */
class MMPursuitState: public PursuitState {
 public:
  // Constructor
  MMPursuitState(const vector<Pos> &p, const vector<bool>& players,
                 int movecount);

  // Constructor
  MMPursuitState(const vector<Pos> &p, double prob,
                 const vector<bool>& players, int movecount);

  // PerformAction performs actions for all players who can play in the state.
  ProbDistribution PerformAction(const vector<shared_ptr<Action>>& actions) override;

  // GetNumPlayers returns number of players who can play in this state.
  inline const int GetNumPlayers() const override;

  // GetPlayers returns who can play in this state.
  inline const vector<bool> GetPlayers() const override {
    return players_;
  }

 private:
  vector<bool> players_;
  int movecount_;
};


/**
 * ObsPursuitState is a class that represents pursuit states,
 * which contains eight surrounding, moves,
 * vector of locations of all players and state probability.
 * A difference is that it uses PursuitObservationLoc. */
class ObsPursuitState: public PursuitState {
 public:
  // Constructor
  explicit ObsPursuitState(const vector<Pos> &p);

  // Constructor
  ObsPursuitState(const vector<Pos> &p, double prob);

  // PerformAction performs actions for all players who can play in the state.
  ProbDistribution PerformAction(const vector<shared_ptr<Action>>& actions) override;
};

/**
 * PursuitDomain is a class that represents pursuit domain,
 * which contain static height and static width.  */
class PursuitDomain: public Domain{
 public:
  // constructor
  PursuitDomain(unsigned int max, unsigned int maxplayers,
                const vector<Pos> &loc);

  // constructor
  explicit PursuitDomain(unsigned int max);

  // destructor
  ~PursuitDomain() override = default;

  // GetInfo returns string containing domain information.
  string GetInfo() final;

  static int height_;
  static int width_;
};

/**
 * PursuitDomainChance is a class that represents pursuit domain,
 * it starts with a ChanceNode, so it can have more first states. */
class PursuitDomainChance: public PursuitDomain{
 public:
  // constructor
  PursuitDomainChance(unsigned int max, unsigned int maxplayers,
                      const vector<Pos> &loc);

  // constructor
  explicit PursuitDomainChance(unsigned int max);
};


extern int countStates;  // temporary for testing treewalk

extern vector<double> reward;  // temporary for testing treewalk

// Domain independent treewalk algorithm
void Pursuit(const unique_ptr<Domain>& domain, State *state,
             unsigned int depth, int players);

// Start method for domain independent treewalk algorithm
void PursuitStart(const unique_ptr<Domain>& domain, unsigned int depth = 0);

#endif  // PURSUIT_H_
