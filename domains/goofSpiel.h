//
// Created by Pavel Rytir on 29/01/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef GTLIB2_GOOFSPIEL_H
#define GTLIB2_GOOFSPIEL_H

#include "../base/base.h"
#include <experimental/optional>

using std::experimental::nullopt;
using std::experimental::optional;

namespace GTLib2 {
    namespace domains {

        class GoofSpielAction : public Action {
        public:
            explicit GoofSpielAction(int card);
            string toString() const override;

            int cardNumber;
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
            string getInfo() const override;
            vector<int> getPlayers() const override;
        };

        class GoofSpielState : public State {
        public:
            GoofSpielState(unordered_set<int> player1Deck, unordered_set<int> player2Deck,
                           unordered_set<int> natureDeck, optional<int> natureSelectedCard,
                           double player1CommulativeReward, double player2CommulativeReward);
            vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;
            OutcomeDistribution performActions(
                    const unordered_map<int, shared_ptr<Action>> &actions) const override;
            unordered_set<int> getPlayersSet() const override ;
            string toString() const override;
            bool operator==(const State &rhs) const override ;

            size_t getHash() const override;

            double player1CommulativeReward;
            double player2CommulativeReward;

            unordered_set<int> player1Deck;
            unordered_set<int> player2Deck;
            unordered_set<int> natureDeck;
            optional<int> natureSelectedCard; // Not in the deck

        };

    }
}



#endif //GTLIB2_GOOFSPIEL_H

#pragma clang diagnostic pop