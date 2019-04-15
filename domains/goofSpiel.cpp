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
#include <algorithm>
#include "domains/goofSpiel.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2::domains {

GoofSpielAction::GoofSpielAction(ActionId id, int card) : Action(id) {
    cardNumber_ = card;
}

string GoofSpielAction::toString() const {
    return "Card: " + std::to_string(cardNumber_);
}

size_t GoofSpielAction::getHash() const {
    return cardNumber_;
}

bool GoofSpielAction::operator==(const Action &that) const {
    if (typeid(that) == typeid(*this)) {
        const auto rhsAction = static_cast<const GoofSpielAction *>(&that);
        return cardNumber_ == rhsAction->cardNumber_;
    }
    return false;
}

GoofSpielObservation::GoofSpielObservation(int initialNumOfCards,
                                           const std::array<int, 3> &chosenCards,
                                           int roundResult)
    : Observation(),
      player0LastCard_(chosenCards[0]),
      player1LastCard_(chosenCards[1]),
      natureCard_(chosenCards[2]),
      roundResult_(roundResult) {

    assert(initialNumOfCards < 20);
    int n = initialNumOfCards + 1;

    id_ = (roundResult_ + 1) // round outcome is 0..2, i.e. 2 bits to shift
        | ((natureCard_ + player0LastCard_ * n + player1LastCard_ * n * n) << 2);
}

void GoofSpielSettings::shuffleChanceCards(int seed) {
    assert(fixChanceCards);
    if (chanceCards.empty()) {
        chanceCards = vector<int>(numCards);
        std::iota(chanceCards.begin(), chanceCards.end(), 1);
    }

    std::default_random_engine eng{seed};
    std::mt19937 randEng(eng());
    std::shuffle(chanceCards.begin(), chanceCards.end(), randEng);
}

vector<int> GoofSpielSettings::getNatureCards() {
    if (chanceCards.empty()) {
        chanceCards = vector<int>(numCards);
        std::iota(chanceCards.begin(), chanceCards.end(), 1);
    }

    assert(chanceCards.size() == numCards);
    return chanceCards;
}

GoofSpielDomain::GoofSpielDomain(GoofSpielSettings settings) :
    Domain(settings.numCards, 2),
    numberOfCards_(settings.numCards),
    fixChanceCards_(settings.fixChanceCards),
    binaryTerminalRewards_(settings.binaryTerminalRewards),
    variant_(settings.variant),
    natureCards_(settings.getNatureCards()) {

    if (binaryTerminalRewards_) {
        maxUtility_ = 1.0;
    } else {
        maxUtility_ = 0.0;
        for (auto natureCard: natureCards_) maxUtility_ += natureCard;
    }

    fixChanceCards_ ? initFixedCards(natureCards_)
                    : initRandomCards(natureCards_);
}


void GoofSpielDomain::initFixedCards(const vector<int> &natureCards) {
    auto deck = vector<int>(numberOfCards_);
    std::iota(deck.begin(), deck.end(), 1);

    std::array<vector<int>, 3> playerDecks = {deck, deck, natureCards};
    int natureFirstCard = playerDecks[2][0];
    playerDecks[2].erase(playerDecks[2].begin());
    vector<double> cumulativeRewards = {0.0, 0.0};
    std::array<vector<int>, 3> playedCards;
    playedCards[0] = {};
    playedCards[1] = {};
    playedCards[2] = {natureFirstCard};

    auto newState = make_shared<GoofSpielState>(this, playerDecks, natureFirstCard,
                                                cumulativeRewards, playedCards);

    auto publicObs = make_shared<GoofSpielObservation>(
        numberOfCards_, std::array<int, 3>{
            NO_CARD_OBSERVATION, NO_CARD_OBSERVATION, natureFirstCard
        }, 0);
    auto player0Obs = make_shared<GoofSpielObservation>(*publicObs);
    auto player1Obs = make_shared<GoofSpielObservation>(*publicObs);

    Outcome outcome(newState, {player0Obs, player1Obs}, publicObs, cumulativeRewards);
    rootStatesDistribution_.emplace_back(outcome, 1.0);
}

void GoofSpielDomain::initRandomCards(const vector<int> &natureCards) {
    auto deck = vector<int>(numberOfCards_);
    std::iota(deck.begin(), deck.end(), 1);

    for (auto const natureFirstCard : natureCards) {
        std::array<vector<int>, 3> playerDecks = {deck, deck, natureCards};
        auto &natureDeck = playerDecks[2];
        natureDeck.erase(std::remove(natureDeck.begin(), natureDeck.end(), natureFirstCard),
                         natureDeck.end());
//        playerDecks[2].erase(playerDecks[2].begin() + i - 1);
        vector<double> cumulativeRewards = {0.0, 0.0};
        std::array<vector<int>, 3> playedCards;
        playedCards[0] = {};
        playedCards[1] = {};
        playedCards[2] = {natureFirstCard};

        auto newState = make_shared<GoofSpielState>(this, playerDecks, natureFirstCard,
                                                    cumulativeRewards, playedCards);

        auto publicObs = make_shared<GoofSpielObservation>(
            numberOfCards_, std::array<int, 3>{
                NO_CARD_OBSERVATION, NO_CARD_OBSERVATION, natureFirstCard
            }, 0);
        auto player0Obs = make_shared<GoofSpielObservation>(*publicObs);
        auto player1Obs = make_shared<GoofSpielObservation>(*publicObs);

        Outcome outcome(newState, {player0Obs, player1Obs}, publicObs, cumulativeRewards);
        rootStatesDistribution_.emplace_back(outcome, 1.0 / deck.size());
    }
}

string GoofSpielDomain::getInfo() const {
    string type = (variant_ ? "IIGoofspiel" : "Goofspiel");
    return type + " with " + std::to_string(numberOfCards_) + " cards and "
        + (fixChanceCards_ ? "fixed nature deck" : "random nature cards");
}

vector<Player> GoofSpielDomain::getPlayers() const {
    return {0, 1};
}

GoofSpielState::GoofSpielState(Domain *domain,
                               std::array<vector<int>, 3> playerDecks,
                               int natureSelectedCard,
                               vector<double> cumulativeRewards,
                               std::array<vector<int>, 3> playedCards) : State(domain) {
    playerDecks_ = std::move(playerDecks);
    natureSelectedCard_ = natureSelectedCard;
    cumulativeRewards_ = std::move(cumulativeRewards);
    playedCards_ = std::move(playedCards);
}

GoofSpielState::GoofSpielState(Domain *domain,
                               const GoofSpielState &previousState,
                               std::array<int, 3> roundPlayedCards,
                               vector<double> cumulativeRewards) : State(domain) {
    playerDecks_ = previousState.playerDecks_;
    playedCards_ = previousState.playedCards_;

    for (int j = 0; j < 3; ++j) {
        playerDecks_[j].erase(
            std::remove(playerDecks_[j].begin(), playerDecks_[j].end(), roundPlayedCards[j]),
            playerDecks_[j].end());

        playedCards_[j].push_back(roundPlayedCards[j]);
        if (j == 2) {
            natureSelectedCard_ = roundPlayedCards[2];
        }
    }
    cumulativeRewards_ = std::move(cumulativeRewards);
}

unsigned long GoofSpielState::countAvailableActionsFor(Player player) const {
    return playerDecks_[player].size();
}

vector<shared_ptr<Action>> GoofSpielState::getAvailableActionsFor(const Player player) const {
    vector<shared_ptr<Action>> actions;
    int id = 0;
    for (const int cardNumber : playerDecks_[player]) {
        actions.push_back(make_shared<GoofSpielAction>(id++, cardNumber));
    }
    return actions;
}

OutcomeDistribution
GoofSpielState::performActions(const vector<PlayerAction> &actions) const {
    const auto goofdomain = static_cast<GoofSpielDomain *>(domain_);
    const std::array<int, 3> chosenCards = {
        dynamic_cast<GoofSpielAction *>(actions[0].second.get())->cardNumber_,
        dynamic_cast<GoofSpielAction *>(actions[1].second.get())->cardNumber_,
        natureSelectedCard_
    };
    const auto &natureDeck = playerDecks_[2];

    int roundResult = chosenCards[0] == chosenCards[1]
                      ? 0 : chosenCards[0] > chosenCards[1] ? 1 : -1;
    double roundReward = roundResult * natureSelectedCard_;

    vector<double> newRewards = {
        cumulativeRewards_[0] + roundReward,
        cumulativeRewards_[1] - roundReward
    };
    if (goofdomain->binaryTerminalRewards_ && natureDeck.empty()) {
        newRewards[0] = newRewards[0] > 1.0 ? 1.0 : (newRewards[0] < -1.0 ? -1.0 : 0.0);
        newRewards[1] = -newRewards[0];
    }

    OutcomeDistribution newOutcomes;

    auto addOutcome = [&](std::array<int, 3> chosenCards, double chanceProb) {
        const auto newState = make_shared<GoofSpielState>(
            goofdomain, *this, chosenCards, newRewards);

        shared_ptr<GoofSpielObservation> publicObs;
        shared_ptr<GoofSpielObservation> obs0;
        shared_ptr<GoofSpielObservation> obs1;

        if (goofdomain->variant_ == IncompleteObservations) {
            publicObs = make_shared<GoofSpielObservation>(
                goofdomain->numberOfCards_, std::array<int, 3>{
                    NO_CARD_OBSERVATION, NO_CARD_OBSERVATION, chosenCards[2]
                }, roundResult);
            obs0 = make_shared<GoofSpielObservation>(
                goofdomain->numberOfCards_, std::array<int, 3>{
                    chosenCards[0], NO_CARD_OBSERVATION, chosenCards[2]
                }, roundResult);
            obs1 = make_shared<GoofSpielObservation>(
                goofdomain->numberOfCards_, std::array<int, 3>{
                    NO_CARD_OBSERVATION, chosenCards[1], chosenCards[2]
                }, roundResult);
        } else {
            publicObs = make_shared<GoofSpielObservation>(
                goofdomain->numberOfCards_, chosenCards, roundResult);
            obs0 = make_shared<GoofSpielObservation>(*publicObs);
            obs1 = make_shared<GoofSpielObservation>(*publicObs);
        }

        const auto newOutcome = Outcome(newState, {obs0, obs1}, publicObs, newRewards);
        newOutcomes.emplace_back(newOutcome, chanceProb);
    };

    if (goofdomain->fixChanceCards_) {
        auto natureCard = natureDeck.empty() ? NO_NATURE_CARD : natureDeck[0];
        addOutcome({chosenCards[0], chosenCards[1], natureCard}, 1.0);
    } else {
        if (natureDeck.empty()) {
            addOutcome({chosenCards[0], chosenCards[1], NO_NATURE_CARD}, 1.0);
        } else {
            for (const auto natureCard : natureDeck) {
                addOutcome({chosenCards[0], chosenCards[1], natureCard}, 1.0 / natureDeck.size());
            }
        }
    }

    return newOutcomes;
}

vector<Player> GoofSpielState::getPlayers() const {
    if (!playerDecks_[0].empty() && !playerDecks_[1].empty()) {
        return {0, 1};
    } else {
        return {};
    }
}

string GoofSpielState::toString() const {
    string ret;
    for (int pl = 0; pl < 3; ++pl) {
        ret.append(((pl < 2) ? "P" + to_string(pl) : "N")
                       + ": [");
        for (int card : playedCards_[pl]) {
            ret.append(to_string(card) + " ");
        }
        ret.append("]");
    }
    return ret;
}

bool GoofSpielState::operator==(const State &rhs) const {
    auto gsState = dynamic_cast<const GoofSpielState &>(rhs);

    return natureSelectedCard_ == gsState.natureSelectedCard_
        && playedCards_ == gsState.playedCards_
        && playerDecks_ == gsState.playerDecks_;
}

size_t GoofSpielState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, playerDecks_);
    boost::hash_combine(seed, playedCards_);
    boost::hash_combine(seed, natureSelectedCard_);
    return seed;
}

}  // namespace GTLib2
#pragma clang diagnostic pop
