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


#include <random>
#include "domains/goofSpiel.h"
#include "boost/range/irange.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
namespace GTLib2 {
namespace domains {
GoofSpielAction::GoofSpielAction(ActionId id, int card) : Action(id) {
  cardNumber_ = card;
}

string GoofSpielAction::toString() const {
  return "Card: " + std::to_string(cardNumber_);
}

size_t GoofSpielAction::getHash() const {
  std::hash<int> h;
  return h(cardNumber_);
}

bool GoofSpielAction::operator==(const Action &that) const {
  if (typeid(that) == typeid(*this)) {
    const auto rhsAction = static_cast<const GoofSpielAction *>(&that);
    return cardNumber_ == rhsAction->cardNumber_;
  }
  return false;
}

GoofSpielObservation::GoofSpielObservation(int id,
                                           optional<int> newBid,
                                           optional<int> player1LastCard,
                                           optional<int> player2LastCard) :
    Observation(id) {
  newBid_ = std::move(newBid);
  player1LastCard_ = std::move(player1LastCard);
  player2LastCard_ = std::move(player2LastCard);
}

IIGoofSpielObservation::IIGoofSpielObservation(int id, optional<int> newBid,
                                               optional<int> myLastCard, optional<int> result) :
    Observation(id), newBid_(move(newBid)), myLastCard_(move(myLastCard)),
    result_(move(result)) {}

GoofSpielDomain::GoofSpielDomain(unsigned int maxDepth, optional<unsigned long int> seed) :
    GoofSpielDomain(13, maxDepth, move(seed)) {}

GoofSpielDomain::GoofSpielDomain(int numberOfCards, unsigned int maxDepth,
                                 optional<unsigned long int> seed) :
    Domain(maxDepth, 2), numberOfCards_(numberOfCards), seed_(seed ? 1 : 0) {
  if (seed) {
    auto range = vector<int>(static_cast<unsigned int>(numberOfCards));
    std::iota(range.begin(), range.end(), 1);
    for (auto &i : range) {
      maxUtility_ += i;
    }
    std::default_random_engine eng{*seed};
    std::mt19937 randEng(eng());
    std::shuffle(range.begin(), range.end(), randEng);
    vector<int> deck(range.begin(), range.end());
    vector<double> rewards(2);
    auto player1Deck = deck;
    auto player2Deck = deck;
    auto natureDeck = deck;
    const int firstCard = *natureDeck.begin();
    natureDeck.erase(natureDeck.begin());
    vector<int> p1;
    vector<int> p2;
    vector<int> n = {firstCard};

    auto newState = make_shared<GoofSpielState>(this, player1Deck, player2Deck, natureDeck,
                                                firstCard, 0.0, 0.0, p1, p2, n);
    auto player1Obs = make_shared<GoofSpielObservation>(firstCard, firstCard, nullopt, nullopt);
    auto player2Obs = make_shared<GoofSpielObservation>(firstCard, firstCard, nullopt, nullopt);

    vector<shared_ptr<Observation>> newObservations{player1Obs, player2Obs};

    Outcome outcome(newState, newObservations, rewards);
    rootStatesDistribution_.emplace_back(outcome, 1.0);
  } else {
    auto range = boost::irange(1, numberOfCards + 1);
    vector<int> deck(range.begin(), range.end());
    for (auto &i : deck) {
      maxUtility_ += i;
    }
    vector<double> rewards(2);
    for (auto const i : range) {
      auto player1Deck = deck;
      auto player2Deck = deck;
      auto natureDeck = deck;

      vector<int> p1;
      vector<int> p2;
      vector<int> n = {i};

      natureDeck.erase(natureDeck.begin() + i - 1);

      auto newState = make_shared<GoofSpielState>(this, player1Deck, player2Deck,
                                                  natureDeck, i, 0.0, 0.0, p1, p2, n);
      auto player1Obs = make_shared<GoofSpielObservation>(i, i, nullopt, nullopt);
      auto player2Obs = make_shared<GoofSpielObservation>(i, i, nullopt, nullopt);

      vector<shared_ptr<Observation>> newObservations{player1Obs, player2Obs};

      Outcome outcome(newState, newObservations, rewards);
      rootStatesDistribution_.emplace_back(outcome, 1.0 / deck.size());
    }
  }
}

string GoofSpielDomain::getInfo() const {
  return "Goof spiel. Max depth is: " + std::to_string(maxDepth_);
}

vector<Player> GoofSpielDomain::getPlayers() const {
  return {0, 1};
}

GoofSpielState::GoofSpielState(Domain *domain,
                               vector<int> player1Deck, vector<int> player2Deck,
                               vector<int> natureDeck, optional<int> natureSelectedCard,
                               double player1CumulativeReward, double player2CumulativeReward,
                               vector<int> player1PlayedCards, vector<int> player2PlayedCards,
                               vector<int> naturePlayedCards) : State(domain) {
  player1Deck_ = std::move(player1Deck);
  player2Deck_ = std::move(player2Deck);
  natureDeck_ = std::move(natureDeck);
  natureSelectedCard_ = std::move(natureSelectedCard);
  player1CumulativeReward_ = player1CumulativeReward;
  player2CumulativeReward_ = player2CumulativeReward;
  player1PlayedCards_ = std::move(player1PlayedCards);
  player2PlayedCards_ = std::move(player2PlayedCards);
  naturePlayedCards_ = std::move(naturePlayedCards);
}

GoofSpielState::GoofSpielState(Domain *domain,
                               const GoofSpielState &previousState,
                               int player1Card,
                               int player2Card,
                               optional<int> newNatureCard,
                               double player1CumulativeReward,
                               double player2CumulativeReward) : State(domain) {
  player1Deck_ = previousState.player1Deck_;
  player1Deck_.erase(std::remove(player1Deck_.begin(), player1Deck_.end(),
      player1Card), player1Deck_.end());
  player2Deck_ = previousState.player2Deck_;
  player2Deck_.erase(std::remove(player2Deck_.begin(), player2Deck_.end(),
      player2Card), player2Deck_.end());

  player1PlayedCards_ = previousState.player1PlayedCards_;
  player1PlayedCards_.push_back(player1Card);

  player2PlayedCards_ = previousState.player2PlayedCards_;
  player2PlayedCards_.push_back(player2Card);

  player1CumulativeReward_ = player1CumulativeReward;
  player2CumulativeReward_ = player2CumulativeReward;

  if (newNatureCard == nullopt) {
    natureDeck_ = vector<int>();
    naturePlayedCards_ = previousState.naturePlayedCards_;
    natureSelectedCard_ = nullopt;
  } else {
    natureDeck_ = previousState.natureDeck_;
    natureDeck_.erase(std::remove(natureDeck_.begin(), natureDeck_.end(),
        *newNatureCard), natureDeck_.end());

    naturePlayedCards_ = previousState.naturePlayedCards_;
    naturePlayedCards_.push_back(*newNatureCard);
    natureSelectedCard_ = newNatureCard;
  }
}

// todo: this is only hotfix
int GoofSpielState::countAvailableActionsFor(Player player) const {
  return int(getAvailableActionsFor(player).size());
}

vector<shared_ptr<Action>> GoofSpielState::getAvailableActionsFor(const Player player) const {
  const auto &deck = player == 0 ? player1Deck_ : player2Deck_;

  vector<shared_ptr<Action>> actions;
  int id = 0;
  for (const int cardNumber : deck) {
    auto action = make_shared<GoofSpielAction>(id, cardNumber);
    actions.push_back(action);
    id++;
  }
  return actions;
}

OutcomeDistribution
GoofSpielState::performActions(const vector<PlayerAction> &actions) const {
  const auto goofdomain = static_cast<GoofSpielDomain *>(domain_);
  const auto player1Action = dynamic_cast<GoofSpielAction*>(actions[0].second.get());
  const auto player2Action = dynamic_cast<GoofSpielAction*>(actions[1].second.get());
  const int p1Card = player1Action->cardNumber_;
  const int p2Card = player2Action->cardNumber_;

  OutcomeDistribution newOutcomes;

  const double thisRoundRewardP1 = p1Card == p2Card ? 0.0
      : p1Card > p2Card ? *natureSelectedCard_ : -*natureSelectedCard_;
  const double thisRoundRewardP2 = -thisRoundRewardP1;

  vector<double> newRewards
      {player1CumulativeReward_ + thisRoundRewardP1, player2CumulativeReward_ + thisRoundRewardP2};

  if (natureDeck_.empty()) {
    const auto newStatex = make_shared<GoofSpielState>(domain_, *this, p1Card, p2Card,
                                                       nullopt, newRewards[0], newRewards[1]);

    const auto player1Observation = make_shared<GoofSpielObservation>(
        goofdomain->numberOfCards_ * p1Card +
            goofdomain->numberOfCards_ * goofdomain->numberOfCards_ * p2Card,
        nullopt, p1Card, p2Card);
    const auto player2Observation = make_shared<GoofSpielObservation>(
        goofdomain->numberOfCards_ * p1Card +
            goofdomain->numberOfCards_ * goofdomain->numberOfCards_ * p2Card,
        nullopt, p1Card, p2Card);

    vector<shared_ptr<Observation>> newObservations{player1Observation, player2Observation};
    const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
    newOutcomes.emplace_back(newOutcome, 1.0);

  } else {
    if (goofdomain->seed_) {
      auto natureCard = *natureDeck_.begin();
      const auto newStatex = make_shared<GoofSpielState>(domain_, *this, p1Card, p2Card,
                                                         natureCard, newRewards[0], newRewards[1]);

      vector<shared_ptr<Observation>> newObservations{
          make_shared<GoofSpielObservation>(natureCard +
              goofdomain->numberOfCards_ * p1Card + goofdomain->numberOfCards_ *
              goofdomain->numberOfCards_ * p2Card, natureCard, p1Card, p2Card),
          make_shared<GoofSpielObservation>(natureCard +
              goofdomain->numberOfCards_ * p1Card + goofdomain->numberOfCards_ *
              goofdomain->numberOfCards_ * p2Card, natureCard, p1Card, p2Card)};

      const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
      newOutcomes.emplace_back(newOutcome, 1.0);
    } else {
      for (const auto natureCard : natureDeck_) {
        const auto newStatex = make_shared<GoofSpielState>(domain_,
                                                           *this,
                                                           p1Card,
                                                           p2Card,
                                                           natureCard,
                                                           newRewards[0],
                                                           newRewards[1]);
        const auto player1Observation = make_shared<GoofSpielObservation>(
            natureCard + goofdomain->numberOfCards_ * p1Card +
                goofdomain->numberOfCards_ * goofdomain->numberOfCards_ * p2Card,
            natureCard, p1Card, p2Card);
        const auto player2Observation = make_shared<GoofSpielObservation>(
            natureCard + goofdomain->numberOfCards_ * p1Card +
                goofdomain->numberOfCards_ * goofdomain->numberOfCards_ * p2Card,
            natureCard, p1Card, p2Card);

        vector<shared_ptr<Observation>> newObservations{player1Observation,
                                                        player2Observation};

        const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
        newOutcomes.emplace_back(newOutcome, 1.0 / natureDeck_.size());
      }
    }
  }
  return newOutcomes;
}

vector<Player> GoofSpielState::getPlayers() const {
  if (!player1Deck_.empty() && !player2Deck_.empty()) {
    return {0, 1};
  } else {
    return {};
  }
}

string GoofSpielState::toString() const {
    string ret = "P1: [";
    for (int card : player1PlayedCards_) {
        ret.append(to_string(card) +" ");
    }
    ret.append("]\nP2: [");
    for (auto card : player2PlayedCards_) {
        ret.append(to_string(card) +" ");
    }
    ret.append("]\nN: [");
    for (auto card : naturePlayedCards_) {
        ret.append(to_string(card) +" ");
    }
    ret.append("]");
    return ret;
}

bool GoofSpielState::operator==(const State &rhs) const {
  auto gsState = dynamic_cast<const GoofSpielState &>(rhs);

  return natureSelectedCard_ == gsState.natureSelectedCard_ &&
      player1CumulativeReward_ == gsState.player1CumulativeReward_ &&
      player2CumulativeReward_ == gsState.player2CumulativeReward_ &&
      player1PlayedCards_ == gsState.player1PlayedCards_ &&
      player2PlayedCards_ == gsState.player2PlayedCards_ &&
      naturePlayedCards_ == gsState.naturePlayedCards_ &&
      player1Deck_ == gsState.player1Deck_ &&
      player2Deck_ == gsState.player2Deck_ &&
      natureDeck_ == gsState.natureDeck_;
}

size_t GoofSpielState::getHash() const {
  size_t seed = 0;
  for (auto i : player1PlayedCards_) {
    boost::hash_combine(seed, i);
  }
  for (auto i : player2PlayedCards_) {
    boost::hash_combine(seed, i);
  }
  for (auto i : naturePlayedCards_) {
    boost::hash_combine(seed, i);
  }
  boost::hash_combine(seed, player1CumulativeReward_);
  boost::hash_combine(seed, player2CumulativeReward_);
  return seed;
}

IIGoofSpielDomain::IIGoofSpielDomain(unsigned int maxDepth, optional<unsigned long int> seed) :
    IIGoofSpielDomain(13, maxDepth, move(seed)) {}

IIGoofSpielDomain::IIGoofSpielDomain(int numberOfCards,
                                     unsigned int maxDepth,
                                     optional<unsigned long int> seed) :
    Domain(maxDepth, 2), numberOfCards_(numberOfCards), seed_(seed ? 1 : 0) {
  if (seed) {
    auto range = vector<int>(static_cast<unsigned int>(numberOfCards));
    std::iota(range.begin(), range.end(), 1);
    for (auto &i : range) {
      maxUtility_ += i;
    }
    std::default_random_engine eng{*seed};
    std::mt19937 randEng(eng());
    std::shuffle(range.begin(), range.end(), randEng);
    vector<int> deck(range.begin(), range.end());
    vector<double> rewards(2);

    auto player1Deck = deck;
    auto player2Deck = deck;
    auto natureDeck = deck;
    const int firstCard = *natureDeck.begin();
    natureDeck.erase(natureDeck.begin());
    vector<int> p1;
    vector<int> p2;
    vector<int> n = {firstCard};

    auto newState = make_shared<IIGoofSpielState>(this, player1Deck, player2Deck,
                                                  natureDeck, firstCard, 0.0, 0.0, p1, p2, n);
    vector<shared_ptr<Observation>> newObservations{
        make_shared<IIGoofSpielObservation>(firstCard, firstCard, nullopt, nullopt),
        make_shared<IIGoofSpielObservation>(firstCard, firstCard, nullopt, nullopt)
    };

    Outcome outcome(newState, newObservations, rewards);
    rootStatesDistribution_.emplace_back(outcome, 1.0);
  } else {
    auto range = boost::irange(1, numberOfCards + 1);
    vector<int> deck(range.begin(), range.end());
    for (auto &i : deck) {
      maxUtility_ += i;
    }
    vector<double> rewards(2);
    for (auto const i : range) {
      auto player1Deck = deck;
      auto player2Deck = deck;
      auto natureDeck = deck;
      vector<int> p1;
      vector<int> p2;
      vector<int> n = {i};
      natureDeck.erase(natureDeck.begin() + i - 1);
      auto newState = make_shared<IIGoofSpielState>(this, player1Deck, player2Deck, natureDeck,
                                                    i, 0.0, 0.0, p1, p2, n);
      vector<shared_ptr<Observation>> newObservations{
          make_shared<IIGoofSpielObservation>(i, i, nullopt, nullopt),
          make_shared<IIGoofSpielObservation>(i, i, nullopt, nullopt)
      };
      Outcome outcome(newState, newObservations, rewards);
      rootStatesDistribution_.emplace_back(outcome, 1.0 / deck.size());
    }
  }
}

string IIGoofSpielDomain::getInfo() const {
  return "Goof spiel. Max depth is: " + std::to_string(maxDepth_);
}

vector<Player> IIGoofSpielDomain::getPlayers() const {
  return {0, 1};
}

IIGoofSpielState::IIGoofSpielState(Domain *domain, vector<int> player1Deck,
                                   vector<int> player2Deck, vector<int> natureDeck,
                                   optional<int> natureSelectedCard, double player1CumulativeReward,
                                   double player2CumulativeReward, vector<int> player1PlayedCards,
                                   vector<int> player2PlayedCards, vector<int> naturePlayedCards) :
    GoofSpielState(domain, move(player1Deck), move(player2Deck), move(natureDeck),
                   move(natureSelectedCard), player1CumulativeReward, player2CumulativeReward,
                   move(player1PlayedCards), move(player2PlayedCards), move(naturePlayedCards)) {}

IIGoofSpielState::IIGoofSpielState(Domain *domain, const GoofSpielState &previousState,
                                   int player1Card, int player2Card, optional<int> newNatureCard,
                                   double player1CumulativeReward, double player2CumulativeReward) :
    GoofSpielState(domain, previousState, player1Card, player2Card, move(newNatureCard),
                   player1CumulativeReward, player2CumulativeReward) {}

OutcomeDistribution
IIGoofSpielState::performActions(const vector<PlayerAction> &actions) const {
  const auto goofdomain = static_cast<IIGoofSpielDomain *>(domain_);
  const auto player1Action = dynamic_cast<GoofSpielAction*>(actions[0].second.get());
  const auto player2Action = dynamic_cast<GoofSpielAction*>(actions[1].second.get());

  const int p1Card = player1Action->cardNumber_;
  const int p2Card = player2Action->cardNumber_;

  OutcomeDistribution newOutcomes;

  const double thisRoundRewardP1 = p1Card == p2Card ? 0.0
                                                    : p1Card > p2Card ? *natureSelectedCard_ : -*natureSelectedCard_;
  const double thisRoundRewardP2 = -thisRoundRewardP1;

  vector<double> newRewards{player1CumulativeReward_ + thisRoundRewardP1,
                            player2CumulativeReward_ + thisRoundRewardP2};

  int result = p1Card == p2Card ? 1 : p1Card > p2Card ? 2 : 0;

  if (natureDeck_.empty()) {
    const auto newStatex = make_shared<IIGoofSpielState>(domain_, *this, p1Card, p2Card,
                                                         nullopt, newRewards[0], newRewards[1]);

    vector<shared_ptr<Observation>> newObservations{
        make_shared<IIGoofSpielObservation>(goofdomain->numberOfCards_ * p1Card +
        result * goofdomain->numberOfCards_ * goofdomain->numberOfCards_, nullopt, p1Card, result),
        make_shared<IIGoofSpielObservation>(goofdomain->numberOfCards_ * p2Card +
        result * goofdomain->numberOfCards_ * goofdomain->numberOfCards_, nullopt, p2Card, 2 - result)
    };
    const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
    newOutcomes.emplace_back(newOutcome, 1.0);

  } else {
    if (goofdomain->seed_) {
      auto natureCard = *natureDeck_.begin();
      const auto newStatex = make_shared<IIGoofSpielState>(domain_,
                                                           *this,
                                                           p1Card,
                                                           p2Card,
                                                           natureCard,
                                                           newRewards[0],
                                                           newRewards[1]);

      vector<shared_ptr<Observation>> newObservations{
          make_shared<IIGoofSpielObservation>(natureCard + goofdomain->numberOfCards_ * p1Card
                                                  + result * goofdomain->numberOfCards_
                                                      * goofdomain->numberOfCards_,
                                              natureCard, p1Card, result),
          make_shared<IIGoofSpielObservation>(natureCard + goofdomain->numberOfCards_ * p2Card
                                                  + (2 - result) * goofdomain->numberOfCards_
                                                      * goofdomain->numberOfCards_,
                                              natureCard, p2Card, 2 - result)};

      const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
      newOutcomes.emplace_back(newOutcome, 1.0);
    } else {
      for (const auto natureCard : natureDeck_) {
        const auto newStatex = make_shared<IIGoofSpielState>(domain_,
                                                             *this,
                                                             p1Card,
                                                             p2Card,
                                                             natureCard,
                                                             newRewards[0],
                                                             newRewards[1]);

        const auto player1Observation = make_shared<IIGoofSpielObservation>(natureCard +
            goofdomain->numberOfCards_ * p1Card + result * goofdomain->numberOfCards_ *
            goofdomain->numberOfCards_, natureCard, p1Card, result);
        const auto player2Observation = make_shared<IIGoofSpielObservation>(natureCard +
            goofdomain->numberOfCards_ * p2Card + (2 - result) * goofdomain->numberOfCards_ *
            goofdomain->numberOfCards_, natureCard, p2Card, 2 - result);

        vector<shared_ptr<Observation>> newObservations{player1Observation, player2Observation};

        const auto newOutcome = Outcome(newStatex, newObservations, newRewards);

        newOutcomes.emplace_back(newOutcome, 1.0 / natureDeck_.size());
      }
    }
  }
  return newOutcomes;
}
}  // namespace domains
}  // namespace GTLib2
#pragma clang diagnostic pop
