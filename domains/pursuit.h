//
// Created by rozliv on 02.08.2017.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_H_
#define PURSUIT_H_

#include <utility>

#include "../base/base.h"
#include "../base/efg.h"

using std::string;

using namespace GTLib2;

struct Pos {
    int y;
    int x;
};

// moves description
const vector<string> movedes_ = {"stay", "right", "down", "left", "up"};

// eight surrounding description
const vector<string> eightdes_ = {"nowhere", "top left", "top", "top right",
                                  "left", "right", "bottom left", "bottom",
                                  "bottom right", "same"};

/**
 * PursuitAction is a class that represents pursuit actions,
 * which are identified by their id and contain where to move.
 */
class PursuitAction: public Action {
 public:
  // constructor
  PursuitAction(int id, int move);

  // Returns move description.
  inline string toString() const final {
    if (id == -1)
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
 * contain vector of mini-observations to others.
 */
class PursuitObservation: public Observation {
 public:
  // constructor
  PursuitObservation(int id, vector<int> values);

  // Returns description.
  string toString() const final;

  // Returns vector of mini-observations to others.
  inline const vector<int>& GetValues() const {
    return values_;
  }

 private:
  vector<int> values_;
};

/**
 * PursuitObservationLoc is a class that represents pursuit observation,
 * which are identified by their id and contain vector of others' locations.
 */
class PursuitObservationLoc: public Observation {
 public:
  // constructor
  PursuitObservationLoc(int id, vector<Pos> values);

  // Returns description.
  string toString() const final;

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
 * vector of locations of all players and state probability.
 */
class PursuitState: public State {
 public:
  // Constructor
  explicit PursuitState(const vector<Pos> &p);

  // Constructor
  PursuitState(const vector<Pos> &p, double prob);

  // Destructor
  ~PursuitState() override = default;

  // GetActions returns possible actions for a player in the state.
  vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;

  // GetActions returns possible actions for a player in the state.
  void GetActions(vector<shared_ptr<Action>>& list, int player) const override;

  // OldPerformAction performs actions for all players who can play in the state.
  OutcomeDistributionOld PerformAction(const vector<shared_ptr<Action>>& actions) override;

    OutcomeDistribution performActions(const unordered_map<int,shared_ptr<Action>> &actions) const override;

  // GetNumPlayers returns number of players who can play in this state.
  inline int getNumberOfPlayers() const override {
    return  place_.size();
  }

  // GetPlayers returns who can play in this state.
  inline const vector<bool>& GetPlayers() const override {
    return players_;
  }




  // AddString adds string s to a string in vector of strings.
  inline void AddString(const string& s, int player) override {
    strings_[player].append(s);
  }

  // ToString returns state description.
  inline string toString(int player) const override {
    return  "player: " + to_string(player) +  ", location: " +
        to_string(place_[player].x) + " " + to_string(place_[player].y) +
        strings_[player] + "\n";
  }

 protected:
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
 * which contains vector of all players and a move count of player on turn.
 */
class MMPursuitState: public PursuitState {
 public:
  // Constructor
  MMPursuitState(const vector<Pos> &p, const vector<bool>& players,
                 int movecount);

  // Constructor
  MMPursuitState(const vector<Pos> &p, double prob,
                 const vector<bool>& players, int movecount);

  // OldPerformAction performs actions for all players who can play in the state.
  OutcomeDistributionOld PerformAction(const vector<shared_ptr<Action>>& actions) override;

  // GetNumPlayers returns number of players who can play in this state.
  inline int getNumberOfPlayers() const override;

  // GetPlayers returns who can play in this state.
  inline const vector<bool>& GetPlayers() const override {
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
 * A difference is that it uses PursuitObservationLoc.
 */
class ObsPursuitState: public PursuitState {
 public:
  // Constructor
  explicit ObsPursuitState(const vector<Pos> &p);

  // Constructor
  ObsPursuitState(const vector<Pos> &p, double prob);

  // OldPerformAction performs actions for all players who can play in the state.
  OutcomeDistributionOld PerformAction(const vector<shared_ptr<Action>>& actions) override;
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
  PursuitDomain(unsigned int max, unsigned int maxplayers,
                const shared_ptr<MMPursuitState>& state);

  // constructor
  explicit PursuitDomain(unsigned int max);

  // destructor
  ~PursuitDomain() override = default;

  // GetInfo returns string containing domain information.
  string getInfo() const final;


  static int height_;
  static int width_;
};

/**
 * PursuitDomainChance is a class that represents pursuit domain,
 * it starts with a ChanceNode, so it can have more first states.
 */
class PursuitDomainChance: public PursuitDomain{
 public:
  // constructor
  PursuitDomainChance(unsigned int max, unsigned int maxplayers,
                      const vector<Pos> &loc);

  // constructor
  PursuitDomainChance(unsigned int max, unsigned int maxplayers,
                const shared_ptr<MMPursuitState>& state);

  // constructor
  explicit PursuitDomainChance(unsigned int max);
};




// Domain independent treewalk algorithm
void Pursuit(const shared_ptr<Domain>& domain, State *state,
             unsigned int depth, int players);

// Start method for domain independent treewalk algorithm
void PursuitStart(const shared_ptr<Domain>& domain, unsigned int depth = 0);

#endif  // PURSUIT_H_

#pragma clang diagnostic pop