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
 * which are identified by their id and contain where to move  */
class PursuitAction: public Action {
 public:
  // constructor
  PursuitAction(int id, int move);

  // Returns move description.
  inline string ToString() final {
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
 * which are identified by their id and contain vector of mini-observations to others  */
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

  // GetPlayers returns number of moves of each player who can play in this state.
  const vector<int> GetPlayers() const override {
    return players_;
  }

  // SetPlayers sets number of players who can play in this state.
  void SetPlayers(vector<int> players) override {
    players_ = players;
  }

  // GetAOH returns action-observation histories of all players.
  inline const vector<vector<int>> GetAOH() const override {
    return aoh_;
  };

  // SetAOH sets action-observation histories of all players.
  inline void SetAOH(const vector<vector<int>>& list) override {
    aoh_ = list;
  }

  // AddString adds string s to a string in vector of strings.
  inline void AddString(const string& s, int player) override {
    strings_[player].append(s);
  }

  // GetString returns a string from vector.
  inline const string& GetString(int player) const override {
    return strings_[player];
  }

  // toString returns state description
  inline string toString(int player) override {
    return  "player: " + to_string(player) +  ", location: " +
        to_string(place_[player].x) + " " + to_string(place_[player].y) +
        strings_[player] + "\n";
  }

 protected:
  vector<vector<int>> aoh_;  // all players' action-observation histories
  vector<Pos> place_;  // locations of all players
  double prob_ = 1;  // state probability
  // eight surrounding
  vector<Pos> eight_ = {{-2, -2}, {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                        {0, 1}, {1, -1}, {1, 0}, {1, 1}, {0, 0}};
  vector<Pos> m_ = {{0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}};  // moves
  vector<double> probdis_ = {0.1, 0.9};  // TODO(rozlijak): temporary
  vector<string> strings_;
  vector<int> players_;
};


class MMPursuitState: public PursuitState {
 public:
  // Constructor
  explicit MMPursuitState(const vector<Pos> &p, const vector<int>& players);

  // Constructor
  MMPursuitState(const vector<Pos> &p, double prob, const vector<int>& players);

  // PerformAction performs actions for all players who can play in the state.
  ProbDistribution PerformAction(const vector<shared_ptr<Action>>& actions) override;

  // GetNumPlayers returns number of players who can play in this state.
  inline const int GetNumPlayers() const override;

  // GetPlayers returns number of moves of each player who can play in this state.
  const vector<int> GetPlayers() const override {
    return players_;
  }

  // SetPlayers sets number of players who can play in this state.
  void SetPlayers(vector<int> players) override {
    players_ = players;
  }

 private:
  vector<int> players_;
};

/**
 * PursuitDomain is a class that represents pursuit domain,
 * which contain static height and static width.  */
class PursuitDomain: public Domain{
 public:
  // constructor
  PursuitDomain(const vector<Pos> &loc, int maxplayers, int max);

  // GetInfo returns string containing domain information.
  string GetInfo() final;

  static int height_;
  static int width_;
};

extern int count;  // temporary for testing treewalk

extern vector<double> reward;  // temporary for testing treewalk

extern std::vector<string> graph;  // temporary for python graphs
extern std::vector<int> pole;  // temporary for python graphs
extern std::vector<int> playarr;  // temporary for python graphs
extern std::vector<int> arrid;  // temporary for python graphs

// Domain independent treewalk algorithm
void Pursuit(const unique_ptr<Domain>& domain, State *state,
             int depth, int players);

#endif  // PURSUIT_H_
