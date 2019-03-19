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
#include "genericPoker.h"

#include <iterator>
#include <algorithm>

namespace GTLib2 {
namespace domains {
GenericPokerAction::GenericPokerAction(ActionId id, int type, int value) :
    Action(id), type_(type), value_(value) {}

size_t GenericPokerAction::getHash() const {
  size_t seed = 0;
  boost::hash_combine(seed, type_);
  boost::hash_combine(seed, value_);
  return seed;
}
bool GenericPokerAction::operator==(const Action &that) const {
  if (typeid(*this) == typeid(that)) {
    const auto rhsAction = static_cast<const GenericPokerAction *>(&that);
    return this->type_ == rhsAction->type_ && this->value_ == rhsAction->value_;
  }
  return false;
}

GenericPokerObservation::GenericPokerObservation(int id, int type, int value) :
    Observation(id), type_(type), value_(value) {}

GenericPokerDomain::GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                                       unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                                       unsigned int maxDifferentRaises, unsigned int ante) :
    Domain(7 + 2 * maxRaisesInRow, 2), maxCardTypes(maxCardTypes),
    maxCardsOfEachType(maxCardsOfTypes), maxRaisesInRow(maxRaisesInRow), ante(ante),
    maxDifferentBets(maxDifferentBets), maxDifferentRaises(maxDifferentRaises) {
  for (int i = 0; i < maxDifferentBets; i++) {
    betsFirstRound.push_back((i + 1) * 2);
  }

  /**
   * represents reward which will be added to previous aggressive action
   */

  for (int i = 0; i < maxDifferentRaises; i++) {
    raisesFirstRound.push_back((i + 1) * 2);
  }

  for (int i : betsFirstRound) {
    betsSecondRound.push_back(2 * i);
  }

  for (int i : raisesFirstRound) {
    raisesSecondRound.push_back(2 * i);
  }

  maxUtility = ante + betsFirstRound.back() + maxRaisesInRow * raisesFirstRound.back()
      + betsSecondRound.back() + maxRaisesInRow * raisesSecondRound.back();
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
      double prob = static_cast<double>(maxCardsOfEachType) / (maxCardTypes * maxCardsOfEachType)
          * (maxCardsOfEachType - occurencyCount) / (maxCardTypes * maxCardsOfEachType - 1);
      auto newState = make_shared<GenericPokerState>(this, p1card, p2card, nullopt,
                                                     ante, next_players);
      vector<shared_ptr<Observation>> newObservations{
          make_shared<GenericPokerObservation>(3 + p1card, PlayCard, p1card),
          make_shared<GenericPokerObservation>(3 + p2card, PlayCard, p2card)
      };
      Outcome outcome(newState, newObservations, rewards);

      rootStatesDistribution.emplace_back(move(outcome), prob);
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
  std::copy(betsFirstRound.begin(), betsFirstRound.end(),
            std::ostream_iterator<int>(bets1, ", "));
  std::stringstream bets2;
  std::copy(betsSecondRound.begin(), betsSecondRound.end(),
            std::ostream_iterator<int>(bets2, ", "));
  std::stringstream raises1;
  std::copy(raisesFirstRound.begin(), raisesFirstRound.end(),
            std::ostream_iterator<int>(raises1, ", "));
  std::stringstream raises2;
  std::copy(raisesSecondRound.begin(), raisesSecondRound.end(),
            std::ostream_iterator<int>(raises2, ", "));
  return "Generic Poker:\nMax card types: " + to_string(maxCardTypes) +
      "\nMax cards of each type: " + to_string(maxCardsOfEachType) +
      "\nMax raises in row: " + to_string(maxRaisesInRow) +
      "\nMax utility: " + to_string(maxUtility) + "\nBets first round: [" +
      bets1.str().substr(0, bets1.str().length() - 2) + "]\nBets second round: [" +
      bets2.str().substr(0, bets2.str().length() - 2) + "]\nRaises first round: [" +
      raises1.str().substr(0, raises1.str().length() - 2) + "]\nRaises second round: [" +
      raises2.str().substr(0, raises2.str().length() - 2) + "]\n";
}

vector<shared_ptr<Action>> GenericPokerState::getAvailableActionsFor(Player player) const {
  auto list = vector<shared_ptr<Action>>();
  int count = 0;
  auto pokerDomain = static_cast<GenericPokerDomain *>(domain);
  if (round_ == pokerDomain->TERMINAL_ROUND) {
    return list;
  }
  if (!lastAction || lastAction->GetType() == Check || lastAction->GetType() == Call) {
    if (round_ == 1) {
      for (int betValue : pokerDomain->betsFirstRound) {
        list.push_back(make_shared<GenericPokerAction>(count, Bet, betValue));
        ++count;
      }
    } else if (round_ == 3) {
      for (int betValue : pokerDomain->betsSecondRound) {
        list.push_back(make_shared<GenericPokerAction>(count, Bet, betValue));
        ++count;
      }
    }
    list.push_back(make_shared<GenericPokerAction>(count, Check, 0));
  } else if (lastAction->GetType() == Bet || lastAction->GetType() == Raise) {
    list.push_back(make_shared<GenericPokerAction>(count, Call, 0));
    ++count;
    if (continuousRaiseCount_ < pokerDomain->maxRaisesInRow) {
      if (round_ == 1) {
        for (int raiseValue : pokerDomain->raisesFirstRound) {
          list.push_back(make_shared<GenericPokerAction>(count, Raise, raiseValue));
          ++count;
        }
      } else if (round_ == 3) {
        for (int raiseValue : pokerDomain->raisesSecondRound) {
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
GenericPokerState::performActions(const vector<PlayerAction> &actions) const {
  const auto pokerDomain = static_cast<GenericPokerDomain *>(domain);
  const auto a1 = dynamic_pointer_cast<GenericPokerAction>(actions[0].second);
  const auto a2 = dynamic_pointer_cast<GenericPokerAction>(actions[1].second);
  OutcomeDistribution newOutcomes;
  vector<Player> next_players = vector<Player>(1);
  auto newLastAction = lastAction;
  double bet, new_pot = pot, newFirstPlayerReward = firstPlayerReward;
  int newContinuousRaiseCount = continuousRaiseCount_, new_round = round_;
  ObservationId id = NO_OBSERVATION;
  auto observations = vector<shared_ptr<Observation>>(2);
  shared_ptr<GenericPokerState> newState;
  if (a1) {
    switch (a1->GetType()) {
      case Raise:
        id = static_cast<int>(3 + pokerDomain->maxCardTypes +
            2 * pokerDomain->maxDifferentBets);
        if (round_ == 1) {
          for (auto &i : pokerDomain->raisesFirstRound) {
            if (a1->GetValue() == i) {
              break;
            }
            ++id;
          }
        } else {
          id += static_cast<int>(pokerDomain->maxDifferentRaises);
          for (auto &i : pokerDomain->raisesSecondRound) {
            if (a1->GetValue() == i) {
              break;
            }
            ++id;
          }
        }
        newContinuousRaiseCount = continuousRaiseCount_ + 1;
        bet = lastAction->GetValue() + a1->GetValue();
        // bet =  2 * (pot - firstPlayerReward) - pot + a1->GetValue();
        new_pot += bet;
        break;

      case Call: id = Call;
        newContinuousRaiseCount = 0;
        new_round = round_ + 1;
        bet = lastAction->GetValue();  // 2 *(pot - firstPlayerReward) - pot;
        new_pot += bet;
        break;

      case Check: id = Check;
        break;
      case Bet: bet = a1->GetValue();
        new_pot += bet;
        id = 3 + pokerDomain->maxCardTypes;
        if (round_ == 1) {
          for (auto &i : pokerDomain->betsFirstRound) {
            if (bet == i) {
              break;
            }
            ++id;
          }
        } else {
          id += static_cast<int>(pokerDomain->maxDifferentBets);
          for (auto &i : pokerDomain->betsSecondRound) {
            if (bet == i) {
              break;
            }
            ++id;
          }
        }
        break;
      case Fold: id = Fold;
        newFirstPlayerReward = pot - firstPlayerReward;
        new_round = pokerDomain->TERMINAL_ROUND;
        break;
      default: break;
    }
    if (new_round == 2 && natureCard_ == nullopt && a1->GetType() == Call) {
      for (int i = 0; i < pokerDomain->maxCardTypes; ++i) {
        if ((player1Card_ == i && player2Card_ == i && pokerDomain->maxCardsOfEachType < 3) ||
            ((player1Card_ == i || player2Card_ == i) && pokerDomain->maxCardsOfEachType < 2)) {
          continue;
        }
        newLastAction = a1;
        next_players[0] = 0;
        int occurrenceCount = 0;
        if (player1Card_ == i)
          ++occurrenceCount;
        if (player2Card_ == i)
          ++occurrenceCount;
        double prob = static_cast<double>(pokerDomain->maxCardsOfEachType - occurrenceCount) /
            (pokerDomain->maxCardTypes * pokerDomain->maxCardsOfEachType - 2);
        newState = make_shared<GenericPokerState>(domain,
                                                  player1Card_,
                                                  player2Card_,
                                                  i,
                                                  newFirstPlayerReward,
                                                  new_pot,
                                                  next_players,
                                                  new_round + 1,
                                                  newLastAction,
                                                  0);
        Outcome outcome(newState, vector<shared_ptr<Observation>>
            {make_shared<GenericPokerObservation>(3 + i, PlayCard, i),
             make_shared<GenericPokerObservation>(3 + i, PlayCard, i)}, vector<double>(2));
        newOutcomes.emplace_back(move(outcome), prob);
      }
      return newOutcomes;
    }
    newLastAction = a1;
    next_players[0] = 1;
    if (new_round == pokerDomain->TERMINAL_ROUND) {
      next_players.clear();
    }
    newState = make_shared<GenericPokerState>(domain,
                                              player1Card_,
                                              player2Card_,
                                              natureCard_,
                                              newFirstPlayerReward,
                                              new_pot,
                                              next_players,
                                              new_round,
                                              newLastAction,
                                              newContinuousRaiseCount);
    observations[0] = make_shared<Observation>(NO_OBSERVATION);
    observations[1] = make_shared<GenericPokerObservation>(id, a1->GetType(), a1->GetValue());
  } else if (a2) {
    switch (a2->GetType()) {
      case Raise:
        id = static_cast<int>(3 + pokerDomain->maxCardTypes +
            2 * pokerDomain->maxDifferentBets);
        if (round_ == 1) {
          for (auto &i : pokerDomain->raisesFirstRound) {
            if (a2->GetValue() == i) {
              break;
            }
            ++id;
          }
        } else {
          id += static_cast<int>(pokerDomain->maxDifferentRaises);
          for (auto &i : pokerDomain->raisesSecondRound) {
            if (a2->GetValue() == i) {
              break;
            }
            ++id;
          }
        }

        newContinuousRaiseCount = continuousRaiseCount_ + 1;
        bet = lastAction->GetValue() + a2->GetValue();  // 2*firstPlayerReward-pot+a2->GetValue();
        new_pot += bet;
        newFirstPlayerReward += bet;
        break;

      case Call:id = Call;
        newContinuousRaiseCount = 0;
        new_round = round_ + 1;
        bet = lastAction->GetValue();  // -2 *firstPlayerReward + pot;
        new_pot += bet;
        newFirstPlayerReward += bet;
        break;

      case Check: id = Check;
        new_round = round_ + 1;
        break;
      case Bet:bet = a2->GetValue();
        new_pot += bet;
        newFirstPlayerReward += bet;
        id = 3 + pokerDomain->maxCardTypes;
        if (round_ == 1) {
          for (auto &i : pokerDomain->betsFirstRound) {
            if (bet == i) {
              break;
            }
            ++id;
          }
        } else {
          id += static_cast<int>(pokerDomain->maxDifferentBets);
          for (auto &i : pokerDomain->betsSecondRound) {
            if (bet == i) {
              break;
            }
            ++id;
          }
        }
        break;
      case Fold: id = Fold;
        new_round = pokerDomain->TERMINAL_ROUND;
        break;
      default: break;
    }
    if (new_round == 2 && natureCard_ == nullopt &&
        (a2->GetType() == Call || a2->GetType() == Check)) {
      for (int i = 0; i < pokerDomain->maxCardTypes; ++i) {
        if ((player1Card_ == i && player2Card_ == i && pokerDomain->maxCardsOfEachType < 3) ||
            ((player1Card_ == i || player2Card_ == i) && pokerDomain->maxCardsOfEachType < 2)) {
          continue;
        }
        newLastAction = a2;
        next_players[0] = 0;
        int occurrenceCount = 0;
        if (player1Card_ == i)
          ++occurrenceCount;
        if (player2Card_ == i)
          ++occurrenceCount;
        double prob = static_cast<double>(pokerDomain->maxCardsOfEachType - occurrenceCount) /
            (pokerDomain->maxCardTypes * pokerDomain->maxCardsOfEachType - 2);
        newState = make_shared<GenericPokerState>(domain,
                                                  player1Card_,
                                                  player2Card_,
                                                  i,
                                                  newFirstPlayerReward,
                                                  new_pot,
                                                  next_players,
                                                  new_round + 1,
                                                  newLastAction,
                                                  0);
        Outcome outcome(newState, vector<shared_ptr<Observation>>
            {make_shared<GenericPokerObservation>(3 + i, PlayCard, i),
             make_shared<GenericPokerObservation>(3 + i, PlayCard, i)}, vector<double>(2));
        newOutcomes.emplace_back(move(outcome), prob);
      }
      return newOutcomes;
    }
    next_players[0] = 0;
    if (new_round == pokerDomain->TERMINAL_ROUND) {
      next_players.clear();
    }
    newLastAction = a2;
    newState = make_shared<GenericPokerState>(domain,
                                              player1Card_,
                                              player2Card_,
                                              natureCard_,
                                              newFirstPlayerReward,
                                              new_pot,
                                              next_players,
                                              new_round,
                                              newLastAction,
                                              newContinuousRaiseCount);
    observations[0] = make_shared<GenericPokerObservation>(id, a2->GetType(), a2->GetValue());
    observations[1] = make_shared<Observation>(NO_OBSERVATION);
  }
  vector<double> rewards(2);
  if (new_round == pokerDomain->TERMINAL_ROUND) {
    int result = hasPlayerOneWon(newLastAction, a1 ? -1 : 1);
    rewards = vector<double>{result*newFirstPlayerReward, -result*newFirstPlayerReward};
  }
  Outcome outcome(newState, move(observations), rewards);
  newOutcomes.emplace_back(move(outcome), 1.0);

  return newOutcomes;
}

GenericPokerState::GenericPokerState(Domain *domain,
                                     int p1card,
                                     int p2card,
                                     optional<int> natureCard,
                                     double firstPlayerReward,
                                     double pot,
                                     vector<Player> players,
                                     int round,
                                     shared_ptr<GenericPokerAction> lastAction,
                                     int continuousRaiseCount) :
    State(domain), player1Card_(p1card), player2Card_(p2card), natureCard_(move(natureCard)),
    pot(pot), firstPlayerReward(firstPlayerReward), players_(move(players)), round_(round),
    continuousRaiseCount_(continuousRaiseCount), lastAction(move(lastAction)) {}

GenericPokerState::GenericPokerState(Domain *domain,
                                     int p1card,
                                     int p2card,
                                     optional<int> natureCard,
                                     unsigned int ante,
                                     vector<Player> players) : GenericPokerState(domain,
                                                                              p1card,
                                                                              p2card,
                                                                              move(natureCard),
                                                                              ante,
                                                                              2 * ante,
                                                                              move(players),
                                                                              1,
                                                                              nullptr,
                                                                              0) {}

bool GenericPokerState::operator==(const State &rhs) const {
  auto State = dynamic_cast<const GenericPokerState &>(rhs);
  return player1Card_ == State.player1Card_
      && (lastAction && State.lastAction ? *lastAction == *State.lastAction : lastAction
          == State.lastAction)
      && player2Card_ == State.player2Card_
      && round_ == State.round_
      && pot == State.pot
      && firstPlayerReward == State.firstPlayerReward
      && natureCard_.value_or(-1) == State.natureCard_.value_or(-1)
      && players_ == State.players_;
}
size_t GenericPokerState::getHash() const {
  size_t seed = 0;
  for (auto &i : players_) {
    boost::hash_combine(seed, i);
  }
  boost::hash_combine(seed, player1Card_);
  boost::hash_combine(seed, player2Card_);
  boost::hash_combine(seed, natureCard_.value_or(-1));
  boost::hash_combine(seed, round_);
  boost::hash_combine(seed, continuousRaiseCount_);
  boost::hash_combine(seed, pot);
  boost::hash_combine(seed, firstPlayerReward);
  if (lastAction) {
    boost::hash_combine(seed, lastAction->getHash());
  }
  return seed;
}

int GenericPokerState::hasPlayerOneWon(const shared_ptr<GenericPokerAction> & lastAction,
    int player) const {
  if (lastAction->GetType() == Fold) {
    return player;
  } else if (player1Card_ == player2Card_) {
    return 0;
  } else if (player1Card_ == natureCard_) {
    return 1;
  } else if (player2Card_ == natureCard_) {
    return -1;
  } else if (player1Card_ - player2Card_ > 0) {
    return 1;
  } else {
    return -1;
  }
}
}  // namespace domains
}  // namespace GTLib2
