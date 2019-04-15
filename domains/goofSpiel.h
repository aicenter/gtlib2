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

#include <vector>
#include <string>
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

    void shuffleChanceCards(int seed);
    vector<int> getNatureCards();
};

class GoofSpielDomain: public Domain {
 public:
    explicit GoofSpielDomain(GoofSpielSettings settings);
    string getInfo() const override;
    vector<Player> getPlayers() const override;
    const int numberOfCards_;
    const bool fixChanceCards_;
    const bool binaryTerminalRewards_;
    const GoofSpielVariant variant_;
    const vector<int> natureCards_;

 private:
    void initRandomCards(const vector<int> &natureCards);
    void initFixedCards(const vector<int> &natureCards);
};


class GoofSpielAction: public Action {
 public:
    GoofSpielAction(ActionId id, int card);
    string toString() const override;
    bool operator==(const Action &that) const override;
    size_t getHash() const override;
    int cardNumber_;
};

constexpr int NO_NATURE_CARD = 0;
constexpr int NO_CARD_OBSERVATION = 0;

class GoofSpielObservation: public Observation {
 public:
    GoofSpielObservation(int initialNumOfCards,
                         const std::array<int, 3> &chosenCards,
                         int roundResult);
    const int natureCard_; // the bidding card
    const int player0LastCard_;
    const int player1LastCard_;
    const int roundResult_; // for player 0 -- 0 draw, 1 win, -1 lose
};


class GoofSpielState: public State {
 public:
    GoofSpielState(Domain *domain,
                   std::array<vector<int>, 3> playerDecks,
                   int natureSelectedCard,
                   vector<double> cumulativeRewards,
                   std::array<vector<int>, 3> playedCards);
    GoofSpielState(Domain *domain,
                   const GoofSpielState &previousState,
                   std::array<int, 3> roundPlayedCards,
                   vector<double> cumulativeRewards);

    unsigned long countAvailableActionsFor(Player player) const override;
    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector<PlayerAction> &actions) const override;
    vector<Player> getPlayers() const override;
    string toString() const override;
    bool operator==(const State &rhs) const override;
    size_t getHash() const override;

    std::array<vector<int>, 3> playerDecks_;
    std::array<vector<int>, 3> playedCards_;
    int natureSelectedCard_;  // Not in the deck. For the last round it will be NO_NATURE_CARD
    vector<double> cumulativeRewards_;
};

}  // namespace GTLib2

#endif  // DOMAINS_GOOFSPIEL_H_

#pragma clang diagnostic pop
