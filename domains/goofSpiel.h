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


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef DOMAINS_GOOFSPIEL_H_
#define DOMAINS_GOOFSPIEL_H_

#include <utility>

#include "base/base.h"

namespace GTLib2::domains {

/**
 * IncompleteObservations correspond to "IIGS",
 * CompleteObservations correspond to "GS".
 */
enum GoofSpielVariant { CompleteObservations, IncompleteObservations };


struct GoofSpielSettings {
    GoofSpielVariant variant = CompleteObservations;
    uint32 numCards = 5;

    bool fixChanceCards = false;
    vector<int> chanceCards = {}; // used only if fixChanceCards == true

    bool binaryTerminalRewards = false;

    void shuffleChanceCards(unsigned long seed);
    vector<int> getNatureCards();
};

class GoofSpielAction: public Action {
 public:
    inline GoofSpielAction() : Action(), cardNumber_(0) {}
    inline GoofSpielAction(ActionId id, int card) : Action(id), cardNumber_(card) {}
    inline string toString() const override { return "Card: " + to_string(cardNumber_); };
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return cardNumber_; };
    const int cardNumber_;
};

struct GoofSpielRevealedInfo : RevealedInfo
{
    GoofSpielRevealedInfo (vector<int> cards) : cardOptions(std::move(cards)) {};
    vector<int> cardOptions;
};

/**
 * In Goofspiel (GS), each player is given a private hand of bid cards with values 1 to N.
 *
 * A different deck of N point cards is placed face up in a stack (nature deck).
 * This deck can be randomly shuffled, or can have a fixed known sequence in advance.
 *
 * On their turn, each player bids for the top point card by secretly choosing a single
 * card in their hand. The highest bidder gets the point card and adds the point total
 * to their score, discarding the points in the case of a tie.
 *
 * This is repeated N times and the player with the highest score wins.
 *
 * In Imperfect Information Goofspiel (IIGS), the players only discover
 * who won or lost a bid, but not the bid cards played. This way all actions are private
 * and information sets have various sizes.
 */
class GoofSpielDomain: public ExtendedDomain {
 public:
    explicit GoofSpielDomain(GoofSpielSettings settings);
    string getInfo() const override;
    vector <Player> getPlayers() const { return {0, 1}; }
    const int numberOfCards_;
    const bool fixChanceCards_;
    const bool binaryTerminalRewards_;
    const GoofSpielVariant variant_;
    const vector<int> natureCards_;

    // Factories for common instances of IIGS
    static unique_ptr<GoofSpielDomain> IIGS(unsigned int n);
    static unique_ptr<GoofSpielDomain> GS(unsigned int n);
    bool proceedAOIDs(const Player playingPlayer, const vector<ActionObservationIds> & aoids, long & startIndex,
                      unordered_map<unsigned long, shared_ptr<RevealedInfo>> & revealedInfo) const override;
    void generateNodes(const Player playingPlayer, const vector<ActionObservationIds> & aoids,
                       const unordered_map<unsigned long, shared_ptr<RevealedInfo>> & revealedInfo,
                       const int max,const std::function<double(const shared_ptr<EFGNode> &)>& func) const override;
    virtual void prepareRevealedMap(unordered_map<unsigned long, shared_ptr<RevealedInfo>> &revealedInfo) const override;

 private:
    void initRandomCards(const vector<int> &natureCards);
    void initFixedCards(const vector<int> &natureCards);
    void recursiveNodeGeneration(const Player playingPlayer, const vector<ActionObservationIds> & aoids,
                                                  const shared_ptr<EFGNode> & node, int depth, int maxDepth, const unordered_map<unsigned long, shared_ptr<RevealedInfo>> &revealedInfo,
                                                  vector<int> remaining, int & counter, const std::function<double(const shared_ptr<EFGNode> &)>& func) const;
};

constexpr int NO_NATURE_CARD = 0;
constexpr int NO_CARD_OBSERVATION = 0;

enum GoofspielRoundOutcome {
    PL0_DRAW = 0,
    PL0_WIN = 1,
    PL0_LOSE = -1
};

class GoofSpielObservation: public Observation {
 public:
    inline GoofSpielObservation() :
        Observation(),
        natureCard_(0), player0LastCard_(0), player1LastCard_(0), roundResult_(PL0_DRAW) {}
    GoofSpielObservation(int initialNumOfCards,
                         const array<int, 3> &chosenCards,
                         GoofspielRoundOutcome roundResult);
    const int natureCard_; // the bidding card
    const int player0LastCard_;
    const int player1LastCard_;
    const GoofspielRoundOutcome roundResult_;
    string toString() const override;
};


class GoofSpielState: public State {
 public:
    inline GoofSpielState(const Domain *domain, array<vector<int>, 3> playerDecks,
                          int natureSelectedCard, array<vector<int>, 3> playedCards) :
        State(domain, hashCombine(98612345434231, playerDecks, playedCards, natureSelectedCard)),
        playerDecks_(move(playerDecks)),
        natureSelectedCard_(natureSelectedCard),
        playedCards_(move(playedCards)) {}

    unsigned long countAvailableActionsFor(Player player) const override;
    vector <shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector <shared_ptr<Action>> &actions) const override;
    vector <Player> getPlayers() const override;
    bool isTerminal() const override;
    string toString() const override;
    bool operator==(const State &rhs) const override;

    const array<vector<int>, 3> playerDecks_;
    const array<vector<int>, 3> playedCards_;
    const int natureSelectedCard_;  // Not in the deck. For the last round it will be NO_NATURE_CARD
};

}  // namespace GTLib2

#endif  // DOMAINS_GOOFSPIEL_H_

#pragma clang diagnostic pop
