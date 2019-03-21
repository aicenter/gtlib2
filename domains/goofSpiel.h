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

#include <experimental/optional>
#include <vector>
#include <string>
#include <utility>
#include "base/base.h"


using std::experimental::nullopt;
using std::experimental::optional;

namespace GTLib2 {
namespace domains {

class GoofSpielAction : public Action {
 public:
  GoofSpielAction(ActionId id, int card);
  string toString() const override;
  bool operator==(const Action &that) const override;
  size_t getHash() const override;
  int cardNumber_;
};

class GoofSpielObservation : public Observation {
 public:
  GoofSpielObservation(int id, optional<int> newBid, optional<int> player1LastCard,
                       optional<int> player2LastCard);

  optional<int> newBid_;
  optional<int> player1LastCard_;
  optional<int> player2LastCard_;
};

class IIGoofSpielObservation : public Observation {
 public:
  IIGoofSpielObservation(int id, optional<int> newBid, optional<int> myLastCard,
                         optional<int> result);

  optional<int> newBid;
  optional<int> myLastCard;
  optional<int> result;
};

class GoofSpielDomain : public Domain {
 public:
  GoofSpielDomain(unsigned int maxDepth, optional<unsigned long int> seed);
  GoofSpielDomain(int numberOfCards, unsigned int maxDepth, optional<unsigned long int> seed);
  string getInfo() const override;
  vector<Player> getPlayers() const override;
  const int numberOfCards;
  const int seed;
};

class IIGoofSpielDomain : public Domain {
 public:
  IIGoofSpielDomain(unsigned int maxDepth, optional<unsigned long int> seed);
  IIGoofSpielDomain(int numberOfCards, unsigned int maxDepth, optional<unsigned long int> seed);
  string getInfo() const override;
  vector<Player> getPlayers() const override;
  const int numberOfCards;
  const int seed;
};

class GoofSpielState : public State {
 public:
  GoofSpielState(Domain *domain, vector<int> player1Deck, vector<int> player2Deck,
                 vector<int> natureDeck, optional<int> natureSelectedCard,
                 double player1CumulativeReward, double player2CumulativeReward,
                 vector<int> player1PlayedCards, vector<int> player2PlayedCards,
                 vector<int> naturePlayedCards);

  GoofSpielState(Domain *domain, const GoofSpielState &previousState, int player1Card,
                 int player2Card, optional<int> newNatureCard,
                 double player1CumulativeReward,
                 double player2CumulativeReward);

  vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
  OutcomeDistribution performActions(
      const vector<PlayerAction> &actions) const override;
  vector<Player> getPlayers() const override;
  string toString() const override;
  bool operator==(const State &rhs) const override;

  size_t getHash() const override;

  vector<int> player1Deck_;
  vector<int> player2Deck_;
  vector<int> natureDeck_;
  vector<int> player1PlayedCards_;
  vector<int> player2PlayedCards_;
  vector<int> naturePlayedCards_;
  optional<int> natureSelectedCard_;  // Not in the deck
  double player1CumulativeReward_;
  double player2CumulativeReward_;
};

class IIGoofSpielState : public GoofSpielState {
 public:
  IIGoofSpielState(Domain *domain, vector<int> player1Deck, vector<int> player2Deck,
                   vector<int> natureDeck, optional<int> natureSelectedCard,
                   double player1CumulativeReward, double player2CumulativeReward,
                   vector<int> player1PlayedCards, vector<int> player2PlayedCards,
                   vector<int> naturePlayedCards);

  IIGoofSpielState(Domain *domain, const GoofSpielState &previousState, int player1Card,
                   int player2Card, optional<int> newNatureCard,
                   double player1CumulativeReward,
                   double player2CumulativeReward);

  OutcomeDistribution performActions(
      const vector<PlayerAction> &actions) const final;
};

}  // namespace domains
}  // namespace GTLib2

#endif  // DOMAINS_GOOFSPIEL_H_

#pragma clang diagnostic pop
