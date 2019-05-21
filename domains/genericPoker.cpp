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
    if (id_ == NO_ACTION)
        return "NoA";
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
            assert(false); // unrecognized option!
    }
}

string GenericPokerObservation::toString() const {
    if (id_ == NO_OBSERVATION)
        return "NoOb";
    switch (type_) {
        case PlayCard:
            return "Card number is " + to_string(value_);
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
            assert(false); // unrecognized option!
    }
}

GenericPokerDomain::GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                                       unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                                       unsigned int maxDifferentRaises, unsigned int ante) :
    Domain(7 + 2 * maxRaisesInRow, 2, true,
           make_shared<GenericPokerAction>(),
           make_shared<GenericPokerObservation>()), maxCardTypes_(maxCardTypes),
    maxCardsOfEachType_(maxCardsOfTypes), maxRaisesInRow_(maxRaisesInRow), ante_(ante),
    maxDifferentBets_(maxDifferentBets), maxDifferentRaises_(maxDifferentRaises) {

    for (int i = 0; i < maxDifferentBets; i++) betsFirstRound_.push_back((i + 1) * 2);
    for (int i = 0; i < maxDifferentRaises; i++) raisesFirstRound_.push_back((i + 1) * 2);
    for (int i : betsFirstRound_) betsSecondRound_.push_back(2 * i);
    for (int i : raisesFirstRound_) raisesSecondRound_.push_back(2 * i);

    maxUtility_ = ante + betsFirstRound_.back() + maxRaisesInRow * raisesFirstRound_.back()
        + betsSecondRound_.back() + maxRaisesInRow * raisesSecondRound_.back();
    vector<double> rewards(2);
    int size = maxCardTypes * maxCardTypes;
    auto next_players = vector<Player>{0};
    for (int p1card = 0; p1card < maxCardTypes; ++p1card) {
        for (int p2card = 0; p2card < maxCardTypes; ++p2card) {
            if (p1card == p2card && maxCardsOfTypes < 2) {
                --size;
                continue;
            }
            int occurencyCount = 0;
            if (p1card == p2card) {
                ++occurencyCount;
            }
            double prob = double(maxCardsOfEachType_) / (maxCardTypes * maxCardsOfEachType_)
                * (maxCardsOfEachType_ - occurencyCount)
                / (maxCardTypes * maxCardsOfEachType_ - 1);
            auto newState = make_shared<GenericPokerState>(this, p1card, p2card, nullopt,
                                                           ante, next_players);
            vector<shared_ptr<Observation>> newObservations{
                make_shared<GenericPokerObservation>(3 + p1card, PlayCard, p1card),
                make_shared<GenericPokerObservation>(3 + p2card, PlayCard, p2card)
            };
            shared_ptr<Observation> publicObservation =
                make_shared<GenericPokerObservation>(3 + p1card, PlayCard, p1card);
            Outcome outcome(newState, newObservations, publicObservation, rewards);

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
    std::stringstream bets1;
    std::copy(betsFirstRound_.begin(), betsFirstRound_.end(),
              std::ostream_iterator<int>(bets1, ", "));
    std::stringstream bets2;
    std::copy(betsSecondRound_.begin(), betsSecondRound_.end(),
              std::ostream_iterator<int>(bets2, ", "));
    std::stringstream raises1;
    std::copy(raisesFirstRound_.begin(), raisesFirstRound_.end(),
              std::ostream_iterator<int>(raises1, ", "));
    std::stringstream raises2;
    std::copy(raisesSecondRound_.begin(), raisesSecondRound_.end(),
              std::ostream_iterator<int>(raises2, ", "));
    return "Generic Poker:\nMax card types: " + to_string(maxCardTypes_) +
        "\nMax cards of each type: " + to_string(maxCardsOfEachType_) +
        "\nMax raises in row: " + to_string(maxRaisesInRow_) +
        "\nMax utility: " + to_string(maxUtility_) + "\nBets first round: [" +
        bets1.str().substr(0, bets1.str().length() - 2) + "]\nBets second round: [" +
        bets2.str().substr(0, bets2.str().length() - 2) + "]\nRaises first round: [" +
        raises1.str().substr(0, raises1.str().length() - 2) + "]\nRaises second round: [" +
        raises2.str().substr(0, raises2.str().length() - 2) + "]\n";
}

// todo: this is only hotfix
unsigned long GenericPokerState::countAvailableActionsFor(Player player) const {
    return getAvailableActionsFor(player).size();
}

vector<shared_ptr<Action>> GenericPokerState::getAvailableActionsFor(Player player) const {
    auto list = vector<shared_ptr<Action>>();
    int count = 0;
    const auto pokerDomain = static_cast<const GenericPokerDomain *>(domain_);
    if (round_ == pokerDomain->TERMINAL_ROUND) {
        return list;
    }
    if (!lastAction_ || lastAction_->GetType() == Check || lastAction_->GetType() == Call) {
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
    } else if (lastAction_->GetType() == Bet || lastAction_->GetType() == Raise) {
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

OutcomeDistribution GenericPokerState::performActions(const vector <shared_ptr<Action>> &actions) const {
    const auto pokerDomain = static_cast<const GenericPokerDomain *>(domain_);
    const auto a1 = dynamic_pointer_cast<GenericPokerAction>(actions[0]);
    const auto a2 = dynamic_pointer_cast<GenericPokerAction>(actions[1]);
    OutcomeDistribution newOutcomes;
    vector<Player> next_players = vector<Player>(1);
    auto newLastAction = lastAction_;
    double bet, new_pot = pot_, newFirstPlayerReward = firstPlayerReward_;
    int newContinuousRaiseCount = continuousRaiseCount_, new_round = round_;
    ObservationId id = NO_OBSERVATION;
    auto observations = vector<shared_ptr<Observation>>(2);
    shared_ptr<GenericPokerState> newState;
    if (a1) {
        switch (a1->GetType()) {
            case Raise:
                id = static_cast<int>(3 + pokerDomain->maxCardTypes_ +
                    2 * pokerDomain->maxDifferentBets_);
                if (round_ == 1) {
                    for (auto &i : pokerDomain->raisesFirstRound_) {
                        if (a1->GetValue() == i) {
                            break;
                        }
                        ++id;
                    }
                } else {
                    id += static_cast<int>(pokerDomain->maxDifferentRaises_);
                    for (auto &i : pokerDomain->raisesSecondRound_) {
                        if (a1->GetValue() == i) {
                            break;
                        }
                        ++id;
                    }
                }
                newContinuousRaiseCount = continuousRaiseCount_ + 1;
                bet = lastAction_->GetValue() + a1->GetValue();
                // bet =  2 * (pot - firstPlayerReward) - pot + a1->GetValue();
                new_pot += bet;
                break;

            case Call:
                id = Call;
                newContinuousRaiseCount = 0;
                new_round = round_ + 1;
                bet = lastAction_->GetValue();  // 2 *(pot - firstPlayerReward) - pot;
                new_pot += bet;
                break;

            case Check:
                id = Check;
                break;
            case Bet:
                bet = a1->GetValue();
                new_pot += bet;
                id = 3 + pokerDomain->maxCardTypes_;
                if (round_ == 1) {
                    for (auto &i : pokerDomain->betsFirstRound_) {
                        if (bet == i) {
                            break;
                        }
                        ++id;
                    }
                } else {
                    id += static_cast<int>(pokerDomain->maxDifferentBets_);
                    for (auto &i : pokerDomain->betsSecondRound_) {
                        if (bet == i) {
                            break;
                        }
                        ++id;
                    }
                }
                break;
            case Fold:
                id = Fold;
                newFirstPlayerReward = pot_ - firstPlayerReward_;
                new_round = pokerDomain->TERMINAL_ROUND;
                break;
            default:
                break;
        }
        if (new_round == 2 && natureCard_ == nullopt && a1->GetType() == Call) {
            for (int i = 0; i < pokerDomain->maxCardTypes_; ++i) {
                if ((player1Card_ == i && player2Card_ == i && pokerDomain->maxCardsOfEachType_ < 3)
                    ||
                        ((player1Card_ == i || player2Card_ == i)
                            && pokerDomain->maxCardsOfEachType_ < 2)) {
                    continue;
                }
                newLastAction = a1;
                next_players[0] = 0;
                int occurrenceCount = 0;
                if (player1Card_ == i)
                    ++occurrenceCount;
                if (player2Card_ == i)
                    ++occurrenceCount;
                double
                    prob = static_cast<double>(pokerDomain->maxCardsOfEachType_ - occurrenceCount) /
                    (pokerDomain->maxCardTypes_ * pokerDomain->maxCardsOfEachType_ - 2);
                newState = make_shared<GenericPokerState>(
                    domain_, player1Card_, player2Card_, i,
                    newFirstPlayerReward, new_pot, next_players, new_round + 1,
                    newLastAction, 0);
                Outcome outcome(newState,
                                vector<shared_ptr<Observation>>
                                    {make_shared<GenericPokerObservation>(3 + i, PlayCard, i),
                                     make_shared<GenericPokerObservation>(3 + i, PlayCard, i)},
                                shared_ptr<Observation>(),
                                vector<double>(2));
                newOutcomes.emplace_back(OutcomeEntry(outcome, prob));
            }
            return newOutcomes;
        }
        newLastAction = a1;
        next_players[0] = 1;
        if (new_round == pokerDomain->TERMINAL_ROUND) {
            next_players.clear();
        }
        newState = make_shared<GenericPokerState>(
            domain_, player1Card_, player2Card_, natureCard_,
            newFirstPlayerReward, new_pot, next_players, new_round,
            newLastAction, newContinuousRaiseCount);
        observations[0] = make_shared<Observation>(NO_OBSERVATION);
        observations[1] = make_shared<GenericPokerObservation>(id, a1->GetType(), a1->GetValue());
    } else if (a2) {
        switch (a2->GetType()) {
            case Raise:
                id = static_cast<int>(3 + pokerDomain->maxCardTypes_ +
                    2 * pokerDomain->maxDifferentBets_);
                if (round_ == 1) {
                    for (auto &i : pokerDomain->raisesFirstRound_) {
                        if (a2->GetValue() == i) {
                            break;
                        }
                        ++id;
                    }
                } else {
                    id += static_cast<int>(pokerDomain->maxDifferentRaises_);
                    for (auto &i : pokerDomain->raisesSecondRound_) {
                        if (a2->GetValue() == i) {
                            break;
                        }
                        ++id;
                    }
                }

                newContinuousRaiseCount = continuousRaiseCount_ + 1;
                bet = lastAction_->GetValue()
                    + a2->GetValue();  // 2*firstPlayerReward-pot+a2->GetValue();
                new_pot += bet;
                newFirstPlayerReward += bet;
                break;

            case Call:
                id = Call;
                newContinuousRaiseCount = 0;
                new_round = round_ + 1;
                bet = lastAction_->GetValue();  // -2 *firstPlayerReward + pot;
                new_pot += bet;
                newFirstPlayerReward += bet;
                break;

            case Check:
                id = Check;
                new_round = round_ + 1;
                break;
            case Bet:
                bet = a2->GetValue();
                new_pot += bet;
                newFirstPlayerReward += bet;
                id = 3 + pokerDomain->maxCardTypes_;
                if (round_ == 1) {
                    for (auto &i : pokerDomain->betsFirstRound_) {
                        if (bet == i) {
                            break;
                        }
                        ++id;
                    }
                } else {
                    id += static_cast<int>(pokerDomain->maxDifferentBets_);
                    for (auto &i : pokerDomain->betsSecondRound_) {
                        if (bet == i) {
                            break;
                        }
                        ++id;
                    }
                }
                break;
            case Fold:
                id = Fold;
                new_round = pokerDomain->TERMINAL_ROUND;
                break;
            default:
                break;
        }
        if (new_round == 2 && natureCard_ == nullopt &&
            (a2->GetType() == Call || a2->GetType() == Check)) {
            for (int i = 0; i < pokerDomain->maxCardTypes_; ++i) {
                if ((player1Card_ == i && player2Card_ == i && pokerDomain->maxCardsOfEachType_ < 3)
                    ||
                        ((player1Card_ == i || player2Card_ == i)
                            && pokerDomain->maxCardsOfEachType_ < 2)) {
                    continue;
                }
                newLastAction = a2;
                next_players[0] = 0;
                int occurrenceCount = 0;
                if (player1Card_ == i)
                    ++occurrenceCount;
                if (player2Card_ == i)
                    ++occurrenceCount;
                double
                    prob = static_cast<double>(pokerDomain->maxCardsOfEachType_ - occurrenceCount) /
                    (pokerDomain->maxCardTypes_ * pokerDomain->maxCardsOfEachType_ - 2);
                newState = make_shared<GenericPokerState>(
                    domain_, player1Card_, player2Card_, i,
                    newFirstPlayerReward, new_pot, next_players, new_round + 1,
                    newLastAction, 0);
                Outcome outcome(newState,
                                vector<shared_ptr<Observation>>
                                    {make_shared<GenericPokerObservation>(3 + i, PlayCard, i),
                                     make_shared<GenericPokerObservation>(3 + i, PlayCard, i)},
                                shared_ptr<Observation>(),
                                vector<double>(2));
                newOutcomes.emplace_back(OutcomeEntry(outcome, prob));
            }
            return newOutcomes;
        }
        next_players[0] = 0;
        if (new_round == pokerDomain->TERMINAL_ROUND) {
            next_players.clear();
        }
        newLastAction = a2;
        newState = make_shared<GenericPokerState>(
            domain_, player1Card_, player2Card_, natureCard_,
            newFirstPlayerReward, new_pot, next_players, new_round,
            newLastAction, newContinuousRaiseCount);
        observations[0] = make_shared<GenericPokerObservation>(id, a2->GetType(), a2->GetValue());
        observations[1] = make_shared<Observation>(NO_OBSERVATION);
    }
    vector<double> rewards(2);
    if (new_round == pokerDomain->TERMINAL_ROUND) {
        int result = hasPlayerOneWon(newLastAction, a1 ? -1 : 1);
        rewards = vector<double>{result * newFirstPlayerReward, -result * newFirstPlayerReward};
    }
    Outcome outcome(newState, move(observations), shared_ptr<Observation>(), rewards);
    newOutcomes.emplace_back(OutcomeEntry(outcome));

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
        && firstPlayerReward_ == rhsState.firstPlayerReward_
        && natureCard_.value_or(-1) == rhsState.natureCard_.value_or(-1)
        && players_ == rhsState.players_;
}

int GenericPokerState::hasPlayerOneWon(const shared_ptr<GenericPokerAction> &lastAction,
                                       Player player) const {
    if (lastAction->GetType() == Fold) return player;
    if (player1Card_ == player2Card_) return 0;
    if (player1Card_ == natureCard_) return 1;
    if (player2Card_ == natureCard_) return -1;
    if (player1Card_ - player2Card_ > 0) return 1;
    return -1;
}

string GenericPokerState::toString() const {
    return "Player 0 card: " + to_string(player1Card_) + "\nPlayer 1 card: " +
        to_string(player2Card_) + "\nNature card: " + to_string(natureCard_.value_or(-1)) +
        "\nPlayer on move: " + to_string(players_[0]) + "\nPot: " + to_string(pot_) +
        "\nReward for first player: " + to_string(firstPlayerReward_) +
        "\nLast action: " + (lastAction_ ? lastAction_->toString() : "Nothing") +
        "\nRound: " + to_string(round_) + "\nContinuous raise count: " +
        to_string(continuousRaiseCount_) + "\n";
}
}  // namespace GTLib2
