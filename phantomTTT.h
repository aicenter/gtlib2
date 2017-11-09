//
// Created by Jacob on 02.11.2017.
//

#ifndef PHANTOMTTT_H_
#define PHANTOMTTT_H_

#include "base.h"

const vector<string> moved_ = {"top left", "top", "top right",
                                 "left", "center", "right", "bottom left",
                                 "bottom", "bottom right"};

/**
 * PhantomTTTAction is a class that represents PhantomTTT actions,
 * which are identified by their id and contain where to play.
 */
class PhantomTTTAction: public Action {
 public:
  // constructor
  PhantomTTTAction(int id, int move);

  // Returns move description.
  inline string ToString() final {
    if (id_ == -1)
      return "NoA";
    return moved_[move_];
  }

  // Returns index to 3x3 array, which describes where to move.
  inline int GetMove() const {
    return move_;
  }
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
  // constructor
  PhantomTTTObservation(int id, int value);

  // Returns description.
  inline string ToString() final {
    if (value_ == 1)
      return "Success";
    return "Fail";
  };

  // Returns vector of mini-observations to others.
  inline int GetValue() const {
    return value_;
  }

 private:
  int value_;
};

/**
 * PhantomTTTState is a class that represents PhantomTTT states,
 * which contains players' board - what they can see,
 * and who can play in the turn.
 */
class PhantomTTTState: public State {
 public:
  // Constructor
  PhantomTTTState(const vector<vector<int>> &p, const vector<bool>& players);

  // Destructor
  ~PhantomTTTState() override = default;

  // GetActions returns possible actions for a player in the state.
  vector<shared_ptr<Action>> GetActions(int player) override;

  // GetActions returns possible actions for a player in the state.
  void GetActions(vector<shared_ptr<Action>>& list, int player) const override;

  // PerformAction performs actions for all players who can play in the state.
  ProbDistribution PerformAction(const vector<shared_ptr<Action>>& actions2) override;

  // GetNumPlayers returns number of players who can play in this state.
  inline const int GetNumPlayers() const override {
    return 1;
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
  inline string ToString(int player) override {
    string s;
    for (int i = 0; i < 9; ++i) {
      switch (place_[player][i]) {
        case 0: s+= "_ "; break;
        case 1: s += "x "; break;
        case 2: s += "o "; break;
        default: s += "- "; break;
      }
      if (i == 2 || i == 5) {
        s += "\n";
      }
    }
    return  "player: " + to_string(player) +  " board:\n" + s + "\n";
  }

 protected:
  vector<vector<int>> place_;  // players' board
  vector<string> strings_;
  vector<bool> players_;
};


/**
 * PhantomTTTDomain is a class that represents PhantomTTT domain,
 * which contain static height and static width.
 */
class PhantomTTTDomain: public Domain{
 public:
  // constructor
  explicit PhantomTTTDomain(unsigned int max);

  // destructor
  ~PhantomTTTDomain() override = default;

  // GetInfo returns string containing domain information.
  string GetInfo() final;
};

#endif  // PHANTOMTTT_H_
