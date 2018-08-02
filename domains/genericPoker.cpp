//
// Created by Jakub Rozlivek on 7/19/18.
//

#include "genericPoker.h"
#include <utility>
#include <iterator>
#include <sstream>

namespace GTLib2 {

  GenericPokerAction::GenericPokerAction(int id, int type, int value):
          Action(id), type_(type), value_(value) {}

  GenericPokerObservation::GenericPokerObservation(int id, int type, int value):
          Observation(id), type_(type), value_(value) {}

  GenericPokerDomain::GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                                         unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                                         unsigned int maxDifferentRaises, unsigned int ante):
          Domain(7+2*maxRaisesInRow, 2), maxCardTypes(maxCardTypes), maxCardsOfEachType(maxCardsOfTypes), 
          maxRaisesInRow(maxRaisesInRow), ante(ante) {

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
            + betsSecondRound.back()+ maxRaisesInRow * raisesSecondRound.back();

    vector<double> rewards(2);
    int size = maxCardTypes*maxCardTypes;
    auto next_players = vector<int>{0};
    for(int p1card = 0; p1card < maxCardTypes; ++p1card) {
      for (int p2card = 0; p2card < maxCardTypes; ++p2card) {
        if(p1card == p2card && maxCardsOfTypes < 2) {
          --size;
          continue;
        }
        auto newState = make_shared<GenericPokerState>(make_shared<GenericPokerDomain>(*this),
                p1card, p2card, nullopt, ante, next_players);
        vector<shared_ptr<Observation>> newObservations{make_shared<GenericPokerObservation>
                (3+p1card, PlayCard, p1card), make_shared<GenericPokerObservation>(3+p2card, PlayCard, p2card)};
        Outcome outcome(newState, newObservations, rewards);

        rootStatesDistribution.emplace_back(outcome,1.0/(size));
      }
    }
  }

  GenericPokerDomain::GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes):
          GenericPokerDomain(maxCardTypes, maxCardsOfTypes, 1, 2, 2, 1) {}

  GenericPokerDomain::GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                                         unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                                         unsigned int maxDifferentRaises):
          GenericPokerDomain(maxCardTypes, maxCardsOfTypes, maxRaisesInRow,
                  maxDifferentBets, maxDifferentRaises, 1) {}

  GenericPokerDomain::GenericPokerDomain(): GenericPokerDomain(4, 3, 1, 2, 2, 1) {}

  string GenericPokerDomain::getInfo() const {

    std::stringstream bets1;
    std::copy(betsFirstRound.begin(), betsFirstRound.end(), std::ostream_iterator<int>(bets1, ", "));
    std::stringstream bets2;
    std::copy(betsSecondRound.begin(), betsSecondRound.end(), std::ostream_iterator<int>(bets2, ", "));
    std::stringstream raises1;
    std::copy(raisesFirstRound.begin(), raisesFirstRound.end(), std::ostream_iterator<int>(raises1, ", "));
    std::stringstream raises2;
    std::copy(raisesSecondRound.begin(), raisesSecondRound.end(), std::ostream_iterator<int>(raises2, ", "));
    return "Generic Poker:\nMax card types: " + to_string(maxCardTypes) +
           "\nMax cards of each type: "+to_string(maxCardsOfEachType) +
           "\nMax raises in row: " + to_string(maxRaisesInRow) +
           "\nMax utility: " + to_string(maxUtility) + "\nBets first round: [" +
           bets1.str().substr(0, bets1.str().length()-2) + "]\nBets second round: [" +
           bets2.str().substr(0, bets2.str().length()-2) + "]\nRaises first round: [" +
           raises1.str().substr(0, raises1.str().length()-2) + "]\nRaises second round: [" +
           raises2.str().substr(0, raises2.str().length()-2) + "]\n";
  }


  vector<shared_ptr<Action>> GenericPokerState::getAvailableActionsFor(int player) const {
    auto list = vector<shared_ptr<Action>>();
    int count = 0;
    auto pokerDomain = dynamic_pointer_cast<GenericPokerDomain>(domain);
    if(round_ == pokerDomain->TERMINAL_ROUND) {
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
  GenericPokerState::performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const {
    auto action1 = std::find_if( actions.begin(), actions.end(),
                                [](pair<int, shared_ptr<Action>> const & elem) { return elem.first == 0; })->second;
    auto action2 = std::find_if( actions.begin(), actions.end(),
                                 [](pair<int, shared_ptr<Action>> const & elem) { return elem.first == 1; })->second;
    auto pokerDomain = dynamic_pointer_cast<GenericPokerDomain>(domain);
    auto a1 = dynamic_pointer_cast<GenericPokerAction>(action1);
    auto a2 = dynamic_pointer_cast<GenericPokerAction>(action2);
    a1 = a1? a1: make_shared<GenericPokerAction>(-1,-1,-1);
    a2 = a2? a2: make_shared<GenericPokerAction>(-1,-1,-1);
    auto observations = vector<shared_ptr<Observation>>(2);
    auto rewards = vector<double>(2);
    OutcomeDistribution newOutcomes;
    vector<int> next_players = vector<int>(1);
    observations.reserve(2);
    auto newLastAction = lastAction;
    rewards.reserve(2);
    double bet, new_pot = pot, newFirstPlayerReward = firstPlayerReward;
    int newContinuousRaiseCount = continuousRaiseCount_, new_round = round_;
    int id = -1;
    int raisePosition = 0, betPosition = 0;

    shared_ptr<GenericPokerState> newState;
    if (a1->getId() > -1) {
      switch (a1->GetType()) {
        case Raise:
          id = static_cast<int>(3 + pokerDomain->maxCardTypes + pokerDomain->betsFirstRound.size() + pokerDomain->betsSecondRound.size());
          for(auto &i : pokerDomain->raisesFirstRound){
            if(a1->GetValue() == i) {
              break;
            }
            ++raisePosition;
          }
          if(raisePosition == pokerDomain->raisesSecondRound.size()){
            for(auto &i : pokerDomain->raisesSecondRound){
              if(a1->GetValue() == i) {
                break;
              }
              ++raisePosition;
            }
          }
          id += raisePosition;
          newContinuousRaiseCount = continuousRaiseCount_ + 1;
          bet = 2 *(pot - firstPlayerReward) - pot + a1->GetValue();
          new_pot = pot+ bet;
          newFirstPlayerReward = firstPlayerReward;
          break;

        case Call:
          id = Call;
          newContinuousRaiseCount = 0;
          new_round = round_ + 1;
          bet =  2 *(pot - firstPlayerReward) - pot;
          new_pot = pot+ bet;
          newFirstPlayerReward = firstPlayerReward;
          break;

        case Check: id = Check; break;
        case Bet:
          bet = a1->GetValue();
          id = 3+pokerDomain->maxCardTypes;
          for(auto &i : pokerDomain->betsFirstRound){
            if(bet == i) {
              break;
            }
            ++betPosition;
          }
          if(betPosition == pokerDomain->betsSecondRound.size()){
            for(auto &i : pokerDomain->betsSecondRound){
              if(bet == i) {
                break;
              }
              ++betPosition;
            }
          }
          id += betPosition;
        break;
        case Fold: id = Fold; new_round = pokerDomain->TERMINAL_ROUND; break;
        default: break;
      }
      if (new_round == 2 && natureCard_ == nullopt && a1->GetType() == Call) {
        int size = pokerDomain->maxCardTypes;
        for(int i = 0; i < pokerDomain->maxCardTypes; ++i) {
          if ((player1Card_ == i && player2Card_ == i && pokerDomain->maxCardsOfEachType < 3) ||
              ((player1Card_ == i || player2Card_ == i) && pokerDomain->maxCardsOfEachType < 2)) {
            --size;
          }
        }
        for(int i = 0; i < pokerDomain->maxCardTypes; ++i) {
          if ((player1Card_ == i && player2Card_ == i && pokerDomain->maxCardsOfEachType < 3) ||
              ((player1Card_ == i || player2Card_ == i)&& pokerDomain->maxCardsOfEachType < 2)) {
            continue;
          }
          newLastAction = a1;
          next_players[0] =0;
          newState = make_shared<GenericPokerState>(domain, player1Card_, player2Card_, i,
                  newFirstPlayerReward, new_pot, next_players, new_round+1, newLastAction, 0);
          observations[0] = make_shared<GenericPokerObservation>(3+i, PlayCard, i);
          observations[1] = make_shared<GenericPokerObservation>(3+i, PlayCard, i);
          Outcome outcome(newState, observations, rewards);
          newOutcomes.emplace_back(outcome,1.0/size);
        }
        return newOutcomes;
      }
      newLastAction = a1;
      next_players[0] = 1;
      if(new_round == pokerDomain->TERMINAL_ROUND) {
        next_players.clear();
      }
      newState = make_shared<GenericPokerState>(domain, player1Card_, player2Card_, natureCard_,
              newFirstPlayerReward, new_pot, next_players, new_round, newLastAction, newContinuousRaiseCount);
      observations[0] = make_shared<GenericPokerObservation>(-1,-1, -1);
      observations[1] = make_shared<GenericPokerObservation>(id, a1->GetType(), a1->GetValue());

    } else if (a2->getId() > -1) {
      switch (a2->GetType()) {
        case Raise:
          id = static_cast<int>(3 + pokerDomain->maxCardTypes + pokerDomain->betsFirstRound.size() + pokerDomain->betsSecondRound.size());
          for(auto &i : pokerDomain->raisesFirstRound){
            if(a2->GetValue() == i) {
              break;
            }
            ++raisePosition;
          }
          if(raisePosition == pokerDomain->raisesSecondRound.size()){
            for(auto &i : pokerDomain->raisesSecondRound){
              if(a2->GetValue() == i) {
                break;
              }
              ++raisePosition;
            }
          }
          id += raisePosition;
          newContinuousRaiseCount = continuousRaiseCount_ + 1;
          bet = 2 *firstPlayerReward - pot + a2->GetValue();
          new_pot = pot+ bet;
          newFirstPlayerReward = firstPlayerReward + bet;
          break;

        case Call:
          id = Call;
          newContinuousRaiseCount = 0;
          new_round = round_ + 1;
          bet = 2 *firstPlayerReward - pot;
          new_pot = pot+ bet;
          newFirstPlayerReward = firstPlayerReward + bet;
          break;

        case Check: id = Check; new_round = round_ + 1; break;
        case Bet:
          bet = a2->GetValue();
          id = 3+pokerDomain->maxCardTypes;
          for(auto &i : pokerDomain->betsFirstRound){
            if(bet == i) {
              break;
            }
            ++betPosition;
          }
          if(betPosition == pokerDomain->betsSecondRound.size()){
            for(auto &i : pokerDomain->betsSecondRound){
              if(bet == i) {
                break;
              }
              ++betPosition;
            }
          }
          id += betPosition;
          break;
        case Fold: id = Fold; new_round = pokerDomain->TERMINAL_ROUND; break;
        default: break;
      }
      if (new_round == 2 && natureCard_ == nullopt && (a2->GetType() == Call || a2->GetType() == Check)) {
        int size = pokerDomain->maxCardTypes;
        for(int i = 0; i < pokerDomain->maxCardTypes; ++i) {
          if ((player1Card_ == i && player2Card_ == i && pokerDomain->maxCardsOfEachType < 3) ||
              ((player1Card_ == i || player2Card_ == i) && pokerDomain->maxCardsOfEachType < 2)) {
            --size;
          }
        }
        for(int i = 0; i < pokerDomain->maxCardTypes; ++i) {
          if ((player1Card_ == i && player2Card_ == i && pokerDomain->maxCardsOfEachType < 3) ||
              ((player1Card_ == i || player2Card_ == i)&& pokerDomain->maxCardsOfEachType < 2)) {
            continue;
          }
          newLastAction = a2;
          next_players[0] =0;
          newState = make_shared<GenericPokerState>(domain, player1Card_, player2Card_, i, newFirstPlayerReward,
                  new_pot, next_players, new_round+1, newLastAction, 0);
          observations[0] = make_shared<GenericPokerObservation>(3+i, PlayCard, i);
          observations[1] = make_shared<GenericPokerObservation>(3+i, PlayCard, i);
          Outcome outcome(newState, observations, rewards);
          newOutcomes.emplace_back(outcome,1.0/size);
        }
        return newOutcomes;
      }
      next_players[0] =0;
      if(new_round == pokerDomain->TERMINAL_ROUND) {
        next_players.clear();
      }
      newLastAction = a2;
      newState = make_shared<GenericPokerState>(domain, player1Card_, player2Card_, natureCard_, newFirstPlayerReward,
              new_pot, next_players, new_round, newLastAction, newContinuousRaiseCount);
      observations[0] = make_shared<GenericPokerObservation>(id, a2->GetType(), a2->GetValue());
      observations[1] = make_shared<GenericPokerObservation>(-1,-1,-1);
    }

    if(new_round == 4) {
      if (newLastAction->GetType() == Fold) {
        if(a1->getId() > -1) {
          rewards[0] = -new_pot;
          rewards[1] = new_pot;
        } else {
          rewards[0] = new_pot;
          rewards[1] = -new_pot;
        }
      } else if (player1Card_ == player2Card_) {
        rewards[0] = 0;
        rewards[1] = 0;
      } else if (player1Card_ == natureCard_) {
        rewards[0] = new_pot;
        rewards[1] = -new_pot;
      } else if (player2Card_ == natureCard_) {
        rewards[0] = -new_pot;
        rewards[1] = new_pot;
      } else if (player1Card_-player2Card_ > 0) {
        rewards[0] = new_pot;
        rewards[1] = -new_pot;
      } else {
        rewards[0] = -new_pot;
        rewards[1] = new_pot;
      }
    }

    Outcome outcome(newState, observations, rewards);
    newOutcomes.emplace_back(outcome,1.0);

    return newOutcomes;
  }

  GenericPokerState::GenericPokerState(const shared_ptr<Domain> &domain, int p1card, int p2card, optional<int> natureCard,  
                                       double firstPlayerReward, double pot, vector<int> players, int round,
           shared_ptr<GenericPokerAction> lastAction, int continuousRaiseCount):
          State(domain) , player1Card_(p1card), player2Card_(p2card), natureCard_(move(natureCard)), pot(pot), firstPlayerReward(firstPlayerReward),
          players_(move(players)), round_(round), continuousRaiseCount_(continuousRaiseCount), lastAction(
          move(lastAction)) {}

  GenericPokerState::GenericPokerState(const shared_ptr<Domain> &domain, int p1card, int p2card,
                                       optional<int> natureCard, unsigned int ante, vector<int> players):
  GenericPokerState(domain, p1card, p2card, move(natureCard), ante, 2*ante, move(players), 1, nullptr, 0) {}

}
