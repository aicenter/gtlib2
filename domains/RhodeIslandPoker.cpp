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

#include "domains/RhodeIslandPoker.h"


namespace GTLib2::domains {
RhodeIslandPokerAction::RhodeIslandPokerAction(ActionId id, unsigned int type, unsigned int value) :
    Action(id), type_(type), value_(value) {}

bool RhodeIslandPokerAction::operator==(const Action &that) const {
    if (typeid(*this) == typeid(that)) {
        const auto rhsAction = static_cast<const RhodeIslandPokerAction *>(&that);
        return type_ == rhsAction->type_ && value_ == rhsAction->value_;
    }
    return false;
}
string RhodeIslandPokerAction::toString() const {
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
            return "ERROR";
    }
}

RhodeIslandPokerObservation::RhodeIslandPokerObservation(int id, unsigned int type,
                                                         unsigned int value, unsigned int color) :
    Observation(id), value_(value), type_(type), color_(color) {}

string RhodeIslandPokerObservation::toString() const {
    if (id_ == NO_OBSERVATION)
        return "NoOb";
    switch (type_) {
        case PlayCard:
            return "Card is " + to_string(value_) + " " + to_string(color_);
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
            return "ERROR";
    }
}

RhodeIslandPokerDomain::RhodeIslandPokerDomain(unsigned int maxCardTypes,
                                               unsigned int maxCardsOfTypes,
                                               unsigned int maxRaisesInRow,
                                               unsigned int maxDifferentBets,
                                               unsigned int maxDifferentRaises,
                                               unsigned int ante) :
    Domain(10 + 2 * maxRaisesInRow, 2, true,
           make_shared<RhodeIslandPokerAction>(),
           make_shared<RhodeIslandPokerObservation>()),
    maxCardTypes_(maxCardTypes),
    maxCardsOfEachType_(maxCardsOfTypes),
    maxRaisesInRow_(maxRaisesInRow),
    maxDifferentBets_(maxDifferentBets),
    maxDifferentRaises_(maxDifferentRaises),
    ante_(ante) {
    for (unsigned int i = 0; i < maxDifferentBets; i++) {
        betsFirstRound_.push_back((i + 1) * 2);
    }

    /**
     * represents reward which will be added to previous aggressive action
     */

    for (unsigned int i = 0; i < maxDifferentRaises; i++) {
        raisesFirstRound_.push_back((i + 1) * 2);
    }

    for (unsigned int i : betsFirstRound_) {
        betsSecondRound_.push_back(2 * i);
    }

    for (unsigned int i : raisesFirstRound_) {
        raisesSecondRound_.push_back(2 * i);
    }

    for (unsigned int i : betsSecondRound_) {
        betsThirdRound_.push_back(2 * i);
    }

    for (unsigned int i : raisesSecondRound_) {
        raisesThirdRound_.push_back(2 * i);
    }

    maxUtility_ = ante + betsFirstRound_.back() + maxRaisesInRow * raisesFirstRound_.back()
        + betsSecondRound_.back() + maxRaisesInRow * raisesSecondRound_.back()
        + betsThirdRound_.back() + maxRaisesInRow * raisesThirdRound_.back();
    vector<double> rewards(2);
    auto next_players = vector<Player>{0};
    double prob = 1.0 / (maxCardsOfTypes * maxCardTypes * (maxCardTypes * maxCardsOfTypes - 1));
    for (unsigned int color = 0; color < maxCardsOfEachType_; ++color) {
        for (unsigned int color2 = 0; color2 < maxCardsOfEachType_; ++color2) {
            for (unsigned int p1card = 0; p1card < maxCardTypes; ++p1card) {
                for (unsigned int p2card = 0; p2card < maxCardTypes; ++p2card) {
                    if (p1card == p2card && color == color2) {
                        continue;
                    }
                    auto newState = make_shared<RhodeIslandPokerState>(this,
                                                                       make_pair(p1card, color),
                                                                       make_pair(p2card, color2),
                                                                       nullopt,
                                                                       nullopt,
                                                                       ante,
                                                                       next_players);
                    vector<shared_ptr<Observation>>
                        newObservations{make_shared<RhodeIslandPokerObservation>
                                            (3 + p1card + color * maxCardTypes,
                                             PlayCard,
                                             p1card,
                                             color),
                                        make_shared<RhodeIslandPokerObservation>
                                            (3 + p2card + color2 * maxCardTypes,
                                             PlayCard,
                                             p2card,
                                             color2)};
                    Outcome outcome(newState, newObservations, shared_ptr<Observation>(), rewards);

                    rootStatesDistribution_.emplace_back(OutcomeEntry(outcome, prob));
                }
            }
        }
    }
}

RhodeIslandPokerDomain::RhodeIslandPokerDomain(unsigned int maxCardTypes,
                                               unsigned int maxCardsOfTypes) :
    RhodeIslandPokerDomain(maxCardTypes, maxCardsOfTypes, 1, 2, 2, 1) {}

RhodeIslandPokerDomain::RhodeIslandPokerDomain(unsigned int maxCardTypes,
                                               unsigned int maxCardsOfTypes,
                                               unsigned int maxRaisesInRow,
                                               unsigned int maxDifferentBets,
                                               unsigned int maxDifferentRaises) :
    RhodeIslandPokerDomain(maxCardTypes, maxCardsOfTypes, maxRaisesInRow,
                           maxDifferentBets, maxDifferentRaises, 1) {}

RhodeIslandPokerDomain::RhodeIslandPokerDomain() : RhodeIslandPokerDomain(4, 4, 1, 2, 2, 1) {}

string RhodeIslandPokerDomain::getInfo() const {
    std::stringstream bets1;
    std::copy(betsFirstRound_.begin(),
              betsFirstRound_.end(),
              std::ostream_iterator<int>(bets1, ", "));
    std::stringstream bets2;
    std::copy(betsSecondRound_.begin(),
              betsSecondRound_.end(),
              std::ostream_iterator<int>(bets2, ", "));
    std::stringstream bets3;
    std::copy(betsSecondRound_.begin(),
              betsSecondRound_.end(),
              std::ostream_iterator<int>(bets3, ", "));
    std::stringstream raises1;
    std::copy(raisesFirstRound_.begin(),
              raisesFirstRound_.end(),
              std::ostream_iterator<int>(raises1, ", "));
    std::stringstream raises2;
    std::copy(raisesSecondRound_.begin(),
              raisesSecondRound_.end(),
              std::ostream_iterator<int>(raises2, ", "));
    std::stringstream raises3;
    std::copy(raisesSecondRound_.begin(),
              raisesSecondRound_.end(),
              std::ostream_iterator<int>(raises3, ", "));
    return "RhodeIsland Poker:\nMax card types: " + to_string(maxCardTypes_) +
        "\nMax cards of each type: " + to_string(maxCardsOfEachType_) +
        "\nMax raises in row: " + to_string(maxRaisesInRow_) +
        "\nMax utility: " + to_string(maxUtility_) + "\nBets first round: [" +
        bets1.str().substr(0, bets1.str().length() - 2) + "]\nBets second round: [" +
        bets2.str().substr(0, bets2.str().length() - 2) + "]\nBets third round: [" +
        bets3.str().substr(0, bets3.str().length() - 2) + "]\nRaises first round: [" +
        raises1.str().substr(0, raises1.str().length() - 2) + "]\nRaises second round: [" +
        raises2.str().substr(0, raises2.str().length() - 2) + "]\nRaises third round: [" +
        raises3.str().substr(0, raises3.str().length() - 2) + "]\n";
}

// todo: this is only hotfix
unsigned long RhodeIslandPokerState::countAvailableActionsFor(Player player) const {
    return getAvailableActionsFor(player).size();
}

vector<shared_ptr<Action>> RhodeIslandPokerState::getAvailableActionsFor(Player) const {
    auto list = vector<shared_ptr<Action>>();
    int count = 0;
    const auto pokerDomain = static_cast<const RhodeIslandPokerDomain *>(domain_);
    if (round_ == pokerDomain->TERMINAL_ROUND) {
        return list;
    }
    if (!lastAction_ || lastAction_->GetType() == Check || lastAction_->GetType() == Call) {
        if (round_ == 1) {
            for (int betValue : pokerDomain->betsFirstRound_) {
                list.push_back(make_shared<RhodeIslandPokerAction>(count, Bet, betValue));
                ++count;
            }
        } else if (round_ == 3) {
            for (int betValue : pokerDomain->betsSecondRound_) {
                list.push_back(make_shared<RhodeIslandPokerAction>(count, Bet, betValue));
                ++count;
            }
        } else if (round_ == 5) {
            for (int betValue : pokerDomain->betsThirdRound_) {
                list.push_back(make_shared<RhodeIslandPokerAction>(count, Bet, betValue));
                ++count;
            }
        }
        list.push_back(make_shared<RhodeIslandPokerAction>(count, Check, 0));
    } else if (lastAction_->GetType() == Bet || lastAction_->GetType() == Raise) {
        list.push_back(make_shared<RhodeIslandPokerAction>(count, Call, 0));
        ++count;
        if (continuousRaiseCount_ < pokerDomain->maxRaisesInRow_) {
            if (round_ == 1) {
                for (int raiseValue : pokerDomain->raisesFirstRound_) {
                    list.push_back(make_shared<RhodeIslandPokerAction>(count, Raise, raiseValue));
                    ++count;
                }
            } else if (round_ == 3) {
                for (int raiseValue : pokerDomain->raisesSecondRound_) {
                    list.push_back(make_shared<RhodeIslandPokerAction>(count, Raise, raiseValue));
                    ++count;
                }
            } else if (round_ == 5) {
                for (int raiseValue : pokerDomain->raisesThirdRound_) {
                    list.push_back(make_shared<RhodeIslandPokerAction>(count, Raise, raiseValue));
                    ++count;
                }
            }
        }
        list.push_back(make_shared<RhodeIslandPokerAction>(count, Fold, 0));
    }
    return list;
}

OutcomeDistribution
RhodeIslandPokerState::performActions(const vector<shared_ptr<Action>> &actions) const {
    const auto pokerDomain = static_cast<const RhodeIslandPokerDomain *>(domain_);
    auto a1 = dynamic_pointer_cast<RhodeIslandPokerAction>(actions[0]);
    auto a2 = dynamic_pointer_cast<RhodeIslandPokerAction>(actions[1]);
    auto observations = vector<shared_ptr<Observation>>(2);
    OutcomeDistribution newOutcomes;
    vector<Player> next_players = vector<Player>(1);
    auto newLastAction = lastAction_;
    double bet, new_pot = pot_, newFirstPlayerReward = firstPlayerReward_;
    int newContinuousRaiseCount = continuousRaiseCount_, new_round = round_;
    ObservationId id = NO_OBSERVATION;

    shared_ptr<RhodeIslandPokerState> newState;
    if (a1) {
        switch (a1->GetType()) {
            case Raise:
                id = static_cast<int>(3
                    + pokerDomain->maxCardsOfEachType_ * pokerDomain->maxCardTypes_
                    + 2 * pokerDomain->maxDifferentBets_);
                switch (round_) {
                    case 1:
                        for (auto &i : pokerDomain->raisesFirstRound_) {
                            if (a1->GetValue() == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    case 3:
                        id += static_cast<int>(pokerDomain->maxDifferentRaises_);
                        for (auto &i : pokerDomain->raisesSecondRound_) {
                            if (a1->GetValue() == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    case 5:
                        id += 2 * static_cast<int>(pokerDomain->maxDifferentRaises_);
                        for (auto &i : pokerDomain->raisesThirdRound_) {
                            if (a1->GetValue() == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    default:
                        break;
                }
                newContinuousRaiseCount = continuousRaiseCount_ + 1;
                bet = lastAction_->GetValue()
                    + a1->GetValue();  // 2 *(pot - firstPlayerReward) - pot + a1->GetValue();
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
                id = 3 + pokerDomain->maxCardsOfEachType_ * pokerDomain->maxCardTypes_;
                switch (round_) {
                    case 1:
                        for (auto &i : pokerDomain->betsFirstRound_) {
                            if (bet == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    case 3:
                        id += static_cast<int>(pokerDomain->maxDifferentBets_);
                        for (auto &i : pokerDomain->betsSecondRound_) {
                            if (bet == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    case 5:
                        id += 2 * static_cast<int>(pokerDomain->maxDifferentBets_);
                        for (auto &i : pokerDomain->betsThirdRound_) {
                            if (bet == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    default:
                        break;
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
        if (new_round == 2 && natureCard1_ == nullopt && a1->GetType() == Call) {
            double prob = 1.0 / (pokerDomain->maxCardsOfEachType_ * pokerDomain->maxCardTypes_ - 2);
            for (unsigned int j = 0; j < pokerDomain->maxCardsOfEachType_; ++j) {
                for (unsigned int i = 0; i < pokerDomain->maxCardTypes_; ++i) {
                    if ((player1Card_.first == i && player1Card_.second == j)
                        || (player2Card_.first == i && player2Card_.second == j)) {
                        continue;
                    }
                    newLastAction = a1;
                    next_players[0] = 0;
                    newState = make_shared<RhodeIslandPokerState>(
                        domain_, player1Card_, player2Card_, make_pair(i, j),
                        nullopt, newFirstPlayerReward, new_pot, next_players, new_round + 1,
                        newLastAction, 0);
                    Outcome outcome(newState,
                                    vector<shared_ptr<Observation>>{
                                        make_shared<RhodeIslandPokerObservation>(
                                            3 + i + j * pokerDomain->maxCardTypes_,
                                            PlayCard,
                                            i,
                                            j),
                                        make_shared<RhodeIslandPokerObservation>(
                                            3 + i + j * pokerDomain->maxCardTypes_,
                                            PlayCard,
                                            i,
                                            j)},
                                    shared_ptr<Observation>(),
                                    vector<double>(2));
                    newOutcomes.emplace_back(OutcomeEntry(outcome, prob));
                }
            }
            return newOutcomes;
        } else if (new_round == 4 && natureCard2_ == nullopt && a1->GetType() == Call) {
            double prob = 1.0 / (pokerDomain->maxCardsOfEachType_ * pokerDomain->maxCardTypes_ - 3);
            for (unsigned int j = 0; j < pokerDomain->maxCardsOfEachType_; ++j) {
                for (unsigned int i = 0; i < pokerDomain->maxCardTypes_; ++i) {
                    if ((player1Card_.first == i && player1Card_.second == j)
                        || (player2Card_.first == i && player2Card_.second == j)
                        || (natureCard1_.value().first == i && natureCard1_.value().second == j)) {
                        continue;
                    }
                    newLastAction = a1;
                    next_players[0] = 0;
                    newState =
                        make_shared<RhodeIslandPokerState>(
                            domain_, player1Card_, player2Card_, natureCard1_,
                            make_pair(i, j), newFirstPlayerReward, new_pot, next_players,
                            new_round + 1, newLastAction, 0);
                    Outcome outcome(newState,
                                    vector<shared_ptr<Observation>>{
                                        make_shared<RhodeIslandPokerObservation>(
                                            3 + i + j * pokerDomain->maxCardTypes_,
                                            PlayCard,
                                            i,
                                            j),
                                        make_shared<RhodeIslandPokerObservation>(
                                            3 + i + j * pokerDomain->maxCardTypes_,
                                            PlayCard,
                                            i,
                                            j)},
                                    shared_ptr<Observation>(),
                                    vector<double>(2));
                    newOutcomes.emplace_back(OutcomeEntry(outcome, prob));
                }
            }
            return newOutcomes;
        }
        newLastAction = a1;
        next_players[0] = 1;
        if (new_round == pokerDomain->TERMINAL_ROUND) {
            next_players.clear();
        }
        newState = make_shared<RhodeIslandPokerState>(
            domain_, player1Card_, player2Card_, natureCard1_, natureCard2_,
            newFirstPlayerReward, new_pot, next_players, new_round, newLastAction,
            newContinuousRaiseCount);
        observations[0] = make_shared<Observation>(NO_OBSERVATION);
        observations[1] =
            make_shared<RhodeIslandPokerObservation>(id, a1->GetType(), a1->GetValue(), 0);

    } else if (a2) {
        switch (a2->GetType()) {
            case Raise:
                id = static_cast<int>(3
                    + pokerDomain->maxCardTypes_ * pokerDomain->maxCardsOfEachType_ +
                    2 * pokerDomain->maxDifferentBets_);
                switch (round_) {
                    case 1:
                        for (auto &i : pokerDomain->raisesFirstRound_) {
                            if (a2->GetValue() == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    case 3:
                        id += static_cast<int>(pokerDomain->maxDifferentRaises_);
                        for (auto &i : pokerDomain->raisesSecondRound_) {
                            if (a2->GetValue() == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    case 5:
                        id += 2 * static_cast<int>(pokerDomain->maxDifferentRaises_);
                        for (auto &i : pokerDomain->raisesThirdRound_) {
                            if (a2->GetValue() == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    default:
                        break;
                }

                newContinuousRaiseCount = continuousRaiseCount_ + 1;
                bet =
                    lastAction_->GetValue() + a2->GetValue();  // 2 *firstPlayerReward - pot +
                // a2->GetValue();
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
                id = 3 + pokerDomain->maxCardTypes_ * pokerDomain->maxCardsOfEachType_;
                switch (round_) {
                    case 1:
                        for (auto &i : pokerDomain->betsFirstRound_) {
                            if (bet == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    case 3:
                        id += static_cast<int>(pokerDomain->maxDifferentBets_);
                        for (auto &i : pokerDomain->betsSecondRound_) {
                            if (bet == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    case 5:
                        id += 2 * static_cast<int>(pokerDomain->maxDifferentBets_);
                        for (auto &i : pokerDomain->betsSecondRound_) {
                            if (bet == i) {
                                break;
                            }
                            ++id;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case Fold:
                id = Fold;
                new_round = pokerDomain->TERMINAL_ROUND;
                break;
            default:
                break;
        }
        if (new_round == 2 && natureCard1_ == nullopt
            && (a2->GetType() == Call || a2->GetType() == Check)) {
            double prob = 1.0 / (pokerDomain->maxCardsOfEachType_ * pokerDomain->maxCardTypes_ - 2);
            for (unsigned int j = 0; j < pokerDomain->maxCardsOfEachType_; ++j) {
                for (unsigned int i = 0; i < pokerDomain->maxCardTypes_; ++i) {
                    if ((player1Card_.first == i && player1Card_.second == j)
                        || (player2Card_.first == i && player2Card_.second == j)) {
                        continue;
                    }
                    newLastAction = a2;
                    next_players[0] = 0;
                    newState = make_shared<RhodeIslandPokerState>(
                        domain_, player1Card_, player2Card_, make_pair(i, j),
                        nullopt, newFirstPlayerReward, new_pot, next_players, new_round + 1,
                        newLastAction, 0);
                    Outcome outcome(newState,
                                    vector<shared_ptr<Observation>>{
                                        make_shared<RhodeIslandPokerObservation>(
                                            3 + i + j * pokerDomain->maxCardTypes_,
                                            PlayCard,
                                            i,
                                            j),
                                        make_shared<RhodeIslandPokerObservation>(
                                            3 + i + j * pokerDomain->maxCardTypes_,
                                            PlayCard,
                                            i,
                                            j)},
                                    shared_ptr<Observation>(),
                                    vector<double>(2));

                    newOutcomes.emplace_back(OutcomeEntry(outcome, prob));
                }
                return newOutcomes;
            }
        } else if (new_round == 4 && natureCard2_ == nullopt
            && (a2->GetType() == Call || a2->GetType() == Check)) {
            double prob = 1.0 / (pokerDomain->maxCardsOfEachType_ * pokerDomain->maxCardTypes_ - 3);
            for (unsigned int j = 0; j < pokerDomain->maxCardsOfEachType_; ++j) {
                for (unsigned int i = 0; i < pokerDomain->maxCardTypes_; ++i) {
                    if ((player1Card_.first == i && player1Card_.second == j)
                        || (player2Card_.first == i && player2Card_.second == j)
                        || (natureCard1_.value().first == i && natureCard1_.value().second == j)) {
                        continue;
                    }
                    newLastAction = a2;
                    next_players[0] = 0;

                    newState = make_shared<RhodeIslandPokerState>(
                        domain_, player1Card_, player2Card_, natureCard1_,
                        make_pair(i, j), newFirstPlayerReward, new_pot, next_players,
                        new_round + 1, newLastAction, 0);
                    Outcome outcome(newState,
                                    vector<shared_ptr<Observation>>{
                                        make_shared<RhodeIslandPokerObservation>(
                                            3 + i + j * pokerDomain->maxCardTypes_,
                                            PlayCard,
                                            i,
                                            j),
                                        make_shared<RhodeIslandPokerObservation>(
                                            3 + i + j * pokerDomain->maxCardTypes_,
                                            PlayCard,
                                            i,
                                            j)},
                                    shared_ptr<Observation>(),
                                    vector<double>(2));

                    newOutcomes.emplace_back(OutcomeEntry(outcome, prob));
                }
            }
            return newOutcomes;
        }
        next_players[0] = 0;
        if (new_round == pokerDomain->TERMINAL_ROUND) {
            next_players.clear();
        }
        newLastAction = a2;
        newState = make_shared<RhodeIslandPokerState>(
            domain_, player1Card_, player2Card_, natureCard1_, natureCard2_,
            newFirstPlayerReward, new_pot, next_players, new_round, newLastAction,
            newContinuousRaiseCount);
        observations[0] =
            make_shared<RhodeIslandPokerObservation>(id, a2->GetType(), a2->GetValue(), 0);
        observations[1] = make_shared<Observation>(NO_OBSERVATION);
    }

    vector<double> rewards(2);

    if (new_round == pokerDomain->TERMINAL_ROUND) {
        int result = hasPlayerOneWon(newLastAction, a1 ? -1 : 1);
        rewards = vector<double>{result * newFirstPlayerReward, -result * newFirstPlayerReward};
    }
    Outcome outcome(newState, observations, shared_ptr<Observation>(), rewards);
    newOutcomes.emplace_back(OutcomeEntry(outcome));

    return newOutcomes;
}

RhodeIslandPokerState::RhodeIslandPokerState(const Domain *domain,
                                             pair<unsigned int, unsigned int> player1Card,
                                             pair<unsigned int, unsigned int> player2Card,
                                             optional<pair<unsigned int, unsigned int>> natureCard1,
                                             optional<pair<unsigned int, unsigned int>> natureCard2,
                                             double firstPlayerReward, double pot,
                                             vector<Player> players, int round,
                                             shared_ptr<RhodeIslandPokerAction> lastAction,
                                             unsigned int continuousRaiseCount) :
    State(domain, hashCombine(498413684454, players, player1Card, player2Card,
                              natureCard1.value_or(make_pair(-1, -1)),
                              natureCard2.value_or(make_pair(-1, -1)),
                              firstPlayerReward, pot, players,
                              round, lastAction, continuousRaiseCount)),
    player1Card_(move(player1Card)),
    player2Card_(move(player2Card)),
    natureCard1_(move(natureCard1)),
    natureCard2_(move(natureCard2)),
    pot_(pot),
    firstPlayerReward_(firstPlayerReward),
    players_(move(players)),
    round_(round),
    continuousRaiseCount_(continuousRaiseCount),
    lastAction_(move(lastAction)) {}

RhodeIslandPokerState::RhodeIslandPokerState(const Domain *domain,
                                             pair<unsigned int, unsigned int> player1card,
                                             pair<unsigned int, unsigned int> player2card,
                                             optional<pair<unsigned int, unsigned int>> natureCard1,
                                             optional<pair<unsigned int, unsigned int>> natureCard2,
                                             unsigned int ante,
                                             vector<Player> players) :
    RhodeIslandPokerState(domain, move(player1card), move(player2card), move(natureCard1),
                          move(natureCard2), ante, 2 * ante, move(players), 1, nullptr, 0) {}


bool RhodeIslandPokerState::operator==(const State &rhs) const {
    auto rhsState = dynamic_cast<const RhodeIslandPokerState &>(rhs);
    return hash_ == rhsState.hash_
        && player1Card_ == rhsState.player1Card_
        && player2Card_ == rhsState.player2Card_
        && round_ == rhsState.round_
        && pot_ == rhsState.pot_
        && firstPlayerReward_ == rhsState.firstPlayerReward_
        && natureCard1_ == rhsState.natureCard1_
        && natureCard2_ == rhsState.natureCard2_
        && players_ == rhsState.players_
        && lastAction_ == rhsState.lastAction_;
}
int RhodeIslandPokerState::hasPlayerOneWon(
    const shared_ptr<RhodeIslandPokerAction> &lastAction, Player player) const {

    if (lastAction->GetType() == Fold) return player;
    if (natureCard1_.value().second == natureCard2_.value().second &&
        natureCard2_.value().first + 1 == natureCard1_.value().first) {
        if (natureCard2_.value().second == player1Card_.second &&
            (natureCard2_.value().first - 1 == player1Card_.first
                || natureCard1_.value().first + 1 == player1Card_.first)) {
            return 1;
        } else if (natureCard2_.value().second == player2Card_.second &&
            (natureCard2_.value().first - 1 == player2Card_.first
                || natureCard1_.value().first + 1 == player2Card_.first)) {
            return -1;
        }
    }
    if (natureCard1_.value().second == natureCard2_.value().second &&
        natureCard2_.value().first - 1 == natureCard1_.value().first) {
        if (natureCard2_.value().second == player1Card_.second &&
            (natureCard2_.value().first + 1 == player1Card_.first
                || natureCard1_.value().first - 1 == player1Card_.first)) {
            return 1;
        } else if (natureCard2_.value().second == player2Card_.second &&
            (natureCard2_.value().first + 1 == player2Card_.first
                || natureCard1_.value().first - 1 == player2Card_.first)) {
            return -1;
        }
    }
    if (natureCard2_.value().first == natureCard1_.value().first) {
        if (natureCard2_.value().first == player1Card_.first) {
            return 1;
        } else if (natureCard2_.value().first == player2Card_.first) {
            return -1;
        }
    }
    if (natureCard2_.value().first + 1 == natureCard1_.value().first) {
        if (natureCard2_.value().first - 1 == player1Card_.first
            || natureCard1_.value().first + 1 == player1Card_.first) {
            return 1;
        } else if (natureCard2_.value().first - 1 == player2Card_.first
            || natureCard1_.value().first + 1 == player2Card_.first) {
            return -1;
        }
    }
    if (natureCard2_.value().first - 1 == natureCard1_.value().first) {
        if (natureCard2_.value().first + 1 == player1Card_.first
            || natureCard1_.value().first - 1 == player1Card_.first) {
            return 1;
        } else if (natureCard2_.value().first + 1 == player2Card_.first
            || natureCard1_.value().first - 1 == player2Card_.first) {
            return -1;
        }
    }
    if (natureCard2_.value().second == natureCard1_.value().second) {
        if (natureCard2_.value().second == player1Card_.second) {
            return 1;
        } else if (natureCard2_.value().second == player2Card_.second) {
            return -1;
        }
    }
    if (player1Card_.first == player2Card_.first) return 0;
    if (player1Card_.first == natureCard1_.value().first
        || player1Card_.first == natureCard2_.value().first)
        return 1;
    if (player2Card_.first == natureCard1_.value().first
        || player2Card_.first == natureCard2_.value().first)
        return -1;
    if (player1Card_.first - player2Card_.first > 0) return 1;
    return -1;
}
string RhodeIslandPokerState::toString() const {
    string
        s =
        "Player 1 card: " + to_string(player1Card_.first) + " " + to_string(player1Card_.second)
            + "\nPlayer 2 card: " + to_string(player2Card_.first) + " "
            + to_string(player2Card_.second)
            +
                "\nNature cards: ";
    if (natureCard1_) {
        s +=
            to_string(natureCard1_.value().first) + " " + to_string(natureCard1_.value().second)
                + "  |  ";
        if (natureCard2_) {
            s += to_string(natureCard2_.value().first) + " "
                + to_string(natureCard2_.value().second);
        }
    }
    return s + "\nPlayer on move: " + to_string(players_[0]) + "\nPot: " + to_string(pot_) +
        "\nReward for first player: " + to_string(firstPlayerReward_) + "\nLast action: " +
        lastAction_->toString() + "\nRound: " + to_string(round_) + "Continuous raise count: " +
        to_string(continuousRaiseCount_) + "\n";
}

}  // namespace GTLib2
