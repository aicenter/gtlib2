//
// Created by Jakub Rozlivek on 02.11.2017.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PHANTOMTTT_H_
#define PHANTOMTTT_H_

#include "../base/base.h"

using namespace GTLib2;

namespace GTLib2 {

    const vector<string> moved_ = {"top left", "top", "top right",
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
        PhantomTTTState(const vector<vector<int>> &p, const vector<int> &players);

        // Destructor
        ~PhantomTTTState() override = default;

        // GetActions returns possible actions for a player in the state.
        vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;

        OutcomeDistribution
        performActions(const unordered_map<int, shared_ptr<Action>> &actions) const override;


        inline vector<int> getPlayers() const final {
          return players_;
        }

        // AddString adds string s to a string in vector of strings.
        inline void AddString(const string &s, int player) override {
          strings_[player].append(s);
        }

        inline bool operator==(const State &rhs) const override {
          auto State = dynamic_cast<const PhantomTTTState&>(rhs);

          return  place_== State.place_ &&
                  strings_ == State.strings_ &&
                  players_ == State.players_;
        }


        inline size_t getHash() const override {
          size_t seed = 0;
          for (auto &i : place_) {
            boost::hash_combine(seed, i);
          }
          for (auto &i : players_) {
            boost::hash_combine(seed, i);
          }
          return seed;
        }


        // ToString returns state description.
        inline string toString(int player) const override {
          string s;
          for (int i = 0; i < 9; ++i) {
            switch (place_[player][i]) {
              case 0: s += "_ "; break;
              case 1: s += "x "; break;
              case 2: s += "o "; break;
              default: s += "- "; break;
            }
            if (i == 2 || i == 5) {
              s += "\n";
            }
          }
          return "player: " + to_string(player) + " board:\n" + s + "\n";
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
          return {0,1};
        }

    };
}
#endif  // PHANTOMTTT_H_

#pragma clang diagnostic pop