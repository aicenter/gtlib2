//
// Created by Jakub Rozlivek on 8/1/18.
//

#include "RhodeIslandPoker.h"
#include <random>
#include <iterator>
#include <sstream>

using std::make_pair;

namespace GTLib2 {
namespace domains {
RhodeIslandPokerAction::RhodeIslandPokerAction(int id, int type, int value) :
    Action(id), type_(type), value_(value) {}

bool RhodeIslandPokerAction::operator==(const Action &that) const {
  const auto rhsAction = dynamic_cast<const RhodeIslandPokerAction *>(&that);
  return this->type_ == rhsAction->type_ && this->value_ == rhsAction->value_;
}

size_t RhodeIslandPokerAction::getHash() const {
  size_t seed = 0;
  boost::hash_combine(seed, type_);
  boost::hash_combine(seed, value_);
  return seed;
}

RhodeIslandPokerObservation::RhodeIslandPokerObservation(int id, int type, int value, int color) :
    Observation(id), type_(type), value_(value), color_(color) {}

RhodeIslandPokerDomain::RhodeIslandPokerDomain(unsigned int maxCardTypes,
                                               unsigned int maxCardsOfTypes,
                                               unsigned int maxRaisesInRow,
                                               unsigned int maxDifferentBets,
                                               unsigned int maxDifferentRaises,
                                               unsigned int ante) :
    Domain(10 + 2 * maxRaisesInRow, 2),
    maxCardTypes(maxCardTypes),
    maxCardsOfEachType(maxCardsOfTypes),
    maxRaisesInRow(maxRaisesInRow),
    ante(ante),
    maxDifferentBets(maxDifferentBets),
    maxDifferentRaises(maxDifferentRaises) {
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

  for (int i : betsSecondRound) {
    betsThirdRound.push_back(2 * i);
  }

  for (int i : raisesSecondRound) {
    raisesThirdRound.push_back(2 * i);
  }

  maxUtility = ante + betsFirstRound.back() + maxRaisesInRow * raisesFirstRound.back()
      + betsSecondRound.back() + maxRaisesInRow * raisesSecondRound.back()
      + betsThirdRound.back() + maxRaisesInRow * raisesThirdRound.back();
  vector<double> rewards(2);
  auto next_players = vector<int>{0};
  double prob = 1.0 / (maxCardsOfTypes * maxCardTypes * (maxCardTypes * maxCardsOfTypes - 1));
  for (int color = 0; color < maxCardsOfEachType; ++color) {
    for (int color2 = 0; color2 < maxCardsOfEachType; ++color2) {
      for (int p1card = 0; p1card < maxCardTypes; ++p1card) {
        for (int p2card = 0; p2card < maxCardTypes; ++p2card) {
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
          vector<shared_ptr<Observation>> newObservations{make_shared<RhodeIslandPokerObservation>
                                                              (3 + p1card + color * maxCardTypes,
                                                               PlayCard,
                                                               p1card,
                                                               color),
                                                          make_shared<RhodeIslandPokerObservation>
                                                              (3 + p2card + color2 * maxCardTypes,
                                                              PlayCard,
                                                              p2card,
                                                              color2)};
          Outcome outcome(newState, newObservations, rewards);

          rootStatesDistribution.emplace_back(outcome, prob);
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
  std::copy(betsFirstRound.begin(), betsFirstRound.end(), std::ostream_iterator<int>(bets1, ", "));
  std::stringstream bets2;
  std::copy(betsSecondRound.begin(),
            betsSecondRound.end(),
            std::ostream_iterator<int>(bets2, ", "));
  std::stringstream bets3;
  std::copy(betsSecondRound.begin(),
            betsSecondRound.end(),
            std::ostream_iterator<int>(bets3, ", "));
  std::stringstream raises1;
  std::copy(raisesFirstRound.begin(),
            raisesFirstRound.end(),
            std::ostream_iterator<int>(raises1, ", "));
  std::stringstream raises2;
  std::copy(raisesSecondRound.begin(),
            raisesSecondRound.end(),
            std::ostream_iterator<int>(raises2, ", "));
  std::stringstream raises3;
  std::copy(raisesSecondRound.begin(),
            raisesSecondRound.end(),
            std::ostream_iterator<int>(raises3, ", "));
  return "RhodeIsland Poker:\nMax card types: " + to_string(maxCardTypes) +
      "\nMax cards of each type: " + to_string(maxCardsOfEachType) +
      "\nMax raises in row: " + to_string(maxRaisesInRow) +
      "\nMax utility: " + to_string(maxUtility) + "\nBets first round: [" +
      bets1.str().substr(0, bets1.str().length() - 2) + "]\nBets second round: [" +
      bets2.str().substr(0, bets2.str().length() - 2) + "]\nBets third round: [" +
      bets3.str().substr(0, bets3.str().length() - 2) + "]\nRaises first round: [" +
      raises1.str().substr(0, raises1.str().length() - 2) + "]\nRaises second round: [" +
      raises2.str().substr(0, raises2.str().length() - 2) + "]\nRaises third round: [" +
      raises3.str().substr(0, raises3.str().length() - 2) + "]\n";
}

vector<shared_ptr<Action>> RhodeIslandPokerState::getAvailableActionsFor(int player) const {
  auto list = vector<shared_ptr<Action>>();
  int count = 0;
  auto pokerDomain = dynamic_cast<RhodeIslandPokerDomain *>(domain);
  if (round_ == pokerDomain->TERMINAL_ROUND) {
    return list;
  }
  if (!lastAction || lastAction->GetType() == Check || lastAction->GetType() == Call) {
    if (round_ == 1) {
      for (int betValue : pokerDomain->betsFirstRound) {
        list.push_back(make_shared<RhodeIslandPokerAction>(count, Bet, betValue));
        ++count;
      }
    } else if (round_ == 3) {
      for (int betValue : pokerDomain->betsSecondRound) {
        list.push_back(make_shared<RhodeIslandPokerAction>(count, Bet, betValue));
        ++count;
      }
    } else if (round_ == 5) {
      for (int betValue : pokerDomain->betsThirdRound) {
        list.push_back(make_shared<RhodeIslandPokerAction>(count, Bet, betValue));
        ++count;
      }
    }
    list.push_back(make_shared<RhodeIslandPokerAction>(count, Check, 0));
  } else if (lastAction->GetType() == Bet || lastAction->GetType() == Raise) {
    list.push_back(make_shared<RhodeIslandPokerAction>(count, Call, 0));
    ++count;
    if (continuousRaiseCount_ < pokerDomain->maxRaisesInRow) {
      if (round_ == 1) {
        for (int raiseValue : pokerDomain->raisesFirstRound) {
          list.push_back(make_shared<RhodeIslandPokerAction>(count, Raise, raiseValue));
          ++count;
        }
      } else if (round_ == 3) {
        for (int raiseValue : pokerDomain->raisesSecondRound) {
          list.push_back(make_shared<RhodeIslandPokerAction>(count, Raise, raiseValue));
          ++count;
        }
      } else if (round_ == 5) {
        for (int raiseValue : pokerDomain->raisesThirdRound) {
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
RhodeIslandPokerState::performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const {
  auto pokerDomain = dynamic_cast<RhodeIslandPokerDomain *>(domain);
  auto
      a1 = dynamic_pointer_cast<RhodeIslandPokerAction>(std::find_if(actions.begin(), actions.end(),
          [](pair<int, shared_ptr<Action>> const &elem) { return elem.first == 0; })->second);
  auto
      a2 = dynamic_pointer_cast<RhodeIslandPokerAction>(std::find_if(actions.begin(), actions.end(),
          [](pair<int, shared_ptr<Action>> const &elem) { return elem.first == 1; })->second);
  auto observations = vector<shared_ptr<Observation>>(2);
  OutcomeDistribution newOutcomes;
  vector<int> next_players = vector<int>(1);
  auto newLastAction = lastAction;
  double bet, new_pot = pot, newFirstPlayerReward = firstPlayerReward;
  int newContinuousRaiseCount = continuousRaiseCount_, new_round = round_;
  int id = -1;

  shared_ptr<RhodeIslandPokerState> newState;
  if (a1) {
    switch (a1->GetType()) {
      case Raise:
        id = static_cast<int>(3 + pokerDomain->maxCardsOfEachType * pokerDomain->maxCardTypes
            + 2 * pokerDomain->maxDifferentBets);
        switch (round_) {
          case 1:
            for (auto &i : pokerDomain->raisesFirstRound) {
              if (a1->GetValue() == i) {
                break;
              }
              ++id;
            }
            break;
          case 3: id += static_cast<int>(pokerDomain->maxDifferentRaises);
            for (auto &i : pokerDomain->raisesSecondRound) {
              if (a1->GetValue() == i) {
                break;
              }
              ++id;
            }
            break;
          case 5: id += 2 * static_cast<int>(pokerDomain->maxDifferentRaises);
            for (auto &i : pokerDomain->raisesThirdRound) {
              if (a1->GetValue() == i) {
                break;
              }
              ++id;
            }
            break;
          default: break;
        }
        newContinuousRaiseCount = continuousRaiseCount_ + 1;
        bet = lastAction->GetValue()
            + a1->GetValue();  // 2 *(pot - firstPlayerReward) - pot + a1->GetValue();
        new_pot += bet;
        break;

      case Call:id = Call;
        newContinuousRaiseCount = 0;
        new_round = round_ + 1;
        bet = lastAction->GetValue();  // 2 *(pot - firstPlayerReward) - pot;
        new_pot += bet;
        break;

      case Check: id = Check;
        break;
      case Bet:bet = a1->GetValue();
        new_pot += bet;
        id = 3 + pokerDomain->maxCardsOfEachType * pokerDomain->maxCardTypes;
        switch (round_) {
          case 1:
            for (auto &i : pokerDomain->betsFirstRound) {
              if (bet == i) {
                break;
              }
              ++id;
            }
            break;
          case 3: id += static_cast<int>(pokerDomain->maxDifferentBets);
            for (auto &i : pokerDomain->betsSecondRound) {
              if (bet == i) {
                break;
              }
              ++id;
            }
            break;
          case 5: id += 2 * static_cast<int>(pokerDomain->maxDifferentBets);
            for (auto &i : pokerDomain->betsThirdRound) {
              if (bet == i) {
                break;
              }
              ++id;
            }
            break;
          default: break;
        }
        break;
      case Fold: id = Fold;
        newFirstPlayerReward = pot - firstPlayerReward;
        new_round = pokerDomain->TERMINAL_ROUND;
        break;
      default: break;
    }
    if (new_round == 2 && natureCard1_ == nullopt && a1->GetType() == Call) {
      double prob = 1.0 / (pokerDomain->maxCardsOfEachType * pokerDomain->maxCardTypes - 2);
      for (int j = 0; j < pokerDomain->maxCardsOfEachType; ++j) {
        for (int i = 0; i < pokerDomain->maxCardTypes; ++i) {
          if ((player1Card_.first == i && player1Card_.second == j)
              || (player2Card_.first == i && player2Card_.second == j)) {
            continue;
          }
          newLastAction = a1.get();
          next_players[0] = 0;
          newState = make_shared<RhodeIslandPokerState>(domain,
                                                        player1Card_,
                                                        player2Card_,
                                                        make_pair(i, j),
                                                        nullopt,
                                                        newFirstPlayerReward,
                                                        new_pot,
                                                        next_players,
                                                        new_round + 1,
                                                        newLastAction,
                                                        0);
          Outcome outcome(newState, vector<shared_ptr<Observation>>{
              make_shared<RhodeIslandPokerObservation>(3 + i + j * pokerDomain->maxCardTypes,
                                                       PlayCard,
                                                       i,
                                                       j),
              make_shared<RhodeIslandPokerObservation>(3 + i + j * pokerDomain->maxCardTypes,
                                                       PlayCard,
                                                       i,
                                                       j)}, vector<double>(2));
          newOutcomes.emplace_back(outcome, prob);
        }
      }
      return newOutcomes;
    } else if (new_round == 4 && natureCard2_ == nullopt && a1->GetType() == Call) {
      double prob = 1.0 / (pokerDomain->maxCardsOfEachType * pokerDomain->maxCardTypes - 3);
      for (int j = 0; j < pokerDomain->maxCardsOfEachType; ++j) {
        for (int i = 0; i < pokerDomain->maxCardTypes; ++i) {
          if ((player1Card_.first == i && player1Card_.second == j)
              || (player2Card_.first == i && player2Card_.second == j)
              || (natureCard1_.value().first == i && natureCard1_.value().second == j)) {
            continue;
          }
          newLastAction = a1.get();
          next_players[0] = 0;
          newState = make_shared<RhodeIslandPokerState>(domain, player1Card_, player2Card_,
                                                        natureCard1_, make_pair(i, j),
                                                        newFirstPlayerReward, new_pot, next_players,
                                                        new_round + 1, newLastAction, 0);
          Outcome outcome(newState, vector<shared_ptr<Observation>>{
              make_shared<RhodeIslandPokerObservation>(3 + i + j * pokerDomain->maxCardTypes,
                                                       PlayCard,
                                                       i,
                                                       j),
              make_shared<RhodeIslandPokerObservation>(3 + i + j * pokerDomain->maxCardTypes,
                                                       PlayCard,
                                                       i,
                                                       j)}, vector<double>(2));
          newOutcomes.emplace_back(outcome, prob);
        }
      }
      return newOutcomes;
    }
    newLastAction = a1.get();
    next_players[0] = 1;
    if (new_round == pokerDomain->TERMINAL_ROUND) {
      next_players.clear();
    }
    newState = make_shared<RhodeIslandPokerState>(domain,
                                                  player1Card_,
                                                  player2Card_,
                                                  natureCard1_,
                                                  natureCard2_,
                                                  newFirstPlayerReward,
                                                  new_pot,
                                                  next_players,
                                                  new_round,
                                                  newLastAction,
                                                  newContinuousRaiseCount);
    observations[0] = make_shared<Observation>(-1);
    observations[1] =
        make_shared<RhodeIslandPokerObservation>(id, a1->GetType(), a1->GetValue(), 0);

  } else if (a2) {
    switch (a2->GetType()) {
      case Raise:
        id = static_cast<int>(3 + pokerDomain->maxCardTypes * pokerDomain->maxCardsOfEachType +
            2 * pokerDomain->maxDifferentBets);
        switch (round_) {
          case 1:
            for (auto &i : pokerDomain->raisesFirstRound) {
              if (a2->GetValue() == i) {
                break;
              }
              ++id;
            }
            break;
          case 3:id += static_cast<int>(pokerDomain->maxDifferentRaises);
            for (auto &i : pokerDomain->raisesSecondRound) {
              if (a2->GetValue() == i) {
                break;
              }
              ++id;
            }
            break;
          case 5:id += 2 * static_cast<int>(pokerDomain->maxDifferentRaises);
            for (auto &i : pokerDomain->raisesThirdRound) {
              if (a2->GetValue() == i) {
                break;
              }
              ++id;
            }
            break;
          default: break;
        }

        newContinuousRaiseCount = continuousRaiseCount_ + 1;
        bet =
            lastAction->GetValue() + a2->GetValue();  // 2 *firstPlayerReward - pot +
        // a2->GetValue();
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
        id = 3 + pokerDomain->maxCardTypes * pokerDomain->maxCardsOfEachType;
        switch (round_) {
          case 1:
            for (auto &i : pokerDomain->betsFirstRound) {
              if (bet == i) {
                break;
              }
              ++id;
            }
            break;
          case 3:id += static_cast<int>(pokerDomain->maxDifferentBets);
            for (auto &i : pokerDomain->betsSecondRound) {
              if (bet == i) {
                break;
              }
              ++id;
            }
            break;
          case 5:id += 2 * static_cast<int>(pokerDomain->maxDifferentBets);
            for (auto &i : pokerDomain->betsSecondRound) {
              if (bet == i) {
                break;
              }
              ++id;
            }
            break;
          default: break;
        }
        break;
      case Fold: id = Fold;
        new_round = pokerDomain->TERMINAL_ROUND;
        break;
      default: break;
    }
    if (new_round == 2 && natureCard1_ == nullopt
        && (a2->GetType() == Call || a2->GetType() == Check)) {
      double prob = 1.0 / (pokerDomain->maxCardsOfEachType * pokerDomain->maxCardTypes - 2);
      for (int j = 0; j < pokerDomain->maxCardsOfEachType; ++j) {
        for (int i = 0; i < pokerDomain->maxCardTypes; ++i) {
          if ((player1Card_.first == i && player1Card_.second == j)
              || (player2Card_.first == i && player2Card_.second == j)) {
            continue;
          }
          newLastAction = a2.get();
          next_players[0] = 0;
          newState = make_shared<RhodeIslandPokerState>(domain,
                                                        player1Card_,
                                                        player2Card_,
                                                        make_pair(i, j),
                                                        nullopt,
                                                        newFirstPlayerReward,
                                                        new_pot,
                                                        next_players,
                                                        new_round + 1,
                                                        newLastAction,
                                                        0);
          Outcome outcome(newState, vector<shared_ptr<Observation>>{
              make_shared<RhodeIslandPokerObservation>(3 + i + j * pokerDomain->maxCardTypes,
                                                       PlayCard,
                                                       i,
                                                       j),
              make_shared<RhodeIslandPokerObservation>(3 + i + j * pokerDomain->maxCardTypes,
                                                       PlayCard,
                                                       i,
                                                       j)}, vector<double>(2));

          newOutcomes.emplace_back(outcome, prob);
        }
        return newOutcomes;
      }
    } else if (new_round == 4 && natureCard2_ == nullopt
        && (a2->GetType() == Call || a2->GetType() == Check)) {
      double prob = 1.0 / (pokerDomain->maxCardsOfEachType * pokerDomain->maxCardTypes - 3);
      for (int j = 0; j < pokerDomain->maxCardsOfEachType; ++j) {
        for (int i = 0; i < pokerDomain->maxCardTypes; ++i) {
          if ((player1Card_.first == i && player1Card_.second == j)
              || (player2Card_.first == i && player2Card_.second == j)
              || (natureCard1_.value().first == i && natureCard1_.value().second == j)) {
            continue;
          }
          newLastAction = a2.get();
          next_players[0] = 0;

          newState = make_shared<RhodeIslandPokerState>(domain,
                                                        player1Card_,
                                                        player2Card_,
                                                        natureCard1_,
                                                        make_pair(i, j),
                                                        newFirstPlayerReward,
                                                        new_pot,
                                                        next_players,
                                                        new_round + 1,
                                                        newLastAction,
                                                        0);
          Outcome outcome(newState, vector<shared_ptr<Observation>>{
              make_shared<RhodeIslandPokerObservation>(3 + i + j * pokerDomain->maxCardTypes,
                                                       PlayCard,
                                                       i,
                                                       j),
              make_shared<RhodeIslandPokerObservation>(3 + i + j * pokerDomain->maxCardTypes,
                                                       PlayCard,
                                                       i,
                                                       j)}, vector<double>(2));

          newOutcomes.emplace_back(outcome, prob);
        }
      }
      return newOutcomes;
    }
    next_players[0] = 0;
    if (new_round == pokerDomain->TERMINAL_ROUND) {
      next_players.clear();
    }
    newLastAction = a2.get();
    newState = make_shared<RhodeIslandPokerState>(domain,
                                                  player1Card_,
                                                  player2Card_,
                                                  natureCard1_,
                                                  natureCard2_,
                                                  newFirstPlayerReward,
                                                  new_pot,
                                                  next_players,
                                                  new_round,
                                                  newLastAction,
                                                  newContinuousRaiseCount);
    observations[0] =
        make_shared<RhodeIslandPokerObservation>(id, a2->GetType(), a2->GetValue(), 0);
    observations[1] = make_shared<Observation>(-1);
  }

  vector<double> rewards(2);

  if (new_round == pokerDomain->TERMINAL_ROUND) {  // TODO: check it
    if (newLastAction->GetType() == Fold) {
      rewards =
          a1 ? vector{-newFirstPlayerReward, newFirstPlayerReward} : vector{newFirstPlayerReward,
                                                                            -newFirstPlayerReward};
    } else if (natureCard1_.value().second == natureCard2_.value().second &&
        natureCard2_.value().first + 1 == natureCard1_.value().first) {
      if (natureCard2_.value().second == player1Card_.second &&
          (natureCard2_.value().first + 1 == player1Card_.first
              || natureCard1_.value().first - 1 == player1Card_.first)) {
        rewards = vector<double>{newFirstPlayerReward, -newFirstPlayerReward};
      } else if (natureCard2_.value().second == player2Card_.second &&
          (natureCard2_.value().first + 1 == player2Card_.first
              || natureCard1_.value().first - 1 == player2Card_.first)) {
        rewards = vector<double>{-newFirstPlayerReward, newFirstPlayerReward};
      }
    } else if (natureCard1_.value().second == natureCard2_.value().second &&
        natureCard2_.value().first - 1 == natureCard1_.value().first) {
      if (natureCard2_.value().second == player1Card_.second &&
          (natureCard2_.value().first - 1 == player1Card_.first
              || natureCard1_.value().first + 1 == player1Card_.first)) {
        rewards = vector<double>{newFirstPlayerReward, -newFirstPlayerReward};
      } else if (natureCard2_.value().second == player2Card_.second &&
          (natureCard2_.value().first - 1 == player2Card_.first
              || natureCard1_.value().first + 1 == player2Card_.first)) {
        rewards = vector<double>{-newFirstPlayerReward, newFirstPlayerReward};
      }
    } else if (natureCard2_.value().first == natureCard1_.value().first) {
      if (natureCard2_.value().first == player1Card_.first) {
        rewards = vector<double>{newFirstPlayerReward, -newFirstPlayerReward};
      } else if (natureCard2_.value().first == player2Card_.first) {
        rewards = vector<double>{-newFirstPlayerReward, newFirstPlayerReward};
      }
    } else if (natureCard2_.value().first + 1 == natureCard1_.value().first) {
      if (natureCard2_.value().first + 1 == player1Card_.first
          || natureCard1_.value().first - 1 == player1Card_.first) {
        rewards = vector<double>{newFirstPlayerReward, -newFirstPlayerReward};
      } else if (natureCard2_.value().first + 1 == player2Card_.first
          || natureCard1_.value().first - 1 == player2Card_.first) {
        rewards = vector<double>{-newFirstPlayerReward, newFirstPlayerReward};
      }
    } else if (natureCard2_.value().first - 1 == natureCard1_.value().first) {
      if (natureCard2_.value().first - 1 == player1Card_.first
          || natureCard1_.value().first + 1 == player1Card_.first) {
        rewards = vector<double>{newFirstPlayerReward, -newFirstPlayerReward};
      } else if (natureCard2_.value().first - 1 == player2Card_.first
          || natureCard1_.value().first + 1 == player2Card_.first) {
        rewards = vector<double>{-newFirstPlayerReward, newFirstPlayerReward};
      }
    } else if (natureCard2_.value().second == natureCard1_.value().second) {
      if (natureCard2_.value().second == player1Card_.second) {
        rewards = vector<double>{newFirstPlayerReward, -newFirstPlayerReward};
      } else if (natureCard2_.value().second == player2Card_.second) {
        rewards = vector<double>{-newFirstPlayerReward, newFirstPlayerReward};
      }
    } else if (player1Card_.first == player2Card_.first) {
    } else if (player1Card_.first == natureCard1_.value().first
        || player1Card_.first == natureCard2_.value().first) {
      rewards = vector<double>{newFirstPlayerReward, -newFirstPlayerReward};
    } else if (player2Card_.first == natureCard1_.value().first
        || player2Card_.first == natureCard2_.value().first) {
      rewards = vector<double>{-newFirstPlayerReward, newFirstPlayerReward};
    } else if (player1Card_.first - player2Card_.first > 0) {
      rewards = vector<double>{newFirstPlayerReward, -newFirstPlayerReward};
    } else {
      rewards = vector<double>{-newFirstPlayerReward, newFirstPlayerReward};
    }
  }
  Outcome outcome(newState, observations, rewards);
  newOutcomes.emplace_back(outcome, 1.0);

  return newOutcomes;
}

RhodeIslandPokerState::RhodeIslandPokerState(Domain *domain,
                                             pair<int, int> p1card,
                                             pair<int, int> p2card,
                                             optional<pair<int, int>> natureCard1,
                                             optional<pair<int, int>> natureCard2,
                                             double firstPlayerReward,
                                             double pot,
                                             vector<int> players,
                                             int round,
                                             RhodeIslandPokerAction *lastAction,
                                             int continuousRaiseCount) :
    State(domain),
    player1Card_(move(p1card)),
    player2Card_(move(p2card)),
    natureCard1_(move(natureCard1)),
    natureCard2_(move(natureCard2)),
    pot(pot),
    firstPlayerReward(firstPlayerReward),
    players_(move(players)),
    round_(round),
    continuousRaiseCount_(continuousRaiseCount),
    lastAction(lastAction) {}

RhodeIslandPokerState::RhodeIslandPokerState(Domain *domain,
                                             pair<int, int> p1card,
                                             pair<int, int> p2card,
                                             optional<pair<int, int>> natureCard1,
                                             optional<pair<int, int>> natureCard2,
                                             unsigned int ante,
                                             vector<int> players) :
    RhodeIslandPokerState(domain, move(p1card), move(p2card), move(natureCard1),
                          move(natureCard2), ante, 2 * ante, move(players), 1, nullptr, 0) {}
size_t RhodeIslandPokerState::getHash() const {
  size_t seed = 0;
  for (auto &i : players_) {
    boost::hash_combine(seed, i);
  }
  boost::hash_combine(seed, player1Card_);
  boost::hash_combine(seed, player2Card_);
  boost::hash_combine(seed, *natureCard1_);
  boost::hash_combine(seed, *natureCard2_);
  boost::hash_combine(seed, round_);
  boost::hash_combine(seed, continuousRaiseCount_);
  boost::hash_combine(seed, pot);
  boost::hash_combine(seed, firstPlayerReward);
  boost::hash_combine(seed, lastAction);
  return seed;
}

bool RhodeIslandPokerState::operator==(const State &rhs) const {
    auto State = dynamic_cast<const RhodeIslandPokerState &>(rhs);

    return player1Card_ == State.player1Card_ &&
        player2Card_ == State.player2Card_ &&
        round_ == State.round_ &&
        pot == State.pot &&
        firstPlayerReward == State.firstPlayerReward &&
        natureCard1_ == State.natureCard1_ &&
        natureCard2_ == State.natureCard2_ &&
        players_ == State.players_ &&
        lastAction == State.lastAction;
}
}  // namespace domains
}  // namespace GTLib2
