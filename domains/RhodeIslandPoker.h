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

#include "base/base.h"
#include <utility>
#include <string>
#include <vector>
#include "domains/genericPoker.h"



// TODO: first version -> needs a check

namespace GTLib2::domains {
/**
 * RhodeIslandAction is a class that represents Rhode Island Poker actions,
 * which are identified by their id and contain card number or move.
 */
class RhodeIslandPokerAction: public Action {
 public:
    RhodeIslandPokerAction(ActionId id, int type, int value);
    inline string toString() const final;
    inline int GetValue() const { return value_; }
    inline int GetType() const { return type_; }
    bool operator==(const Action &that) const override;

 private:
    const int value_;
    const int type_;
};

/**
 * RhodeIslandPokerObservation is a class that represents Rhode Island Poker observations,
 * which are identified by their id and contain a move type with value and color in case of card.
 */
class RhodeIslandPokerObservation: public Observation {
 public:
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

    inline string toString() const final;
    inline int GetValue() const { return value_; }
    inline int GetType() const { return type_; }

 private:
    const int value_;
    const int type_;
    const int color_;
};

/**
 * RhodeIslandPokerState is a class that represents Rhode Island Poker states,
 * which contains nature cards, pot, round etc. and who can play in the turn.
 */
class RhodeIslandPokerState: public State {
 public:
    RhodeIslandPokerState(const Domain *domain, pair<int, int> player1Card,
                          pair<int, int> player2Card, optional<pair<int, int>> natureCard1,
                          optional<pair<int, int>> natureCard2, double firstPlayerReward,
                          double pot, vector<Player> players, int round,
                          shared_ptr<RhodeIslandPokerAction> lastAction, int continuousRaiseCount);
    RhodeIslandPokerState(const Domain *domain,
                          pair<int, int> player1card, pair<int, int> player2card,
                          optional<pair<int, int>> natureCard1,
                          optional<pair<int, int>> natureCard2,
                          unsigned int ante,
                          vector<Player> players);
    ~RhodeIslandPokerState() override = default;

    unsigned long countAvailableActionsFor(Player player) const override;
    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector<PlayerAction> &actions) const override;
    inline vector<Player> getPlayers() const final { return players_; }
    int hasPlayerOneWon(const shared_ptr<RhodeIslandPokerAction> &lastAction, Player player) const;
    bool operator==(const State &rhs) const override;
    inline string toString() const override;

 protected:
    const vector<Player> players_;
    const shared_ptr<RhodeIslandPokerAction> lastAction_;
    const optional<pair<int, int>> natureCard1_;  // first number, second color (type)
    const optional<pair<int, int>> natureCard2_;
    const double pot_;
    const double firstPlayerReward_;
    const pair<int, int> player1Card_;  // first number, second color (type)
    const pair<int, int> player2Card_;
    const int round_;
    const int continuousRaiseCount_;
};

/**
 * RhodeIslandPokerDomain is a class that represents Rhode Island Poker domain,
 * which contain possible bets and raises, max card types, max cards of each type, max different
 * bets and raises and Max utility.
 */
class RhodeIslandPokerDomain: public Domain {
 public:
    RhodeIslandPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                           unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                           unsigned int maxDifferentRaises, unsigned int ante);
    RhodeIslandPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                           unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                           unsigned int maxDifferentRaises);
    RhodeIslandPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes);
    RhodeIslandPokerDomain();
    ~RhodeIslandPokerDomain() override = default;

    string getInfo() const final;
    inline vector<Player> getPlayers() const final { return {0, 1}; }

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
}  // namespace GTLib2

#endif  // DOMAINS_RHODEISLANDPOKER_H_
