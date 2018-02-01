//
// Created by Pavel Rytir on 29/01/2018.
//

#include <random>
#include "goofSpiel.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
namespace GTLib2 {
    namespace domains {


        GoofSpielAction::GoofSpielAction(int card) : Action(card) {
            cardNumber = card;
        }

        string GoofSpielAction::toString() const {
            return "Card number: " + std::to_string(cardNumber);
        }

        GoofSpielObservation::GoofSpielObservation(optional<int> newBid, optional<int> player1LastCard,
                                                   optional<int> player2LastCard) :
                Observation(1) {
            this->newBid = newBid;
            this->player1LastCard = player1LastCard;
            this->player2LastCard = player2LastCard;
            this->id = newBid.value_or(0) + 14*player1LastCard.value_or(0) + 14*14*player2LastCard.value_or(0);

        }

        GoofSpielDomain::GoofSpielDomain(int maxDepth) : Domain(maxDepth,2){
            unordered_set<int> deck = {1,2,3,4,5,6,7,8,9,10,11,12,13};
            unordered_map<int,double> rewards;
            rewards[1] = 0.0;
            rewards[2] = 0.0;

            for (auto i : deck) {
                auto player1Deck = deck;
                auto player2Deck = deck;
                auto natureDeck = deck;

                natureDeck.erase(i);

                auto newState = make_shared<GoofSpielState>(player1Deck,player2Deck,natureDeck,i,0.0,0.0);
                auto player1Obs = make_shared<GoofSpielObservation>(i,nullopt,nullopt);
                auto player2Obs = make_shared<GoofSpielObservation>(i,nullopt,nullopt);

                unordered_map<int,shared_ptr<Observation>> newObservations;

                newObservations[1] = player1Obs;
                newObservations[2] = player2Obs;

                Outcome outcome(newState, newObservations, rewards);

                rootStatesDistribution[outcome] = 1.0/deck.size();

            }
        }

        string GoofSpielDomain::getInfo() const {
            return "Goof spiel. Max depth is: " + std::to_string(maxDepth);
        }

        vector<int> GoofSpielDomain::getPlayers() const {
            return {1,2};
        }

        GoofSpielState::GoofSpielState(unordered_set<int> player1Deck, unordered_set<int> player2Deck,
                                       unordered_set<int> natureDeck, optional<int> natureSelectedCard,
                                       double player1CumulativeReward, double player2CumulativeReward) {
            this->player1Deck = std::move(player1Deck);
            this->player2Deck = std::move(player2Deck);
            this->natureDeck = std::move(natureDeck);
            this->natureSelectedCard = natureSelectedCard;
            this->player1CumulativeReward = player1CumulativeReward;
            this->player2CumulativeReward = player2CumulativeReward;

        }

        vector<shared_ptr<Action>> GoofSpielState::getAvailableActionsFor(const int player) const {
            auto& deck = player == 1 ? player1Deck : player2Deck;

            vector<shared_ptr<Action>> actions;

            for (int cardNumber : deck) {
                auto action = make_shared<GoofSpielAction>(cardNumber);
                actions.push_back(action);
            }

            return actions;
        }

        OutcomeDistribution
        GoofSpielState::performActions(const unordered_map<int, shared_ptr<Action>> &actions) const {
            auto player1Action = std::dynamic_pointer_cast<GoofSpielAction>(actions.at(1));
            auto player2Action = std::dynamic_pointer_cast<GoofSpielAction>(actions.at(2));

            int p1Card = player1Action->cardNumber;
            int p2Card = player2Action->cardNumber;

            auto newPlayer1Deck = unordered_set<int>(player1Deck.begin(),player1Deck.end());
            newPlayer1Deck.erase(player1Action->cardNumber);

            auto newPlayer2Deck = unordered_set<int>(player2Deck.begin(),player2Deck.end());
            newPlayer2Deck.erase(player2Action->cardNumber);

            OutcomeDistribution newOutcomes;

            unordered_map<int,double> newRewards;
            double thisRoundRewardP1 = p1Card == p2Card ?
                                   ((double) *natureSelectedCard) / 2 : p1Card > p2Card ? *natureSelectedCard : 0.0;

            double thisRoundRewardP2 = p1Card == p2Card ?
                            ((double) *natureSelectedCard) / 2 : p2Card > p1Card ? *natureSelectedCard : 0.0;

            newRewards[1] = player1CumulativeReward + thisRoundRewardP1;
            newRewards[2] = player2CumulativeReward + thisRoundRewardP2;


            for (auto natureCard : natureDeck) {
                auto newNatureDeck = unordered_set<int>(natureDeck.begin(),natureDeck.end());
                newNatureDeck.erase(natureCard);
                auto newState = make_shared<GoofSpielState>(newPlayer1Deck,
                                                            newPlayer2Deck,
                                                            newNatureDeck,
                                                            natureCard,
                newRewards[1],
                newRewards[2]);

                auto player1Observation = make_shared<GoofSpielObservation>(natureCard,p1Card,p2Card);
                auto player2Observation = make_shared<GoofSpielObservation>(natureCard,p1Card,p2Card);

                unordered_map<int,shared_ptr<Observation>> newObservations;
                newObservations[1] = player1Observation;
                newObservations[2] = player2Observation;

                auto newOutcome = Outcome(newState, newObservations, newRewards);
                newOutcomes[newOutcome] = 1.0/natureDeck.size();
            }

            return newOutcomes;
        }

        unordered_set<int> GoofSpielState::getPlayersSet() const {
            if (!player1Deck.empty() && !player2Deck.empty()) {
                return {1,2};
            } else {
                return {};
            }
        }

        string GoofSpielState::toString() const {
            return "Number of cards left: " + std::to_string(player1Deck.size());
        }

        bool GoofSpielState::operator==(const State &rhs) const {
            auto gsState = dynamic_cast<const GoofSpielState&>(rhs);
            return player1Deck == gsState.player1Deck && player2Deck == gsState.player2Deck &&
                                                         natureDeck == gsState.natureDeck;
        }

        size_t GoofSpielState::getHash() const {
            size_t seed = 0;
            for (auto i : player1Deck) {
                boost::hash_combine(seed, i);
            }
            for (auto i : player2Deck) {
                boost::hash_combine(seed, i);
            }
            for (auto i : natureDeck) {
                boost::hash_combine(seed, i);
            }
            boost::hash_combine(seed, player1CumulativeReward);
            boost::hash_combine(seed, player2CumulativeReward);
            return seed;
        }
    }
}
#pragma clang diagnostic pop