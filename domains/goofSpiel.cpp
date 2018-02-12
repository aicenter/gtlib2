//
// Created by Pavel Rytir on 29/01/2018.
//

#include <random>
#include "goofSpiel.h"
#include "boost/range/irange.hpp"


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
            //TODO: Fix this. the id generation does not work for the number of card grater than 13
            this->id = newBid.value_or(0) + 14*player1LastCard.value_or(0) + 14*14*player2LastCard.value_or(0);

        }
        GoofSpielDomain::GoofSpielDomain(int maxDepth) : GoofSpielDomain(13, maxDepth) {

        }

        GoofSpielDomain::GoofSpielDomain(int numberOfCards, int maxDepth) : Domain(maxDepth,2){

            auto range = boost::irange(1, numberOfCards + 1);
            unordered_set<int> deck(range.begin(),range.end());

            unordered_map<int,double> rewards;
            rewards[1] = 0.0;
            rewards[2] = 0.0;

            for (auto const i : deck) {
                auto player1Deck = deck;
                auto player2Deck = deck;
                auto natureDeck = deck;

                vector<int> p1;
                vector<int> p2;
                vector<int> n = {i};

                natureDeck.erase(i);

                auto newState = make_shared<GoofSpielState>(player1Deck,player2Deck,natureDeck,i,0.0,0.0,p1,p2,n);
                auto player1Obs = make_shared<GoofSpielObservation>(i,nullopt,nullopt);
                auto player2Obs = make_shared<GoofSpielObservation>(i,nullopt,nullopt);

                unordered_map<int,shared_ptr<Observation>> newObservations;

                newObservations[1] = player1Obs;
                newObservations[2] = player2Obs;

                Outcome outcome(newState, newObservations, rewards);

                rootStatesDistribution.emplace_back(outcome,1.0/deck.size());

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
                                       double player1CumulativeReward, double player2CumulativeReward,
                                       vector<int> player1PlayedCards, vector<int> player2PlayedCards,
                                       vector<int> naturePlayedCards) {
            this->player1Deck = std::move(player1Deck);
            this->player2Deck = std::move(player2Deck);
            this->natureDeck = std::move(natureDeck);
            this->natureSelectedCard = std::move(natureSelectedCard);
            this->player1CumulativeReward = player1CumulativeReward;
            this->player2CumulativeReward = player2CumulativeReward;
            this->player1PlayedCards = std::move(player1PlayedCards);
            this->player2PlayedCards = std::move(player2PlayedCards);
            this->naturePlayedCards = std::move(naturePlayedCards);

        }

        vector<shared_ptr<Action>> GoofSpielState::getAvailableActionsFor(const int player) const {
            const auto& deck = player == 1 ? player1Deck : player2Deck;

            vector<shared_ptr<Action>> actions;

            for (const int cardNumber : deck) {
                auto action = make_shared<GoofSpielAction>(cardNumber);
                actions.push_back(action);
            }

            return actions;
        }

        OutcomeDistribution
        GoofSpielState::performActions(const unordered_map<int, shared_ptr<Action>> &actions) const {
            const auto player1Action = std::dynamic_pointer_cast<GoofSpielAction>(actions.at(1));
            const auto player2Action = std::dynamic_pointer_cast<GoofSpielAction>(actions.at(2));

            const int p1Card = player1Action->cardNumber;
            const int p2Card = player2Action->cardNumber;

            auto newPlayer1Deck = unordered_set<int>(player1Deck.begin(),player1Deck.end());
            newPlayer1Deck.erase(player1Action->cardNumber);

            auto newPlayer1PlayedCards = player1PlayedCards;
            newPlayer1PlayedCards.push_back(player1Action->cardNumber);

            auto newPlayer2Deck = unordered_set<int>(player2Deck.begin(),player2Deck.end());
            newPlayer2Deck.erase(player2Action->cardNumber);

            auto newPlayer2PlayedCards = player2PlayedCards;
            newPlayer2PlayedCards.push_back(player2Action->cardNumber);

            OutcomeDistribution newOutcomes;

            unordered_map<int,double> newRewards;
            const double thisRoundRewardP1 = p1Card == p2Card ?
                                   ((double) *natureSelectedCard) / 2 : p1Card > p2Card ? *natureSelectedCard : 0.0;

            const double thisRoundRewardP2 = p1Card == p2Card ?
                            ((double) *natureSelectedCard) / 2 : p2Card > p1Card ? *natureSelectedCard : 0.0;

            newRewards[1] = player1CumulativeReward + thisRoundRewardP1;
            newRewards[2] = player2CumulativeReward + thisRoundRewardP2;


            if (natureDeck.empty()) {
                //final state
                const auto newNatureDeck = unordered_set<int>();

                const auto newState = make_shared<GoofSpielState>(newPlayer1Deck,
                                                            newPlayer2Deck,
                                                            newNatureDeck,
                                                            nullopt,
                                                            newRewards[1],
                                                            newRewards[2],
                                                            newPlayer1PlayedCards,
                                                            newPlayer2PlayedCards,
                                                            naturePlayedCards);

                const auto player1Observation = make_shared<GoofSpielObservation>(nullopt, p1Card, p2Card);
                const auto player2Observation = make_shared<GoofSpielObservation>(nullopt, p1Card, p2Card);

                unordered_map<int, shared_ptr<Observation>> newObservations;
                newObservations[1] = player1Observation;
                newObservations[2] = player2Observation;

                const auto newOutcome = Outcome(newState, newObservations, newRewards);
                newOutcomes.emplace_back(newOutcome,1.0);


            } else {
                for (const auto natureCard : natureDeck) {
                    auto newNatureDeck = unordered_set<int>(natureDeck.begin(), natureDeck.end());
                    newNatureDeck.erase(natureCard);
                    auto newNaturePlayedCards = naturePlayedCards;
                    newNaturePlayedCards.push_back(natureCard);

                    const auto newState = make_shared<GoofSpielState>(newPlayer1Deck,
                                                                newPlayer2Deck,
                                                                newNatureDeck,
                                                                natureCard,
                                                                newRewards[1],
                                                                newRewards[2],
                                                                newPlayer1PlayedCards,
                                                                newPlayer2PlayedCards,
                                                                newNaturePlayedCards);

                    const auto player1Observation = make_shared<GoofSpielObservation>(natureCard, p1Card, p2Card);
                    const auto player2Observation = make_shared<GoofSpielObservation>(natureCard, p1Card, p2Card);

                    unordered_map<int, shared_ptr<Observation>> newObservations;
                    newObservations[1] = player1Observation;
                    newObservations[2] = player2Observation;

                    const auto newOutcome = Outcome(newState, newObservations, newRewards);
                    newOutcomes.emplace_back(newOutcome,1.0 / natureDeck.size());
                }
            }

            return newOutcomes;
        }

        unordered_set<int> GoofSpielState::getPlayersSet() const {
            unordered_set<int> players;
            if (!player1Deck.empty()) {
                players.insert(1);
            }

            if (!player2Deck.empty()) {
                players.insert(2);
            }

            return players;
        }

        string GoofSpielState::toString() const {
            string ret = "p1played: ";
            for (int card : player1PlayedCards) {
                ret.append(" " + std::to_string(card) + " ");
            }
            ret.append(" p2played: ");
            for (auto card : player2PlayedCards) {
                ret.append(" " + std::to_string(card) + " ");
            }
            ret.append(" naturePlayed ");
            for (auto card : naturePlayedCards) {
                ret.append(" " + std::to_string(card) + " ");
            }
            return ret;
        }

        bool GoofSpielState::operator==(const State &rhs) const {
            auto gsState = dynamic_cast<const GoofSpielState&>(rhs);

            return player1Deck == gsState.player1Deck && player2Deck == gsState.player2Deck &&
                    natureDeck == gsState.natureDeck && natureSelectedCard == gsState.natureSelectedCard &&
                    player1CumulativeReward == gsState.player1CumulativeReward &&
                    player2CumulativeReward == gsState.player2CumulativeReward &&
                    player1PlayedCards == gsState.player1PlayedCards &&
                    player2PlayedCards == gsState.player2PlayedCards &&
                    naturePlayedCards == gsState.naturePlayedCards;
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