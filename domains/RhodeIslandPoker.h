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


#ifndef DOMAINS_RHODEISLANDPOKER_H_
#define DOMAINS_RHODEISLANDPOKER_H_

#include <experimental/optional>
#include <utility>
#include <string>
#include <vector>
#include "base/base.h"
#include "domains/genericPoker.h"

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
  RhodeIslandPokerAction(ActionId id, int type, int value);

  // Returns move description.
  inline string toString() const final {
    if (id_ == NO_ACTION)
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
 * which are identified by their id and contain a move type with value and color in case of card.
 */
class RhodeIslandPokerObservation : public Observation {
 public:
  // constructor
  explicit RhodeIslandPokerObservation(int id, int type, int value, int color);
  /**
   * id: 0 - check; 1 - call; 2 - fold; from 3 to 3+maxCardTypes - first played cards;
   * then next maxCardTypes numbers - second played cards
   * then next BetsFirstRound.size() numbers - bets in first round
   * then next BetsSecondRound.size() numbers - bets in second round
   * then next BetsThirdRound.size() numbers - bets in third round
   * then next RaisesFirstRound.size() numbers - raises in first round
   * then next RaisesSecondRound.size() numbers - raises in second round
   * then next RaisesThirdRound.size() numbers - raises in third round
   */

  // Returns description.
  inline string toString() const final {
    if (id_ == NO_OBSERVATION)
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
 * which contains nature cards, pot, round etc. and who can play in the turn.
 */
class RhodeIslandPokerState : public State {
 public:
  // Constructor
  RhodeIslandPokerState(Domain *domain, pair<int, int> p1card,
                        pair<int, int> p2card, optional<pair<int, int>> natureCard1,
                        optional<pair<int, int>> natureCard2, double firstPlayerReward,
                        double pot, vector<Player> players, int round,
                        shared_ptr<RhodeIslandPokerAction> lastAction, int continuousRaiseCount);

  RhodeIslandPokerState(Domain *domain,
                        pair<int, int> p1card,
                        pair<int, int> p2card,
                        optional<pair<int, int>> natureCard1,
                        optional<pair<int, int>> natureCard2,
                        unsigned int ante,
                        vector<Player> players);

  // Destructor
  ~RhodeIslandPokerState() override = default;

  unsigned long countAvailableActionsFor(Player player) const override;

  // GetActions returns possible actions for a player in the state.
  vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;

  OutcomeDistribution
  performActions(const vector<PlayerAction> &actions) const override;

  inline vector<Player> getPlayers() const final {
    return players_;
  }

  int hasPlayerOneWon(const shared_ptr<RhodeIslandPokerAction> &lastAction, Player player) const;

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
    return s + "\nPlayer on move: " + to_string(players_[0]) + "\nPot: " + to_string(pot_) +
        "\nReward for first player: " + to_string(firstPlayerReward_) + "\nLast action: " +
        lastAction_->toString() + "\nRound: " + to_string(round_) + "Continuous raise count: " +
        to_string(continuousRaiseCount_) + "\n";
  }

 protected:
  vector<Player> players_;
  shared_ptr<RhodeIslandPokerAction> lastAction_;
  optional<pair<int, int>> natureCard1_;  // first number, second color (type)
  optional<pair<int, int>> natureCard2_;
  double pot_;
  double firstPlayerReward_;
  pair<int, int> player1Card_;  // first number, second color (type)
  pair<int, int> player2Card_;
  int round_;
  int continuousRaiseCount_;
};

/**
 * RhodeIslandPokerDomain is a class that represents Rhode Island Poker domain,
 * which contain possible bets and raises, max card types, max cards of each type, max different
 * bets and raises and Max utility.
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

  inline vector<Player> getPlayers() const final {
    return {0, 1};
  }

  vector<int> betsFirstRound_;
  vector<int> raisesFirstRound_;
  vector<int> betsSecondRound_;
  vector<int> raisesSecondRound_;
  vector<int> betsThirdRound_;
  vector<int> raisesThirdRound_;
  const unsigned int maxCardTypes_;  // cisla
  const unsigned int maxCardsOfEachType_;  // barvy
  const unsigned int maxRaisesInRow_;
  const unsigned int maxDifferentBets_;
  const unsigned int maxDifferentRaises_;
  const unsigned int ante_;
  const int TERMINAL_ROUND = 6;
};
}  // namespace domains
}  // namespace GTLib2

#endif  // DOMAINS_RHODEISLANDPOKER_H_
