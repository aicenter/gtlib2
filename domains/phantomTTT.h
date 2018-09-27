//
// Created by Jakub Rozlivek on 02.11.2017.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef DOMAINS_PHANTOMTTT_H_
#define DOMAINS_PHANTOMTTT_H_

#include <utility>
#include <vector>
#include <string>
#include "base/base.h"

namespace GTLib2 {
namespace domains {
const std::array<string, 9> moved_ = {"top left", "top", "top right",
                                      "left", "center", "right", "bottom left",
                                      "bottom", "bottom right"};

/**
 * PhantomTTTAction is a class that represents PhantomTTT actions,
 * which are identified by their id and contain where to play.
 */
class PhantomTTTAction : public Action {
 public:
  // constructor
  PhantomTTTAction(int id, int move);

  // Returns move description.
  inline string toString() const final {
    if (id == -1)
      return "NoA";
    return moved_[move_];
  }

  bool operator==(const Action &that) const override;
  size_t getHash() const override;

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
class PhantomTTTObservation : public Observation {
 public:
  // constructor
  explicit PhantomTTTObservation(int id);

  // Returns description.
  inline string toString() const final {
    if (id == 1)
      return "Success";
    return "Fail";
  };
};

/**
 * PhantomTTTState is a class that represents PhantomTTT states,
 * which contains players' board - what they can see,
 * and who can play in the turn.
 */
class PhantomTTTState : public State {
 public:
  // Constructor
  PhantomTTTState(Domain *domain, vector<vector<int>> p, vector<int> players);

  // Destructor
  ~PhantomTTTState() override = default;

  // GetActions returns possible actions for a player in the state.
  vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;

  OutcomeDistribution
  performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const override;

  inline vector<int> getPlayers() const final {
    return players_;
  }

  bool operator==(const State &rhs) const override;

  size_t getHash() const override;

  // ToString returns state description.
  inline string toString() const override {
    string s;
    for (int player = 0; player < place_.size(); ++player) {
      s += "Player: " + to_string(player) + " board:\n";
      for (int i = 0; i < 9; ++i) {
        switch (place_[player][i]) {
          case 0:s += "_ ";
            break;
          case 1:s += "x ";
            break;
          case 2:s += "o ";
            break;
          default:s += "- ";
            break;
        }
        if (i == 2 || i == 5 || i == 8) {
          s += "\n";
        }
      }
    }
    return s;
  }

 protected:
  vector<vector<int>> place_;  // players' board
  vector<string> strings_;
  vector<int> players_;
};

/**
 * PhantomTTTDomain is a class that represents PhantomTTT domain,
 * which contain static height and static width.
 */
class PhantomTTTDomain : public Domain {
 public:
  // constructor
  explicit PhantomTTTDomain(unsigned int max);

  // destructor
  ~PhantomTTTDomain() override = default;

  // GetInfo returns string containing domain information.
  string getInfo() const final;

  inline vector<int> getPlayers() const final {
    return {0, 1};
  }
};
}  // namespace domains
}  // namespace GTLib2
#endif  // DOMAINS_PHANTOMTTT_H_

#pragma clang diagnostic pop
