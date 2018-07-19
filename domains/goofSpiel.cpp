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
                                        vector<int> naturePlayedCards) : State() {
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

        GoofSpielState::GoofSpielState(const GoofSpielState &previousState, int player1Card, int player2Card,
                                       optional<int> newNatureCard, double player1CumulativeReward,
                                       double player2CumulativeReward) : State() {
            player1Deck = previousState.player1Deck;
            player1Deck.erase(player1Card);
            player2Deck = previousState.player2Deck;
            player2Deck.erase(player2Card);

            player1PlayedCards = previousState.player1PlayedCards;
            player1PlayedCards.push_back(player1Card);

            player2PlayedCards = previousState.player2PlayedCards;
            player2PlayedCards.push_back(player2Card);

            this->player1CumulativeReward = player1CumulativeReward;
            this->player2CumulativeReward = player2CumulativeReward;

            if (newNatureCard == nullopt) {
                natureDeck = unordered_set<int>();
                naturePlayedCards = previousState.naturePlayedCards;
                natureSelectedCard = nullopt;
            } else {
                natureDeck = previousState.natureDeck;
                natureDeck.erase(*newNatureCard);

                naturePlayedCards = previousState.naturePlayedCards;
                naturePlayedCards.push_back(*newNatureCard);

                natureSelectedCard = newNatureCard;
            }
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

            OutcomeDistribution newOutcomes;

            unordered_map<int,double> newRewards;
            const double thisRoundRewardP1 = p1Card == p2Card ?
                                   ((double) *natureSelectedCard) / 2 : p1Card > p2Card ? *natureSelectedCard : 0.0;

            const double thisRoundRewardP2 = p1Card == p2Card ?
                            ((double) *natureSelectedCard) / 2 : p2Card > p1Card ? *natureSelectedCard : 0.0;

            newRewards[1] = player1CumulativeReward + thisRoundRewardP1;
            newRewards[2] = player2CumulativeReward + thisRoundRewardP2;


            if (natureDeck.empty()) {
                const auto newStatex = make_shared<GoofSpielState>(*this, p1Card, p2Card, nullopt,
                                                                   newRewards[1],
                                                                   newRewards[2]);

                const auto player1Observation = make_shared<GoofSpielObservation>(nullopt, p1Card, p2Card);
                const auto player2Observation = make_shared<GoofSpielObservation>(nullopt, p1Card, p2Card);

                unordered_map<int, shared_ptr<Observation>> newObservations;
                newObservations[1] = player1Observation;
                newObservations[2] = player2Observation;

                const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
                newOutcomes.emplace_back(newOutcome,1.0);


            } else {
                for (const auto natureCard : natureDeck) {

                    const auto newStatex = make_shared<GoofSpielState>(*this, p1Card, p2Card, natureCard,
                                                                       newRewards[1],
                                                                       newRewards[2]);


                    const auto player1Observation = make_shared<GoofSpielObservation>(natureCard, p1Card, p2Card);
                    const auto player2Observation = make_shared<GoofSpielObservation>(natureCard, p1Card, p2Card);

                    unordered_map<int, shared_ptr<Observation>> newObservations;
                    newObservations[1] = player1Observation;
                    newObservations[2] = player2Observation;

                    const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
                    newOutcomes.emplace_back(newOutcome,1.0 / natureDeck.size());
                }
            }

            return newOutcomes;
        }

        vector<int> GoofSpielState::getPlayers() const {
            if (!player1Deck.empty() && !player2Deck.empty()) {
                return {1,2};
            } else {
                return {};
            }
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

            return  natureSelectedCard == gsState.natureSelectedCard &&
                    player1CumulativeReward == gsState.player1CumulativeReward &&
                    player2CumulativeReward == gsState.player2CumulativeReward &&
                    player1PlayedCards == gsState.player1PlayedCards &&
                    player2PlayedCards == gsState.player2PlayedCards &&
                    naturePlayedCards == gsState.naturePlayedCards &&
            player1Deck == gsState.player1Deck && player2Deck == gsState.player2Deck &&
            natureDeck == gsState.natureDeck;
        }

        size_t GoofSpielState::getHash() const {
            size_t seed = 0;
            for (auto i : player1PlayedCards) {
                boost::hash_combine(seed, i);
            }
            for (auto i : player2PlayedCards) {
                boost::hash_combine(seed, i);
            }
            for (auto i : naturePlayedCards) {
                boost::hash_combine(seed, i);
            }
            boost::hash_combine(seed, player1CumulativeReward);
            boost::hash_combine(seed, player2CumulativeReward);
            return seed;
        }


      SeedGoofSpielDomain::SeedGoofSpielDomain(int maxDepth, unsigned long int seed):
              SeedGoofSpielDomain(13, maxDepth, seed) {}

      SeedGoofSpielDomain::SeedGoofSpielDomain(int numberOfCards, int maxDepth, unsigned long int seed):
              Domain(maxDepth,2) {
          auto range = vector<int>(numberOfCards);
          std::iota(range.begin(), range.end(), 1);
          std::default_random_engine eng{seed};
          std::mt19937 randEng(eng());
          std::shuffle(range.begin(), range.end(), randEng);
          unordered_set<int> deck(range.begin(),range.end());

          unordered_map<int,double> rewards;
          rewards[1] = 0.0;
          rewards[2] = 0.0;

          auto player1Deck = deck;
          auto player2Deck = deck;
          auto natureDeck = deck;
          auto firstCard = natureDeck.begin();
          cout << *firstCard <<"prvni karta\n";
          natureDeck.erase(firstCard);
          vector<int> p1;
          vector<int> p2;
          vector<int> n = {*firstCard};

          auto newState = make_shared<SeedGoofSpielState>(player1Deck,player2Deck,natureDeck,*firstCard,0.0,0.0,p1,p2,n);
          auto player1Obs = make_shared<GoofSpielObservation>(*firstCard,nullopt,nullopt);
          auto player2Obs = make_shared<GoofSpielObservation>(*firstCard,nullopt,nullopt);

          unordered_map<int,shared_ptr<Observation>> newObservations;

          newObservations[1] = player1Obs;
          newObservations[2] = player2Obs;

          Outcome outcome(newState, newObservations, rewards);
          rootStatesDistribution.emplace_back(outcome,1.0);

        }

      string SeedGoofSpielDomain::getInfo() const {
          return "Goof spiel. Max depth is: " + std::to_string(maxDepth);
      }

      vector<int> SeedGoofSpielDomain::getPlayers() const {
          return {1,2};
      }

      SeedGoofSpielState::SeedGoofSpielState(unordered_set<int> player1Deck,
                                             unordered_set<int> player2Deck,
                                             unordered_set<int> natureDeck,
                                             optional<int> natureSelectedCard,
                                             double player1CumulativeReward,
                                             double player2CumulativeReward,
                                             vector<int> player1PlayedCards,
                                             vector<int> player2PlayedCards,
                                             vector<int> naturePlayedCards) : GoofSpielState(
              move(player1Deck), move(player2Deck), move(natureDeck), move(natureSelectedCard), player1CumulativeReward,
              player2CumulativeReward, move(player1PlayedCards), move(player2PlayedCards), move(naturePlayedCards)) {}

      SeedGoofSpielState::SeedGoofSpielState(const GoofSpielState &previousState, int player1Card,
                                             int player2Card, optional<int> newNatureCard,
                                             double player1CumulativeReward,
                                             double player2CumulativeReward) : GoofSpielState(
              previousState, player1Card, player2Card, move(newNatureCard), player1CumulativeReward,
              player2CumulativeReward) {}

      OutcomeDistribution SeedGoofSpielState::performActions(
              const unordered_map<int, shared_ptr<Action>> &actions) const {
        const auto player1Action = std::dynamic_pointer_cast<GoofSpielAction>(actions.at(1));
        const auto player2Action = std::dynamic_pointer_cast<GoofSpielAction>(actions.at(2));

        const int p1Card = player1Action->cardNumber;
        const int p2Card = player2Action->cardNumber;

        OutcomeDistribution newOutcomes;

        unordered_map<int,double> newRewards;
        const double thisRoundRewardP1 = p1Card == p2Card ?
                                         ((double) *natureSelectedCard) / 2 : p1Card > p2Card ? *natureSelectedCard : 0.0;

        const double thisRoundRewardP2 = p1Card == p2Card ?
                                         ((double) *natureSelectedCard) / 2 : p2Card > p1Card ? *natureSelectedCard : 0.0;

        newRewards[1] = player1CumulativeReward + thisRoundRewardP1;
        newRewards[2] = player2CumulativeReward + thisRoundRewardP2;

        unordered_map<int, shared_ptr<Observation>> newObservations;
        if (natureDeck.empty()) {
          const auto newStatex = make_shared<GoofSpielState>(*this, p1Card, p2Card, nullopt,
                                                             newRewards[1],
                                                             newRewards[2]);

          newObservations[1] = make_shared<GoofSpielObservation>(nullopt, p1Card, p2Card);
          newObservations[2] = make_shared<GoofSpielObservation>(nullopt, p1Card, p2Card);

          const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
          newOutcomes.emplace_back(newOutcome,1.0);


        } else {
          auto natureCard = *natureDeck.begin();

          const auto newStatex = make_shared<GoofSpielState>(*this, p1Card, p2Card, natureCard,
                                                             newRewards[1],
                                                             newRewards[2]);

          newObservations[1] = make_shared<GoofSpielObservation>(natureCard, p1Card, p2Card);
          newObservations[2] = make_shared<GoofSpielObservation>(natureCard, p1Card, p2Card);

          const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
          newOutcomes.emplace_back(newOutcome,1.0);
        }

        return newOutcomes;
      }

    }
}
#pragma clang diagnostic pop