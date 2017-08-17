//
// Created by rozliv on 02.08.2017.
//

#ifndef PURSUIT_H_
#define PURSUIT_H_

#include "base.h"

//pokus o comment
class PursuitAction: public Action {
 public:
  PursuitAction(int id, int move);

  string ToString() final;


  inline int GetMove() const {
    return move_;
  }
 private:
  int move_;
};

class PursuitObservation: public Observation {
 public:
  PursuitObservation(int id, vector<int> values);

  string ToString() final;

  inline const vector<int>& GetValues() const {
    return values_;
  }

 private:
  vector<int> values_;
};


class PursuitState: public State {
 public:
  explicit PursuitState(const vector<Pos> &p);

  PursuitState(const vector<Pos> &p, double prob);

  vector<shared_ptr<Action>> GetActions(int player) override;

  void GetActions(vector<shared_ptr<Action>>& list, int player) const final;

  ProbDistribution PerformAction(const vector<shared_ptr<Action>>& actions) final;

  inline const vector<Pos>& GetPlace() const final {
    return place_;
  }

  inline const vector<Pos>& GetEight() const {
    return eight_;
  }

  inline const vector<Pos>& GetMoves() const {
    return m_;
  }

  inline const vector<double>& GetProb() const {
    return probdis_;
  }

  inline double GetPro() const {
    return prob_;
  }

 private:
  vector<Pos> place_;
  double prob_ = 1;
  vector<Pos> eight_ = {{-2, -2}, {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                        {0, 1}, {1, -1}, {1, 0}, {1, 1}, {0, 0}};
  vector<Pos> m_ = {{0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}};
  vector<double> probdis_ = {0.1, 0.9};  // TODO(rozlijak): docasne
};


class PursuitDomain: public Domain{
 public:
  PursuitDomain(const vector<Pos> &loc, int maxPlayers, int max);

  string GetInfo() final; //pokus o comment

  static int height_;
  static int width_;
};

extern int count;

extern vector<double> reward;

void Pursuit(const unique_ptr<Domain>& domain, State *state,
             int depth, int players);

#endif  // PURSUIT_H_
