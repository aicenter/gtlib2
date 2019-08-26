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

#include "domains/genericPoker.h"


namespace GTLib2::domains {

bool GenericPokerAction::operator==(const Action &that) const {
    if (typeid(*this) != typeid(that)) {
        return false;
    }

    const auto rhsAction = static_cast<const GenericPokerAction *>(&that);
    return hash_ == rhsAction->hash_
        && type_ == rhsAction->type_
        && value_ == rhsAction->value_;
}
string GenericPokerAction::toString() const {
    if (id_ == NO_ACTION) return "NoA";
    switch (type_) {
        case Check:
            return "Check";
        case Bet:
            return "Bet: " + to_string(value_);
        case Call:
            return "Call";
        case Raise:
            return "Raise: " + to_string(value_);
        case Fold:
            return "Fold";
        default:
            unreachable("unrecognized option!");
    }
}

string GenericPokerObservation::toString() const {
    if (id_ == NO_OBSERVATION) return "NoOb";
    switch (type_) {
        case PlayCard:
            return "Play: " + to_string(value_);
        case Check:
            return "Check";
        case Bet:
            return "Bet:" + to_string(value_);
        case Call:
            return "Call";
        case Raise:
            return "Raise:" + to_string(value_);
        case Fold:
            return "Fold";
        default:
            unreachable("unrecognized option!");
    }
}

GenericPokerDomain::GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfEachType,
                                       unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                                       unsigned int maxDifferentRaises, unsigned int ante) :
    Domain(7 + 2 * maxRaisesInRow, 2, true,
           make_shared<GenericPokerAction>(),
           make_shared<GenericPokerObservation>()),

    maxCardTypes_(maxCardTypes),
    maxCardsOfEachType_(maxCardsOfEachType),
    maxRaisesInRow_(maxRaisesInRow),
    maxDifferentBets_(maxDifferentBets),
    maxDifferentRaises_(maxDifferentRaises),
    ante_(ante) {

    assert(maxCardTypes_ >= 1);
    assert(maxCardsOfEachType_ >= 1);
    // Players play one card and nature one card = 2*1 + 1
    assert(maxCardTypes_ * maxCardsOfEachType_ >= 3);
    assert(maxRaisesInRow_ >= 1);
    assert(maxDifferentBets_ >= 1);
    assert(maxDifferentRaises_ >= 1);

    for (int i = 1; i <= maxDifferentBets; i++) betsFirstRound_.push_back(i * 2);
    for (int i = 1; i <= maxDifferentRaises; i++) raisesFirstRound_.push_back(i * 2);
    for (int i : betsFirstRound_) betsSecondRound_.push_back(i * 2);
    for (int i : raisesFirstRound_) raisesSecondRound_.push_back(i * 2);

    maxUtility_ = ante_
        + betsFirstRound_.back()
        + maxRaisesInRow_ * raisesFirstRound_.back()
        + (maxCardTypes_ > 1 ? betsSecondRound_.back() : 0) // todo: not entirely sure this is correct
        + maxRaisesInRow_ * raisesSecondRound_.back();

    for (int p1card = 0; p1card < maxCardTypes_; ++p1card) {
        for (int p2card = 0; p2card < maxCardTypes_; ++p2card) {
            // impossible situation that both players get the same card,
            // but there is only one such card the pile
            if (p1card == p2card && maxCardsOfEachType_ == 1) continue;

            int occurencyCount = p1card == p2card ? 1 : 0;

            const double prob = double(maxCardsOfEachType_ - occurencyCount) /
                (maxCardTypes_ * (maxCardsOfEachType_ * maxCardTypes_ - 1));
            const auto newState = make_shared<GenericPokerState>(this, p1card, p2card, nullopt,
                                                                 ante, Player(0));
            const vector<shared_ptr<Observation>> privateObs{
                make_shared<GenericPokerObservation>(3 + p1card, PlayCard, p1card),
                make_shared<GenericPokerObservation>(3 + p2card, PlayCard, p2card)
            };
            const Outcome outcome(newState, privateObs, noObservation_, vector<double>(2));
            rootStatesDistribution_.emplace_back(OutcomeEntry(outcome, prob));
        }
    }
}

GenericPokerDomain::GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes) :
    GenericPokerDomain(maxCardTypes, maxCardsOfTypes, 1, 2, 2, 1) {}

GenericPokerDomain::GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                                       unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                                       unsigned int maxDifferentRaises) :
    GenericPokerDomain(maxCardTypes, maxCardsOfTypes, maxRaisesInRow,
                       maxDifferentBets, maxDifferentRaises, 1) {}

GenericPokerDomain::GenericPokerDomain() : GenericPokerDomain(4, 3, 1, 2, 2, 1) {}

string GenericPokerDomain::getInfo() const {
    std::stringstream ss;
    ss << "Generic Poker:"
       << "\nMax card types: " << maxCardTypes_
       << "\nMax cards of each type: " << maxCardsOfEachType_
       << "\nMax utility: " << maxUtility_
       << "\nAnte: " << ante_
       << "\nMax raises in row: " << maxRaisesInRow_
       << "\nBets first round: " << betsFirstRound_
       << "\nBets second round: " << betsSecondRound_
       << "\nRaises first round: " << raisesFirstRound_
       << "\nRaises second round: " << raisesSecondRound_;
    return ss.str();
}

unsigned long GenericPokerState::countAvailableActionsFor(Player player) const {
    if (round_ == POKER_TERMINAL_ROUND) return 0;

    const auto pokerDomain = static_cast<const GenericPokerDomain *>(domain_);
    if (!lastAction_ || lastAction_->getType() == Check || lastAction_->getType() == Call) {
        long cnt = 1;
        if (round_ == 1) cnt += pokerDomain->betsFirstRound_.size();
        if (round_ == 3) cnt += pokerDomain->betsSecondRound_.size();
        return cnt;
    }

    if (lastAction_->getType() == Bet || lastAction_->getType() == Raise) {
        long cnt = 2;
        if (continuousRaiseCount_ < pokerDomain->maxRaisesInRow_) {
            if (round_ == 1) cnt += pokerDomain->raisesFirstRound_.size();
            if (round_ == 3) cnt += pokerDomain->raisesSecondRound_.size();
        }
        return cnt;
    }

    return 0;
}

vector<shared_ptr<Action>> GenericPokerState::getAvailableActionsFor(Player player) const {
    auto list = vector<shared_ptr<Action>>();
    if (round_ == POKER_TERMINAL_ROUND) return list;

    int count = 0;
    const auto pokerDomain = static_cast<const GenericPokerDomain *>(domain_);

    if (!lastAction_ || lastAction_->getType() == Check || lastAction_->getType() == Call) {
        if (round_ == 1) {
            for (int betValue : pokerDomain->betsFirstRound_) {
                list.push_back(make_shared<GenericPokerAction>(count, Bet, betValue));
                ++count;
            }
        } else if (round_ == 3) {
            for (int betValue : pokerDomain->betsSecondRound_) {
                list.push_back(make_shared<GenericPokerAction>(count, Bet, betValue));
                ++count;
            }
        }
        list.push_back(make_shared<GenericPokerAction>(count, Check, 0));
        return list;
    }

    if (lastAction_->getType() == Bet || lastAction_->getType() == Raise) {
        list.push_back(make_shared<GenericPokerAction>(count, Call, 0));
        ++count;
        if (continuousRaiseCount_ < pokerDomain->maxRaisesInRow_) {
            if (round_ == 1) {
                for (int raiseValue : pokerDomain->raisesFirstRound_) {
                    list.push_back(make_shared<GenericPokerAction>(count, Raise, raiseValue));
                    ++count;
                }
            } else if (round_ == 3) {
                for (int raiseValue : pokerDomain->raisesSecondRound_) {
                    list.push_back(make_shared<GenericPokerAction>(count, Raise, raiseValue));
                    ++count;
                }
            }
        }
        list.push_back(make_shared<GenericPokerAction>(count, Fold, 0));
    }

    return list;
}

OutcomeDistribution
GenericPokerState::performActions(const vector<shared_ptr<Action>> &actions) const {
    const auto pokerDomain = static_cast<const GenericPokerDomain *>(domain_);
    assert(getPlayers().size() == 1);
    const Player currentPlayer = getPlayers()[0];
    const auto pokerAction = dynamic_pointer_cast<GenericPokerAction>(actions[currentPlayer]);
    assert(pokerAction->getId() != NO_ACTION);
    assert(actions[opponent(currentPlayer)]->getId() == NO_ACTION);

    double currentBet = 0.;
    double newFirstPlayerReward = cumulativeFirstPlayerReward_;
    int newContinuousRaiseCount = continuousRaiseCount_;
    int newRound = round_;
    PokerObservationId obsId = 0;

    switch (pokerAction->getType()) {
        case Raise:
            obsId = int(3 + pokerDomain->maxCardTypes_ + 2 * pokerDomain->maxDifferentBets_);
            if (round_ == 1) {
                for (auto &i : pokerDomain->raisesFirstRound_) {
                    if (pokerAction->getValue() == i) break;
                    ++obsId;
                }
            } else {
                obsId += int(pokerDomain->maxDifferentRaises_);
                for (auto &i : pokerDomain->raisesSecondRound_) {
                    if (pokerAction->getValue() == i) break;
                    ++obsId;
                }
            }
            newContinuousRaiseCount = continuousRaiseCount_ + 1;
            currentBet = lastAction_->getValue() + pokerAction->getValue();
            if (currentPlayer == Player(1)) newFirstPlayerReward += currentBet;
            break;

        case Call:
            obsId = Call;
            newContinuousRaiseCount = 0;
            newRound = round_ + 1;
            currentBet = lastAction_->getValue();
            if (currentPlayer == Player(1)) newFirstPlayerReward += currentBet;
            break;

        case Check:
            obsId = Check;
            if (currentPlayer == Player(1)) newRound = round_ + 1;
            break;

        case Bet:
            currentBet = pokerAction->getValue();
            if (currentPlayer == Player(1)) newFirstPlayerReward += currentBet;
            obsId = 3 + pokerDomain->maxCardTypes_;
            if (round_ == 1) {
                for (auto &i : pokerDomain->betsFirstRound_) {
                    if (currentBet == i) break;
                    ++obsId;
                }
            } else {
                obsId += static_cast<int>(pokerDomain->maxDifferentBets_);
                for (auto &i : pokerDomain->betsSecondRound_) {
                    if (currentBet == i) break;
                    ++obsId;
                }
            }
            break;

        case Fold:
            obsId = Fold;
            if (currentPlayer == Player(0))
                newFirstPlayerReward = pot_ - cumulativeFirstPlayerReward_;
            newRound = POKER_TERMINAL_ROUND;
            break;

        default:
            unreachable("unrecognized option!");
    }

    double newPot = pot_ + currentBet;

    if (newRound == 2 && natureCard_ == nullopt
        && (pokerAction->getType() == Call
            || (pokerAction->getType() == Check && currentPlayer == Player(1)))) {
        return revealChanceCard(newFirstPlayerReward, newPot, pokerAction);
    } else {
        return progressRound(newFirstPlayerReward, newContinuousRaiseCount, newPot,
                             currentPlayer, newRound, obsId, pokerAction);
    }
}

OutcomeDistribution GenericPokerState::progressRound(
    double newFirstPlayerReward, double newContinuousRaiseCount, double newPot,
    Player currentPlayer, int newRound, PokerObservationId obsId,
    const shared_ptr<GenericPokerAction> &pokerAction) const {
    auto nextPlayer = opponent(currentPlayer);
    vector<double> rewards(2);
    if (newRound == POKER_TERMINAL_ROUND) {
        nextPlayer = NO_PLAYER;
        int result = hasPlayerOneWon(pokerAction, currentPlayer == Player(0) ? -1 : 1);
        rewards = vector<double>{result * newFirstPlayerReward, -result * newFirstPlayerReward};
    }

    const auto newState = make_shared<GenericPokerState>(
        domain_, player1Card_, player2Card_, natureCard_,
        newFirstPlayerReward, newPot, nextPlayer, newRound,
        pokerAction, newContinuousRaiseCount);

    const auto pubObs = make_shared<GenericPokerObservation>(
        obsId, pokerAction->getType(), pokerAction->getValue());

    return OutcomeDistribution{OutcomeEntry(Outcome(newState, {pubObs, pubObs}, pubObs, rewards))};
}

OutcomeDistribution GenericPokerState::revealChanceCard(double newFirstPlayerReward, double new_pot,
                                                        const shared_ptr<GenericPokerAction> &pokerAction) const {
    const auto pokerDomain = static_cast<const GenericPokerDomain *>(domain_);

    OutcomeDistribution newOutcomes;
    // Nature card is going to be one of the possible types
    for (int natureCard = 0; natureCard < pokerDomain->maxCardTypes_; ++natureCard) {

        // Do not have a chance card in impossible situations
        const bool bothPlayersPlayed = player1Card_ == natureCard
            && player2Card_ == natureCard
            && pokerDomain->maxCardsOfEachType_ == 2;
        const bool onePlayerPlayed = (player1Card_ == natureCard || player2Card_ == natureCard)
            && pokerDomain->maxCardsOfEachType_ == 1;
        if (bothPlayersPlayed || onePlayerPlayed) continue;

        int occurrenceCount = 0;
        if (player1Card_ == natureCard) ++occurrenceCount;
        if (player2Card_ == natureCard) ++occurrenceCount;

        const double prob = double(pokerDomain->maxCardsOfEachType_ - occurrenceCount)
            / (pokerDomain->maxCardTypes_ * pokerDomain->maxCardsOfEachType_ - 2);
        const auto newState = make_shared<GenericPokerState>(
            domain_, player1Card_, player2Card_, natureCard,
            newFirstPlayerReward, new_pot, Player(0), 3, pokerAction, 0);

        const auto observation = make_shared<GenericPokerObservation>(
            3 + natureCard, PlayCard, natureCard);
        Outcome outcome(newState, {observation, observation}, observation, vector<double>(2));
        newOutcomes.emplace_back(OutcomeEntry(outcome, prob));
    }

    return newOutcomes;
}

bool GenericPokerState::operator==(const State &rhs) const {
    auto rhsState = dynamic_cast<const GenericPokerState &>(rhs);
    return hash_ == rhsState.hash_
        && player1Card_ == rhsState.player1Card_
        && (lastAction_ && rhsState.lastAction_
            ? *lastAction_ == *rhsState.lastAction_
            : lastAction_ == rhsState.lastAction_)
        && player2Card_ == rhsState.player2Card_
        && round_ == rhsState.round_
        && pot_ == rhsState.pot_
        && cumulativeFirstPlayerReward_ == rhsState.cumulativeFirstPlayerReward_
        && natureCard_.value_or(-1) == rhsState.natureCard_.value_or(-1)
        && actingPlayer_ == rhsState.actingPlayer_;
}

int GenericPokerState::hasPlayerOneWon(const shared_ptr<GenericPokerAction> &lastAction,
                                       int sign) const {
    if (lastAction->getType() == Fold) return sign;
    if (player1Card_ == player2Card_) return 0;
    if (player1Card_ == natureCard_) return 1;
    if (player2Card_ == natureCard_) return -1;
    if (player1Card_ - player2Card_ > 0) return 1;
    return -1;
}

string GenericPokerState::toString() const {
    return "Player 0 card: " + to_string(player1Card_) +
        "\nPlayer 1 card: " + to_string(player2Card_) +
        "\nNature card: " + to_string(natureCard_.value_or(-1)) +
        "\nActing player: " + to_string(actingPlayer_) +
        "\nPot: " + to_string(pot_) +
        "\nReward for first player: " + to_string(cumulativeFirstPlayerReward_) +
        "\nLast action: " + (lastAction_ ? lastAction_->toString() : "Nothing") +
        "\nRound: " + to_string(round_) +
        "\nContinuous raise count: " + to_string(continuousRaiseCount_) + "\n";
}
}  // namespace GTLib2
