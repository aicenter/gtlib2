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


#include <base/random.h>
#include "domains/goofSpiel.h"


namespace GTLib2::domains {

bool GoofSpielAction::operator==(const Action &that) const {
    if (typeid(that) == typeid(*this)) {
        const auto rhsAction = static_cast<const GoofSpielAction *>(&that);
        return cardNumber_ == rhsAction->cardNumber_;
    }
    return false;
}

GoofSpielObservation::GoofSpielObservation(int initialNumOfCards,
                                           const array<int, 3> &chosenCards,
                                           GoofspielRoundOutcome roundResult)
    : Observation(),
      natureCard_(chosenCards[2]),
      player0LastCard_(chosenCards[0]),
      player1LastCard_(chosenCards[1]),
      roundResult_(roundResult) {

    assert(initialNumOfCards < 20);
    int n = initialNumOfCards + 1;

    id_ = (roundResult_ + 1) // round outcome is 0..2, i.e. 2 bits to shift
        | ((natureCard_ + player0LastCard_ * n + player1LastCard_ * n * n) << 2);
}
string GoofSpielObservation::toString() const {
    return to_string(roundResult_);
}

void GoofSpielSettings::shuffleChanceCards(unsigned long seed) {
    assert(fixChanceCards);
    if (chanceCards.empty()) {
        chanceCards = vector<int>(numCards);
        std::iota(chanceCards.begin(), chanceCards.end(), 1);
    }

//    std::default_random_engine eng{static_cast<std::default_random_engine::result_type>(seed)};
//    std::mt19937 randEng(eng());
    std::mt19937 randEng(seed);
    shuffleVector(chanceCards,
                  randEng);//std::shuffle(chanceCards.begin(), chanceCards.end(), randEng);
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
    Domain(settings.numCards + 1, 2, true,
           make_shared<GoofSpielAction>(),
           make_shared<GoofSpielObservation>()),
    numberOfCards_(settings.numCards),
    fixChanceCards_(settings.fixChanceCards),
    binaryTerminalRewards_(settings.binaryTerminalRewards),
    variant_(settings.variant),
    natureCards_(settings.getNatureCards()) {

    assert(numberOfCards_ >= 1);

    if (numberOfCards_ == 1) maxUtility_ = 0.;
    else if (binaryTerminalRewards_) {
        maxUtility_ = 1.0;
    } else {
        // can't win the smallest card
        maxUtility_ = -2 * (*std::min_element(natureCards_.begin(), natureCards_.end()));
        for (auto natureCard: natureCards_) maxUtility_ += natureCard;
    }

    fixChanceCards_ ? initFixedCards(natureCards_)
                    : initRandomCards(natureCards_);
}


void GoofSpielDomain::initFixedCards(const vector<int> &natureCards) {
    auto deck = vector<int>(numberOfCards_);
    std::iota(deck.begin(), deck.end(), 1);

    array<vector<int>, 3> playerDecks = {deck, deck, natureCards};
    int natureFirstCard = playerDecks[2][0];
    playerDecks[2].erase(playerDecks[2].begin());
    array<vector<int>, 3> playedCards;
    playedCards[0] = {};
    playedCards[1] = {};
    playedCards[2] = {natureFirstCard};

    auto newState = make_shared<GoofSpielState>(this, playerDecks, natureFirstCard, playedCards, 0);

    auto publicObs = make_shared<GoofSpielObservation>(
        numberOfCards_, array<int, 3>{
            NO_CARD_OBSERVATION, NO_CARD_OBSERVATION, natureFirstCard
        }, PL0_DRAW);
    auto player0Obs = make_shared<GoofSpielObservation>(*publicObs);
    auto player1Obs = make_shared<GoofSpielObservation>(*publicObs);

    Outcome outcome(newState, {player0Obs, player1Obs}, publicObs, {0.0, 0.0});
    rootStatesDistribution_.emplace_back(OutcomeEntry(outcome));
}

void GoofSpielDomain::initRandomCards(const vector<int> &natureCards) {
    auto deck = vector<int>(numberOfCards_);
    std::iota(deck.begin(), deck.end(), 1);

    for (auto const natureFirstCard : natureCards) {
        array<vector<int>, 3> playerDecks = {deck, deck, natureCards};
        auto &natureDeck = playerDecks[2];
        natureDeck.erase(std::remove(natureDeck.begin(), natureDeck.end(), natureFirstCard),
                         natureDeck.end());
        array<vector<int>, 3> playedCards;
        playedCards[0] = {};
        playedCards[1] = {};
        playedCards[2] = {natureFirstCard};

        auto newState = make_shared<GoofSpielState>(
            this, playerDecks, natureFirstCard, playedCards, 0);

        auto publicObs = make_shared<GoofSpielObservation>(
            numberOfCards_, array<int, 3>{
                NO_CARD_OBSERVATION, NO_CARD_OBSERVATION, natureFirstCard
            }, PL0_DRAW);
        auto player0Obs = make_shared<GoofSpielObservation>(*publicObs);
        auto player1Obs = make_shared<GoofSpielObservation>(*publicObs);

        Outcome outcome(newState, {player0Obs, player1Obs}, publicObs, {0.0, 0.0});
        rootStatesDistribution_.emplace_back(OutcomeEntry(outcome, 1.0 / deck.size()));
    }
}

string GoofSpielDomain::getInfo() const {
    std::stringstream ss;
    ss << (variant_ ? "IIGoofspiel" : "Goofspiel");
    ss << " with " + to_string(numberOfCards_) + " cards and ";
    if (fixChanceCards_) {
        ss << "fixed nature deck " << natureCards_;
    } else {
        ss << "random nature cards";
    }
    if (binaryTerminalRewards_) ss << " and binary terminal rewards";
    return ss.str();
}

unique_ptr<GoofSpielDomain> GoofSpielDomain::IIGS(unsigned int n) {
    auto chanceCards = vector<int>(n);
    std::generate(chanceCards.begin(), chanceCards.end(), [&n] { return n--; });
    return make_unique<GoofSpielDomain>(GoofSpielSettings{
        /*.variant=*/IncompleteObservations,
        /*.numCards=*/static_cast<uint32>(chanceCards.size()),
        /*.fixChanceCards=*/true,
        /*.chanceCards=*/chanceCards
    });
}

unique_ptr<GoofSpielDomain> GoofSpielDomain::GS(unsigned int n) {
    auto chanceCards = vector<int>(n);
    std::generate(chanceCards.begin(), chanceCards.end(), [&n] { return n--; });
    return make_unique<GoofSpielDomain>(GoofSpielSettings{
        /*.variant=*/CompleteObservations,
        /*.numCards=*/static_cast<uint32>(chanceCards.size()),
        /*.fixChanceCards=*/true,
        /*.chanceCards=*/chanceCards
    });
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
GoofSpielState::performActions(const vector<shared_ptr<Action>> &actions) const {
    const auto goofdomain = static_cast<const GoofSpielDomain *>(domain_);
    const array<int, 3> chosenCards = {
        dynamic_cast<GoofSpielAction &>(*actions[0]).cardNumber_,
        dynamic_cast<GoofSpielAction &>(*actions[1]).cardNumber_,
        natureSelectedCard_
    };
    const auto &natureDeck = playerDecks_[2];

    bool isLastRound = natureDeck.empty();
    const GoofspielRoundOutcome roundResult = chosenCards[0] == chosenCards[1]
                                              ? PL0_DRAW : chosenCards[0] > chosenCards[1] ? PL0_WIN
                                                                                           : PL0_LOSE;
    const double roundReward = goofdomain->binaryTerminalRewards_
                               ? 0 : (double) roundResult * natureSelectedCard_;
    const double cumulativeRewards = cumulativeRewards_ + roundResult * natureSelectedCard_;
    vector<double> newRewards;
    if (isLastRound && goofdomain->binaryTerminalRewards_) {
        const double finalReward = cumulativeRewards == 0 ? 0 : cumulativeRewards > 0 ? 1 : -1;
        newRewards = {finalReward, -finalReward};
    } else newRewards = {roundReward, -roundReward};

    OutcomeDistribution newOutcomes;

    auto addOutcome = [&](array<int, 3> pickedCards, double chanceProb) {
        auto nextPlayerDecks = playerDecks_;
        auto nextPlayedCards = playedCards_;
        const int nextNatureSelectedCard = pickedCards[2];

        for (int j = 0; j < 3; ++j) {
            nextPlayerDecks[j].erase(
                std::remove(nextPlayerDecks[j].begin(), nextPlayerDecks[j].end(), pickedCards[j]),
                nextPlayerDecks[j].end());

            nextPlayedCards[j].push_back(pickedCards[j]);
        }
        const auto newState = make_shared<GoofSpielState>(
            goofdomain, nextPlayerDecks, nextNatureSelectedCard, nextPlayedCards,
            cumulativeRewards);

        shared_ptr<GoofSpielObservation> publicObs;
        shared_ptr<GoofSpielObservation> obs0;
        shared_ptr<GoofSpielObservation> obs1;

        if (goofdomain->variant_ == IncompleteObservations) {
            publicObs = make_shared<GoofSpielObservation>(
                goofdomain->numberOfCards_, array<int, 3>{
                    NO_CARD_OBSERVATION, NO_CARD_OBSERVATION, pickedCards[2]
                }, roundResult);
            obs0 = make_shared<GoofSpielObservation>(
                goofdomain->numberOfCards_, array<int, 3>{
                    pickedCards[0], NO_CARD_OBSERVATION, pickedCards[2]
                }, roundResult);
            obs1 = make_shared<GoofSpielObservation>(
                goofdomain->numberOfCards_, array<int, 3>{
                    NO_CARD_OBSERVATION, pickedCards[1], pickedCards[2]
                }, roundResult);
        } else {
            publicObs = make_shared<GoofSpielObservation>(
                goofdomain->numberOfCards_, pickedCards, roundResult);
            obs0 = make_shared<GoofSpielObservation>(*publicObs);
            obs1 = make_shared<GoofSpielObservation>(*publicObs);
        }

        const auto newOutcome = Outcome(newState, {obs0, obs1}, publicObs, newRewards);
        newOutcomes.emplace_back(OutcomeEntry(newOutcome, chanceProb));
    };

    if (goofdomain->fixChanceCards_) {
        const auto natureCard = isLastRound ? NO_NATURE_CARD : natureDeck[0];
        addOutcome({chosenCards[0], chosenCards[1], natureCard}, 1.0);
    } else {
        if (isLastRound) {
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
    std::stringstream ss;
    for (int pl = 0; pl < 3; ++pl) {
        ss << ((pl < 2) ? "P" + to_string(pl) + ": " : "N:  ");
        ss << playedCards_[pl];

        ss << endl;
    }
    return ss.str();
}

bool GoofSpielState::operator==(const State &rhs) const {
    auto gsState = dynamic_cast<const GoofSpielState &>(rhs);

    return hash_ == gsState.hash_
        && natureSelectedCard_ == gsState.natureSelectedCard_
        && playedCards_ == gsState.playedCards_
        && playerDecks_ == gsState.playerDecks_;
}

bool GoofSpielState::isTerminal() const {
    return playerDecks_[0].empty() && playerDecks_[1].empty();
}

}  // namespace GTLib2
