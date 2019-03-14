//
// Created by Pavel Rytir on 29/01/2018.
//

#include <random>
#include "domains/goofSpiel.h"
#include "boost/range/irange.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
namespace GTLib2 {
namespace domains {
GoofSpielAction::GoofSpielAction(int id, int card) : Action(id) {
  cardNumber = card;
}

string GoofSpielAction::toString() const {
  return "Card number: " + std::to_string(cardNumber);
}

size_t GoofSpielAction::getHash() const {
  std::hash<int> h;
  return h(cardNumber);
}

bool GoofSpielAction::operator==(const Action &that) const {
  if (typeid(that) == typeid(*this)) {
    const auto rhsAction = static_cast<const GoofSpielAction *>(&that);
    return this->cardNumber == rhsAction->cardNumber;
  }
  return false;
}

GoofSpielObservation::GoofSpielObservation(int id,
                                           optional<int> newBid,
                                           optional<int> player1LastCard,
                                           optional<int> player2LastCard) :
    Observation(id) {
  this->newBid = std::move(newBid);
  this->player1LastCard = std::move(player1LastCard);
  this->player2LastCard = std::move(player2LastCard);
}

IIGoofSpielObservation::IIGoofSpielObservation(int id, optional<int> newBid,
                                               optional<int> myLastCard, optional<int> result) :
    Observation(id), newBid(move(newBid)), myLastCard(move(myLastCard)),
    result(move(result)) {}

GoofSpielDomain::GoofSpielDomain(unsigned int maxDepth, optional<unsigned long int> seed) :
    GoofSpielDomain(
        13,
        maxDepth,
        move(seed)) {}

GoofSpielDomain::GoofSpielDomain(int numberOfCards, unsigned int maxDepth,
                                 optional<unsigned long int> seed) :
    Domain(maxDepth, 2), numberOfCards(numberOfCards), seed(seed ? 1 : 0) {
  if (seed) {
    auto range = vector<int>(static_cast<unsigned int>(numberOfCards));
    std::iota(range.begin(), range.end(), 1);
    for (auto &i : range) {
      maxUtility += i;
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
    rootStatesDistribution.emplace_back(outcome, 1.0);
  } else {
    auto range = boost::irange(1, numberOfCards + 1);
    vector<int> deck(range.begin(), range.end());
    for (auto &i : deck) {
      this->maxUtility += i;
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
      rootStatesDistribution.emplace_back(outcome, 1.0 / deck.size());
    }
  }
}

string GoofSpielDomain::getInfo() const {
  return "Goof spiel. Max depth is: " + std::to_string(maxDepth);
}

vector<int> GoofSpielDomain::getPlayers() const {
  return {0, 1};
}

GoofSpielState::GoofSpielState(Domain *domain,
                               vector<int> player1Deck, vector<int> player2Deck,
                               vector<int> natureDeck, optional<int> natureSelectedCard,
                               double player1CumulativeReward, double player2CumulativeReward,
                               vector<int> player1PlayedCards, vector<int> player2PlayedCards,
                               vector<int> naturePlayedCards) : State(domain) {
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

GoofSpielState::GoofSpielState(Domain *domain,
                               const GoofSpielState &previousState,
                               int player1Card,
                               int player2Card,
                               optional<int> newNatureCard,
                               double player1CumulativeReward,
                               double player2CumulativeReward) : State(domain) {
  player1Deck = previousState.player1Deck;
  player1Deck.erase(std::remove(player1Deck.begin(), player1Deck.end(),
      player1Card), player1Deck.end());
  player2Deck = previousState.player2Deck;
  player2Deck.erase(std::remove(player2Deck.begin(), player2Deck.end(),
      player2Card), player2Deck.end());

  player1PlayedCards = previousState.player1PlayedCards;
  player1PlayedCards.push_back(player1Card);

  player2PlayedCards = previousState.player2PlayedCards;
  player2PlayedCards.push_back(player2Card);

  this->player1CumulativeReward = player1CumulativeReward;
  this->player2CumulativeReward = player2CumulativeReward;

  if (newNatureCard == nullopt) {
    natureDeck = vector<int>();
    naturePlayedCards = previousState.naturePlayedCards;
    natureSelectedCard = nullopt;
  } else {
    natureDeck = previousState.natureDeck;
    natureDeck.erase(std::remove(natureDeck.begin(), natureDeck.end(),
        *newNatureCard), natureDeck.end());

    naturePlayedCards = previousState.naturePlayedCards;
    naturePlayedCards.push_back(*newNatureCard);
    natureSelectedCard = newNatureCard;
  }
}

vector<shared_ptr<Action>> GoofSpielState::getAvailableActionsFor(const int player) const {
  const auto &deck = player == 0 ? player1Deck : player2Deck;

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
GoofSpielState::performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const {
  const auto goofdomain = static_cast<GoofSpielDomain *>(domain);
  const auto player1Action = dynamic_cast<GoofSpielAction*>(actions[0].second.get());
  const auto player2Action = dynamic_cast<GoofSpielAction*>(actions[1].second.get());
  const int p1Card = player1Action->cardNumber;
  const int p2Card = player2Action->cardNumber;

  OutcomeDistribution newOutcomes;

  const double thisRoundRewardP1 = p1Card == p2Card ? 0.0
      : p1Card > p2Card ? *natureSelectedCard : -*natureSelectedCard;
  const double thisRoundRewardP2 = -thisRoundRewardP1;

  vector<double> newRewards
      {player1CumulativeReward + thisRoundRewardP1, player2CumulativeReward + thisRoundRewardP2};

  if (natureDeck.empty()) {
    const auto newStatex = make_shared<GoofSpielState>(domain, *this, p1Card, p2Card,
                                                       nullopt, newRewards[0], newRewards[1]);

    const auto player1Observation = make_shared<GoofSpielObservation>(
        goofdomain->numberOfCards * p1Card +
            goofdomain->numberOfCards * goofdomain->numberOfCards * p2Card,
        nullopt, p1Card, p2Card);
    const auto player2Observation = make_shared<GoofSpielObservation>(
        goofdomain->numberOfCards * p1Card +
            goofdomain->numberOfCards * goofdomain->numberOfCards * p2Card,
        nullopt, p1Card, p2Card);

    vector<shared_ptr<Observation>> newObservations{player1Observation, player2Observation};
    const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
    newOutcomes.emplace_back(newOutcome, 1.0);

  } else {
    if (goofdomain->seed) {
      auto natureCard = *natureDeck.begin();
      const auto newStatex = make_shared<GoofSpielState>(domain, *this, p1Card, p2Card,
                                                         natureCard, newRewards[0], newRewards[1]);

      vector<shared_ptr<Observation>> newObservations{
          make_shared<GoofSpielObservation>(natureCard +
              goofdomain->numberOfCards * p1Card + goofdomain->numberOfCards *
              goofdomain->numberOfCards * p2Card, natureCard, p1Card, p2Card),
          make_shared<GoofSpielObservation>(natureCard +
              goofdomain->numberOfCards * p1Card + goofdomain->numberOfCards *
              goofdomain->numberOfCards * p2Card, natureCard, p1Card, p2Card)};

      const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
      newOutcomes.emplace_back(newOutcome, 1.0);
    } else {
      for (const auto natureCard : natureDeck) {
        const auto newStatex = make_shared<GoofSpielState>(domain,
                                                           *this,
                                                           p1Card,
                                                           p2Card,
                                                           natureCard,
                                                           newRewards[0],
                                                           newRewards[1]);
        const auto player1Observation = make_shared<GoofSpielObservation>(
            natureCard + goofdomain->numberOfCards * p1Card +
                goofdomain->numberOfCards * goofdomain->numberOfCards * p2Card,
            natureCard, p1Card, p2Card);
        const auto player2Observation = make_shared<GoofSpielObservation>(
            natureCard + goofdomain->numberOfCards * p1Card +
                goofdomain->numberOfCards * goofdomain->numberOfCards * p2Card,
            natureCard, p1Card, p2Card);

        vector<shared_ptr<Observation>> newObservations{player1Observation,
                                                        player2Observation};

        const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
        newOutcomes.emplace_back(newOutcome, 1.0 / natureDeck.size());
      }
    }
  }
  return newOutcomes;
}

vector<int> GoofSpielState::getPlayers() const {
  if (!player1Deck.empty() && !player2Deck.empty()) {
    return {0, 1};
  } else {
    return {};
  }
}

string GoofSpielState::toString() const {
  string ret = "p1played: ";
  for (int card : player1PlayedCards) {
    ret.append(" " + to_string(card) + " ");
  }
  ret.append(" p2played: ");
  for (auto card : player2PlayedCards) {
    ret.append(" " + to_string(card) + " ");
  }
  ret.append(" naturePlayed ");
  for (auto card : naturePlayedCards) {
    ret.append(" " + to_string(card) + " ");
  }
  return ret;
}

bool GoofSpielState::operator==(const State &rhs) const {
  auto gsState = dynamic_cast<const GoofSpielState &>(rhs);

  return natureSelectedCard == gsState.natureSelectedCard &&
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

IIGoofSpielDomain::IIGoofSpielDomain(unsigned int maxDepth, optional<unsigned long int> seed) :
    IIGoofSpielDomain(13, maxDepth, move(seed)) {}

IIGoofSpielDomain::IIGoofSpielDomain(int numberOfCards,
                                     unsigned int maxDepth,
                                     optional<unsigned long int> seed) :
    Domain(maxDepth, 2), numberOfCards(numberOfCards), seed(seed ? 1 : 0) {
  if (seed) {
    auto range = vector<int>(static_cast<unsigned int>(numberOfCards));
    std::iota(range.begin(), range.end(), 1);
    for (auto &i : range) {
      maxUtility += i;
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
    rootStatesDistribution.emplace_back(outcome, 1.0);
  } else {
    auto range = boost::irange(1, numberOfCards + 1);
    vector<int> deck(range.begin(), range.end());
    for (auto &i : deck) {
      maxUtility += i;
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
      rootStatesDistribution.emplace_back(outcome, 1.0 / deck.size());
    }
  }
}

string IIGoofSpielDomain::getInfo() const {
  return "Goof spiel. Max depth is: " + std::to_string(maxDepth);
}

vector<int> IIGoofSpielDomain::getPlayers() const {
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
IIGoofSpielState::performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const {
  const auto goofdomain = static_cast<IIGoofSpielDomain *>(domain);
  const auto player1Action = dynamic_cast<GoofSpielAction*>(actions[0].second.get());
  const auto player2Action = dynamic_cast<GoofSpielAction*>(actions[1].second.get());

  const int p1Card = player1Action->cardNumber;
  const int p2Card = player2Action->cardNumber;

  OutcomeDistribution newOutcomes;

  const double thisRoundRewardP1 = p1Card == p2Card ?
                                   *natureSelectedCard / 2.0 : p1Card > p2Card ? *natureSelectedCard
                                                                               : 0.0;

  const double thisRoundRewardP2 = p1Card == p2Card ?
                                   *natureSelectedCard / 2.0 : p2Card > p1Card ? *natureSelectedCard
                                                                               : 0.0;

  vector<double> newRewards{player1CumulativeReward + thisRoundRewardP1,
                            player2CumulativeReward + thisRoundRewardP2};

  int result = p1Card == p2Card ? 1 : p1Card > p2Card ? 2 : 0;

  if (natureDeck.empty()) {
    const auto newStatex = make_shared<IIGoofSpielState>(domain, *this, p1Card, p2Card,
                                                         nullopt, newRewards[0], newRewards[1]);

    vector<shared_ptr<Observation>> newObservations{
        make_shared<IIGoofSpielObservation>(goofdomain->numberOfCards * p1Card +
        result * goofdomain->numberOfCards * goofdomain->numberOfCards, nullopt, p1Card, result),
        make_shared<IIGoofSpielObservation>(goofdomain->numberOfCards * p2Card +
        result * goofdomain->numberOfCards * goofdomain->numberOfCards, nullopt, p2Card, 2 - result)
    };
    const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
    newOutcomes.emplace_back(newOutcome, 1.0);

  } else {
    if (goofdomain->seed) {
      auto natureCard = *natureDeck.begin();
      const auto newStatex = make_shared<IIGoofSpielState>(domain,
                                                           *this,
                                                           p1Card,
                                                           p2Card,
                                                           natureCard,
                                                           newRewards[0],
                                                           newRewards[1]);

      vector<shared_ptr<Observation>> newObservations{
          make_shared<IIGoofSpielObservation>(natureCard + goofdomain->numberOfCards * p1Card
                                                  + result * goofdomain->numberOfCards
                                                      * goofdomain->numberOfCards,
                                              natureCard, p1Card, result),
          make_shared<IIGoofSpielObservation>(natureCard + goofdomain->numberOfCards * p2Card
                                                  + (2 - result) * goofdomain->numberOfCards
                                                      * goofdomain->numberOfCards,
                                              natureCard, p2Card, 2 - result)};

      const auto newOutcome = Outcome(newStatex, newObservations, newRewards);
      newOutcomes.emplace_back(newOutcome, 1.0);
    } else {
      for (const auto natureCard : natureDeck) {
        const auto newStatex = make_shared<IIGoofSpielState>(domain,
                                                             *this,
                                                             p1Card,
                                                             p2Card,
                                                             natureCard,
                                                             newRewards[0],
                                                             newRewards[1]);

        const auto player1Observation = make_shared<IIGoofSpielObservation>(natureCard +
            goofdomain->numberOfCards * p1Card + result * goofdomain->numberOfCards *
            goofdomain->numberOfCards, natureCard, p1Card, result);
        const auto player2Observation = make_shared<IIGoofSpielObservation>(natureCard +
            goofdomain->numberOfCards * p2Card + (2 - result) * goofdomain->numberOfCards *
            goofdomain->numberOfCards, natureCard, p2Card, 2 - result);

        vector<shared_ptr<Observation>> newObservations{player1Observation, player2Observation};

        const auto newOutcome = Outcome(newStatex, newObservations, newRewards);

        newOutcomes.emplace_back(newOutcome, 1.0 / natureDeck.size());
      }
    }
  }
  return newOutcomes;
}
}  // namespace domains
}  // namespace GTLib2
#pragma clang diagnostic pop
