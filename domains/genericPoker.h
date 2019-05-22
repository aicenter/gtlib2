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

namespace GTLib2::domains {

enum PokerMoves { Check, Call, Fold, Bet, Raise, PlayCard };

constexpr int POKER_TERMINAL_ROUND = 4;

/**
 * GenericPokerAction is a class that represents Generic Poker actions,
 * which are identified by their id and contain card number or move.
 */
class GenericPokerAction: public Action {
 public:
    inline GenericPokerAction() : Action(), type_(0), value_(0), hash_(0) {}
    inline GenericPokerAction(ActionId id, unsigned int type, unsigned int value)
        : Action(id), type_(type), value_(value),
          hash_(hashCombine(698645853454825462, type_, value_)) {}
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return hash_; };
    inline string toString() const final;

    inline unsigned int getValue() const { return value_; }
    inline unsigned int getType() const { return type_; }

 private:
    const unsigned int value_;
    const unsigned int type_;
    const HashType hash_;
};

/**
 * ID can have following value:
 *
 * 0 - check
 * 1 - call
 * 2 - fold
 * 3 to 3+maxCardTypes - played cards;
 * then next BetsFirstRound.size() numbers - bets in first round
 * then next BetsSecondRound.size() numbers - bets in second round
 * then next RaisesFirstRound.size() numbers - raises in first round
 * then next RaisesSecondRound.size() numbers - raises in second round
 */
typedef ObservationId PokerObservationId;

/**
 * GenericPokerObservation are identified by their id and contain a move type with value.
 */
class GenericPokerObservation: public Observation {
 public:
    inline GenericPokerObservation() : Observation(), value_(0), type_(0) {}
    inline GenericPokerObservation(PokerObservationId id, unsigned int type, unsigned int value) :
        Observation(id), type_(type), value_(value) {}

    string toString() const final;
    inline unsigned int getValue() const { return value_; }
    inline unsigned int getType() const { return type_; }

 private:
    const unsigned int value_;
    const unsigned int type_;
};

/**
 * Generic Poker (GP) is a simplified poker game inspired by Leduc Hold’em.
 *
 * First, both players are required to put ante in the pot.
 *
 * Next, chance deals a single private card to each
 * player, and the betting round begins. A player can either fold (the
 * opponent wins the pot), check (let the opponent make the next
 * move), bet (add some amount of chips, as first in the round), call
 * (add the amount of chips equal to the last bet of the opponent into
 * the pot), or raise (match and increase the bet of the opponent).
 * If no further raise is made by any of the players, the betting
 * round ends, chance deals one public card on the table, and a second
 * betting round with the same rules begins. After the second betting
 * round ends, the outcome of the game is determined - a player wins if:
 *
 *   (1) her private card matches the table card and the opponent’s
 *       card does not match, or
 *   (2) none of the players’ cards matches the  table card and her private card
 *       is higher than the private card of the opponent.
 *
 * If no player wins, the game is a draw and the pot is split.
 *
 * The parameters of the game are:
 *
 * maxCardTypes       - the number of types of the cards
 * maxCardsOfEachType - the number of cards of each type
 * maxRaisesInRow     - the maximum length of sequence of raises in a betting round
 * maxDifferentBets   - the number of different sizes of bets for bet actions
 *                    (i.e., amount of chips added to the pot)
 * maxDifferentRaises - the number of different sizes of bets for raise actions
 *                      (i.e., amount of chips added to the pot)
 * ante               - initial bet players must place
 *
 * This game has only public actions. However, it includes additional chance nodes
 * later in the game, which reveal part of the information not available before.
 * Moreover it has integer results and not just win/draw/loss.
 */
class GenericPokerDomain: public Domain {
 public:
    GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfEachType,
                       unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                       unsigned int maxDifferentRaises, unsigned int ante);
    GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfEachType,
                       unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                       unsigned int maxDifferentRaises);
    GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes);
    GenericPokerDomain();
    ~GenericPokerDomain() override = default;
    string getInfo() const final;
    vector<int> betsFirstRound_;
    vector<int> raisesFirstRound_;
    vector<int> betsSecondRound_;
    vector<int> raisesSecondRound_;
    const unsigned int maxCardTypes_;        // numbers
    const unsigned int maxCardsOfEachType_;  // colors
    const unsigned int maxRaisesInRow_;
    const unsigned int maxDifferentBets_;
    const unsigned int maxDifferentRaises_;
    const unsigned int ante_;
};

/**
 * GenericPokerState is a class that represents Generic Poker states,
 * which contains nature cards, pot, round etc. and who can play in the turn.
 */
class GenericPokerState: public State {
 public:
    inline GenericPokerState(const Domain *domain,
                             int player1Card, int player2Card, optional<int> natureCard,
                             double firstPlayerReward, double pot,
                             Player actingPlayer, int round,
                             shared_ptr<GenericPokerAction> lastAction,
                             int continuousRaiseCount) :
        State(domain, hashCombine(23184231156465, actingPlayer, player1Card, player2Card,
                                  natureCard.value_or(-1), round, continuousRaiseCount, pot,
                                  firstPlayerReward)),
        player1Card_(player1Card), player2Card_(player2Card), natureCard_(move(natureCard)),
        pot_(pot), firstPlayerReward_(firstPlayerReward), actingPlayer_(move(actingPlayer)),
        round_(round),
        continuousRaiseCount_(continuousRaiseCount), lastAction_(move(lastAction)) {}

    inline GenericPokerState(const Domain *domain, int p1card, int p2card, optional<int> natureCard,
                             unsigned int ante, Player actingPlayer)
        : GenericPokerState(domain, p1card, p2card, move(natureCard), ante, 2 * ante,
                            actingPlayer, 1, nullptr, 0) {}

    ~GenericPokerState() override = default;

    unsigned long countAvailableActionsFor(Player player) const override;
    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector<shared_ptr<Action>> &actions) const override;

    inline vector<Player> getPlayers() const final { return vector<Player>{actingPlayer_}; }
    int hasPlayerOneWon(const shared_ptr<GenericPokerAction> &lastAction, Player player) const;
    inline bool isTerminal() const override { return round_ == POKER_TERMINAL_ROUND; };

    bool operator==(const State &rhs) const override;
    inline string toString() const override;

 protected:
    const Player actingPlayer_;
    const shared_ptr<GenericPokerAction> lastAction_;
    const optional<int> natureCard_;
    const double pot_;
    const double firstPlayerReward_;
    const int player1Card_;
    const int player2Card_;
    const int round_;
    const int continuousRaiseCount_;
 private:
    OutcomeDistribution revealChanceCard(double newFirstPlayerReward, double new_pot,
                                         const shared_ptr<GenericPokerAction> &pokerAction) const;

    OutcomeDistribution progressRound(double newFirstPlayerReward, double newContinuousRaiseCount,
                                      double newPot, Player currentPlayer,
                                      int newRound, PokerObservationId obsId,
                                      const shared_ptr<GenericPokerAction> &pokerAction) const;
};

}  // namespace GTLib2

#endif  // DOMAINS_GENERICPOKER_H_
