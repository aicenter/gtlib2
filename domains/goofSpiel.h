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
            explicit GoofSpielAction(int id, int card);
            string toString() const override;

            int cardNumber;
            int id;
        };

        class GoofSpielObservation : public Observation {
        public:
            GoofSpielObservation(optional<int> newBid,optional<int> player1LastCard,
                                 optional<int> player2LastCard);

            optional<int> newBid;
            optional<int> player1LastCard;
            optional<int> player2LastCard;
        };

        class GoofSpielDomain : public Domain {
        public:
            explicit GoofSpielDomain(int maxDepth);
            GoofSpielDomain(int numberOfCards, int maxDepth);
            string getInfo() const override;
            vector<int> getPlayers() const override;
        };

        class SeedGoofSpielDomain : public Domain {
         public:
          SeedGoofSpielDomain(int maxDepth, unsigned long int seed);
          SeedGoofSpielDomain(int numberOfCards, int maxDepth, unsigned long int seed);
          string getInfo() const override;
          vector<int> getPlayers() const override;
        };

        class GoofSpielState : public State {
        public:
            GoofSpielState(const shared_ptr<Domain> &domain, unordered_set<int> player1Deck, unordered_set<int> player2Deck,
                           unordered_set<int> natureDeck, optional<int> natureSelectedCard,
                           double player1CumulativeReward, double player2CumulativeReward,
                           vector<int> player1PlayedCards, vector<int> player2PlayedCards,
                           vector<int> naturePlayedCards);

            GoofSpielState(const shared_ptr<Domain> &domain, const GoofSpielState& previousState, int player1Card,
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

        class SeedGoofSpielState : public GoofSpielState {
         public:
          SeedGoofSpielState(const shared_ptr<Domain> &domain, unordered_set<int> player1Deck, unordered_set<int> player2Deck,
                  unordered_set<int> natureDeck, optional<int> natureSelectedCard,
          double player1CumulativeReward, double player2CumulativeReward,
                  vector<int> player1PlayedCards, vector<int> player2PlayedCards,
                  vector<int> naturePlayedCards);

          SeedGoofSpielState(const shared_ptr<Domain> &domain, const GoofSpielState& previousState, int player1Card,
                         int player2Card, optional<int> newNatureCard,
                         double player1CumulativeReward,
                         double player2CumulativeReward);

          OutcomeDistribution performActions(
                  const vector<pair<int, shared_ptr<Action>>> &actions) const override;

        };

    }
}



#endif //GTLIB2_GOOFSPIEL_H

#pragma clang diagnostic pop