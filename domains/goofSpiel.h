//
// Created by Pavel Rytir on 29/01/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef GTLIB2_GOOFSPIEL_H
#define GTLIB2_GOOFSPIEL_H

#include "../base/base.h"
#include <experimental/optional>
#include <random>

using std::experimental::nullopt;
using std::experimental::optional;

namespace GTLib2 {
    namespace domains {

        class GoofSpielAction : public Action {
        public:
            GoofSpielAction(int id, int card);
            string toString() const override;
            bool operator==(const Action &that) const override;
            size_t getHash() const override;
            int cardNumber;
        };

        class GoofSpielObservation : public Observation {
        public:
            GoofSpielObservation(int id, optional<int> newBid,optional<int> player1LastCard,
                                 optional<int> player2LastCard);

            optional<int> newBid;
            optional<int> player1LastCard;
            optional<int> player2LastCard;
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
            GoofSpielDomain(int maxDepth, optional<unsigned long int> seed);
            GoofSpielDomain(int numberOfCards, int maxDepth, optional<unsigned long int> seed);
            string getInfo() const override;
            vector<int> getPlayers() const override;
            const int numberOfCards;
            const int seed;
        };

      class IIGoofSpielDomain : public Domain {
       public:
        IIGoofSpielDomain(int maxDepth, optional<unsigned long int> seed);
        IIGoofSpielDomain(int numberOfCards, int maxDepth, optional<unsigned long int> seed);
        string getInfo() const override;
        vector<int> getPlayers() const override;
        const int numberOfCards;
        const int seed;
      };


        class GoofSpielState : public State {
        public:
            GoofSpielState(Domain* domain, unordered_set<int> player1Deck, unordered_set<int> player2Deck,
                           unordered_set<int> natureDeck, optional<int> natureSelectedCard,
                           double player1CumulativeReward, double player2CumulativeReward,
                           vector<int> player1PlayedCards, vector<int> player2PlayedCards,
                           vector<int> naturePlayedCards);

            GoofSpielState(Domain* domain, const GoofSpielState& previousState, int player1Card,
                           int player2Card, optional<int> newNatureCard,
                           double player1CumulativeReward,
                            double player2CumulativeReward);

            vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;
            OutcomeDistribution performActions(
                    const vector<pair<int, shared_ptr<Action>>> &actions) const override;
            vector<int> getPlayers() const override ;
            string toString() const override;
            bool operator==(const State &rhs) const override ;

            size_t getHash() const override;

          unordered_set<int> player1Deck;
          unordered_set<int> player2Deck;
          unordered_set<int> natureDeck;
          vector<int> player1PlayedCards;
          vector<int> player2PlayedCards;
          vector<int> naturePlayedCards;
          optional<int> natureSelectedCard; // Not in the deck
          double player1CumulativeReward;
          double player2CumulativeReward;
        };

      class IIGoofSpielState : public GoofSpielState {
       public:
        IIGoofSpielState(Domain* domain, unordered_set<int> player1Deck, unordered_set<int> player2Deck,
                           unordered_set<int> natureDeck, optional<int> natureSelectedCard,
                           double player1CumulativeReward, double player2CumulativeReward,
                           vector<int> player1PlayedCards, vector<int> player2PlayedCards,
                           vector<int> naturePlayedCards);

        IIGoofSpielState(Domain* domain, const GoofSpielState& previousState, int player1Card,
                           int player2Card, optional<int> newNatureCard,
                           double player1CumulativeReward,
                           double player2CumulativeReward);

        OutcomeDistribution performActions(
                const vector<pair<int, shared_ptr<Action>>> &actions) const final;
      };

    }
}



#endif //GTLIB2_GOOFSPIEL_H

#pragma clang diagnostic pop