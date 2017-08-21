//
// Created by rozliv on 02.08.2017.
//

#ifndef PURSUIT_H_
#define PURSUIT_H_

#include "base.h"


/**
 * PursuitAction is a class that represents pursuit actions,
 * which are identified by their id and contain where to move  */
class PursuitAction: public Action {
 public:
  // constructor
  PursuitAction(int id, int move);

  string ToString() final;

  // Returns index to array, which describes where to move.
  inline int GetMove() const {
    return move_;
  }

  inline int GetID() const {
    return id_;
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

  // GetActions returns possible actions for a player in the state.
  vector<shared_ptr<Action>> GetActions(int player) final;

  // GetActions returns possible actions for a player in the state.
  void GetActions(vector<shared_ptr<Action>>& list, int player) const final;

  // PerformAction performs actions for all players who can play in the state.
  ProbDistribution PerformAction(const vector<shared_ptr<Action>>& actions) final;

  // SetPlayers sets number of players who can play in this state.
  inline void SetPlayers(unsigned int number) final {
    numplayers_ = number;
  }

  // GetPlace returns locations of all players.
  inline const vector<Pos>& GetPlace() const {
    return place_;
  }

  // GetPlayers returns number of players who can play in this state.
  inline const int GetPlayers() const final {
    return numplayers_ == 0? place_.size() : numplayers_;
  }

  // GetAOH returns action-observation histories of all players.
  inline const vector<vector<int>>& GetAOH() const final {
    return aoh_;
  };

  // SetAOH sets action-observation histories of all players.
  inline void SetAOH(const vector<vector<int>>& list) final {
    aoh_ = list;
  }

 private:
  vector<vector<int>> aoh_;  // all players' action-observation histories
  vector<Pos> place_;  // locations of all players
  double prob_ = 1;  // state probability
  vector<Pos> eight_ = {{-2, -2}, {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                        {0, 1}, {1, -1}, {1, 0}, {1, 1}, {0, 0}};  // eight surrounding
  vector<Pos> m_ = {{0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}};  // moves
  vector<double> probdis_ = {0.1, 0.9};  // TODO(rozlijak): temporary
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

// Domain independent treewalk algorithm
void Pursuit(const unique_ptr<Domain>& domain, State *state,
             int depth, int players);

#endif  // PURSUIT_H_
