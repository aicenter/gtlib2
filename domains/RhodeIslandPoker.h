//
// Created by Jakub Rozlivek on 8/1/18.
//

#ifndef DOMAINS_RHODEISLANDPOKER_H_
#define DOMAINS_RHODEISLANDPOKER_H_

#include <experimental/optional>
#include <utility>
#include "../base/base.h"
#include "genericPoker.h"

using std::experimental::nullopt;
using std::experimental::optional;


// TODO: first version -> needs a check

namespace GTLib2 {
namespace domains {
/**
 * RhodeIslandAction is a class that represents Rhode Island Poker actions,
 * which are identified by their id and contain card number or move.
 */
class RhodeIslandPokerAction : public Action {
 public:
  // constructor
  RhodeIslandPokerAction(int id, int type, int value);

  // Returns move description.
  inline string toString() const final {
    if (id == -1)
      return "NoA";
    switch (type_) {
      case Check: return "Check";
      case Bet: return "Bet: " + to_string(value_);
      case Call: return "Call";
      case Raise: return "Raise: " + to_string(value_);
      case Fold: return "Fold";
      default: return "ERROR";
    }
  }

  inline int GetValue() const {
    return value_;
  }

  inline int GetType() const {
    return type_;
  }

  bool operator==(const Action &that) const override;
  size_t getHash() const override;

 private:
  int value_;
  int type_;
};

/**
 * RhodeIslandPokerObservation is a class that represents Rhode Island Poker observations,
 * which are identified by their id and contain an integer value
 * indicating if an action was successful(1) or not (0).
 */
class RhodeIslandPokerObservation : public Observation {  // TODO: predelat
 public:
  // constructor
  explicit RhodeIslandPokerObservation(int id, int type, int value, int color);
  /**
   * id: 0 - check; 1 - call; 2 - fold; from 3 to 3+maxCardTypes - first played cards;
   * then next maxCardTypes numbers - second played cards
   * then next BetsFirstRound.size() numbers - bets in first round
   * then next BetsSecondRound.size() numbers - bets in second round
   * then next RaisesFirstRound.size() numbers - raises in first round
   * then next RaisesSecondRound.size() numbers - raises in second round
   */

  // Returns description.
  inline string toString() const final {
    if (id == -1)
      return "NoOb";
    switch (type_) {
      case PlayCard: return "Card is " + to_string(value_) + " " + to_string(color_);
      case Check: return "Check";
      case Bet: return "Bet:" + to_string(value_);
      case Call: return "Call";
      case Raise: return "Raise:" + to_string(value_);
      case Fold: return "Fold";
      default: return "ERROR";
    }
  }
  inline int GetValue() const {
    return value_;
  }

  inline int GetType() const {
    return type_;
  }

 private:
  int value_;
  int type_;
  int color_;
};

/**
 * RhodeIslandPokerState is a class that represents Rhode Island Poker states,
 * which contains players' board - what they can see,
 * and who can play in the turn.
 */
class RhodeIslandPokerState : public State {
 public:
  // Constructor
  RhodeIslandPokerState(Domain *domain, pair<int, int> p1card,
                        pair<int, int> p2card, optional<pair<int, int>> natureCard1,
                        optional<pair<int, int>> natureCard2, double firstPlayerReward,
                        double pot, vector<int> players, int round,
                        RhodeIslandPokerAction *lastAction, int continuousRaiseCount);

  RhodeIslandPokerState(Domain *domain,
                        pair<int, int> p1card,
                        pair<int, int> p2card,
                        optional<pair<int, int>> natureCard1,
                        optional<pair<int, int>> natureCard2,
                        unsigned int ante,
                        vector<int> players);

  // Destructor
  ~RhodeIslandPokerState() override = default;

  // GetActions returns possible actions for a player in the state.
  vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;

  OutcomeDistribution
  performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const override;

  inline vector<int> getPlayers() const final {
    return players_;
  }

  bool operator==(const State &rhs) const override;

  size_t getHash() const override;

  inline string toString() const override {
    string
        s = "Player 1 card: " + to_string(player1Card_.first) + " " + to_string(player1Card_.second)
        + "\nPlayer 2 card: " + to_string(player2Card_.first) + " " + to_string(player2Card_.second)
        +
            "\nNature cards: ";
    if (natureCard1_) {
      s += to_string(natureCard1_.value().first) + " " + to_string(natureCard1_.value().second)
          + "  |  ";
      if (natureCard2_) {
        s += to_string(natureCard2_.value().first) + " " + to_string(natureCard2_.value().second);
      }
    }
    return s + "\nPlayer on move: " + to_string(players_[0]) + "\nPot: " + to_string(pot) +
        "\nReward for first player: " + to_string(firstPlayerReward) + "\nLast action: " +
        lastAction->toString() + "\nRound: " + to_string(round_) + "Continuous raise count: " +
        to_string(continuousRaiseCount_) + "\n";
  }

 protected:
  vector<int> players_;
  RhodeIslandPokerAction *lastAction;
  optional<pair<int, int>> natureCard1_;  // first number, second color (type)
  optional<pair<int, int>> natureCard2_;
  double pot;
  double firstPlayerReward;
  pair<int, int> player1Card_;  // first number, second color (type)
  pair<int, int> player2Card_;
  int round_;
  int continuousRaiseCount_;
};

/**
 * RhodeIslandPokerDomain is a class that represents Rhode Island Poker domain,
 * which contain static height and static width.
 */
class RhodeIslandPokerDomain : public Domain {
 public:
  // constructor
  RhodeIslandPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                         unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                         unsigned int maxDifferentRaises, unsigned int ante);

  RhodeIslandPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                         unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                         unsigned int maxDifferentRaises);

  RhodeIslandPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes);

  RhodeIslandPokerDomain();

  // destructor
  ~RhodeIslandPokerDomain() override = default;

  // GetInfo returns string containing domain information.
  string getInfo() const final;

  inline vector<int> getPlayers() const final {
    return {0, 1};
  }

  vector<int> betsFirstRound;
  vector<int> raisesFirstRound;
  vector<int> betsSecondRound;
  vector<int> raisesSecondRound;
  vector<int> betsThirdRound;
  vector<int> raisesThirdRound;
  const unsigned int maxCardTypes;  // cisla
  const unsigned int maxCardsOfEachType;  // barvy
  const unsigned int maxRaisesInRow;
  const unsigned int maxDifferentBets;
  const unsigned int maxDifferentRaises;
  int maxUtility;
  const unsigned int ante;
  const int TERMINAL_ROUND = 6;
};
}  // namespace domains
}  // namespace GTLib2

#endif  // DOMAINS_RHODEISLANDPOKER_H_
