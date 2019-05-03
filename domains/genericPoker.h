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


#ifndef DOMAINS_GENERICPOKER_H_
#define DOMAINS_GENERICPOKER_H_

#include "base/base.h"
#include <utility>
#include <string>
#include <vector>


namespace GTLib2::domains {

enum MOVES { Check, Call, Fold, Bet, Raise, PlayCard };

/**
 * GenericPokerAction is a class that represents Generic Poker actions,
 * which are identified by their id and contain card number or move.
 */
class GenericPokerAction: public Action {
 public:
    inline GenericPokerAction(ActionId id, int type, int value)
        : Action(id), type_(type), value_(value),
          hash_(hashCombine(698645853454825462, type_, value_)) {}
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return hash_; };
    inline string toString() const final;

    inline int GetValue() const { return value_; }
    inline int GetType() const { return type_; }

 private:
    const int value_;
    const int type_;
    const HashType hash_;
};

/**
 * GenericPokerObservation is a class that represents Generic Poker observations,
 * which are identified by their id and contain a move type with value.
 */
class GenericPokerObservation: public Observation {
 public:
    inline GenericPokerObservation(int id, int type, int value) :
        Observation(id), type_(type), value_(value) {}
    /**
     * id: 0 - check; 1 - call; 2 - fold; from 3 to 3+maxCardTypes - played cards;
     * then next BetsFirstRound.size() numbers - bets in first round
     * then next BetsSecondRound.size() numbers - bets in second round
     * then next RaisesFirstRound.size() numbers - raises in first round
     * then next RaisesSecondRound.size() numbers - raises in second round
     */

    string toString() const final;
    inline int GetValue() const { return value_; }
    inline int GetType() const { return type_; }

 private:
    const int value_;
    const int type_;
};

/**
 * GenericPokerState is a class that represents Generic Poker states,
 * which contains nature cards, pot, round etc. and who can play in the turn.
 */
class GenericPokerState: public State {
 public:
    inline GenericPokerState(Domain *domain, int player1Card, int player2Card,
                             optional<int> natureCard, double firstPlayerReward, double pot,
                             vector <Player> players, int round,
                             shared_ptr <GenericPokerAction> lastAction,
                             int continuousRaiseCount) :
        State(domain, hashCombine(23184231156465, players, player1Card, player2Card,
                                  natureCard.value_or(-1), round, continuousRaiseCount, pot,
                                  firstPlayerReward)),
        player1Card_(player1Card), player2Card_(player2Card), natureCard_(move(natureCard)),
        pot_(pot), firstPlayerReward_(firstPlayerReward), players_(move(players)), round_(round),
        continuousRaiseCount_(continuousRaiseCount), lastAction_(move(lastAction)) {}

    inline GenericPokerState(Domain *domain, int p1card, int p2card, optional<int> natureCard,
                             unsigned int ante, vector <Player> players)
        : GenericPokerState(domain, p1card, p2card, move(natureCard), ante, 2 * ante,
                            move(players), 1, nullptr, 0) {}

    ~GenericPokerState() override = default;

    unsigned long countAvailableActionsFor(Player player) const override;
    vector <shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector <PlayerAction> &actions) const override;

    inline vector <Player> getPlayers() const final { return players_; }
    int hasPlayerOneWon(const shared_ptr <GenericPokerAction> &lastAction, Player player) const;

    bool operator==(const State &rhs) const override;
    inline string toString() const override;

 protected:
    const vector <Player> players_;
    const shared_ptr <GenericPokerAction> lastAction_;
    const optional<int> natureCard_;
    const double pot_;
    const double firstPlayerReward_;
    const int player1Card_;
    const int player2Card_;
    const int round_;
    const int continuousRaiseCount_;
};

/**
 * GenericPokerDomain is a class that represents Generic Poker domain,
 * which contain possible bets and raises, max card types, max cards of each type, max different
 * bets and raises and Max utility.
 */
class GenericPokerDomain: public Domain {
 public:
    GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                       unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                       unsigned int maxDifferentRaises, unsigned int ante);
    GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                       unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                       unsigned int maxDifferentRaises);
    GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes);
    GenericPokerDomain();
    ~GenericPokerDomain() override = default;
    string getInfo() const final;
    inline vector <Player> getPlayers() const final { return {0, 1}; }

    vector<int> betsFirstRound_;
    vector<int> raisesFirstRound_;
    vector<int> betsSecondRound_;
    vector<int> raisesSecondRound_;
    const unsigned int maxCardTypes_;   // numbers
    const unsigned int maxCardsOfEachType_;  // colors
    const unsigned int maxRaisesInRow_;
    const unsigned int maxDifferentBets_;
    const unsigned int maxDifferentRaises_;
    const unsigned int ante_;
    const int TERMINAL_ROUND = 4;
};
}  // namespace GTLib2

#endif  // DOMAINS_GENERICPOKER_H_
