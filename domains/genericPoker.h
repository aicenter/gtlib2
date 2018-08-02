//
// Created by Jakub Rozlivek on 7/19/18.
//

#ifndef GTLIB2_GENERICPOKER_H
#define GTLIB2_GENERICPOKER_H


#include "../base/base.h"
#include <experimental/optional>
#include <random>

using std::experimental::nullopt;
using std::experimental::optional;

using namespace GTLib2;

namespace GTLib2 {
  enum MOVES {
    Check, Call, Fold, Bet, Raise, PlayCard
  };

/**
 * GenericPokerAction is a class that represents Generic Poker actions,
 * which are identified by their id and contain card number or move.
 */
  class GenericPokerAction : public Action {
   public:
    // constructor
    GenericPokerAction(int id,int type, int value);

    // Returns move description.
    inline string toString() const final {
      if (id == -1)
        return "NoA";
      switch (type_) {
        case Check: return "Check";
        case Bet: return "Bet: " + to_string(value_);
        case Call: return "Call";
        case Raise: return "Raise: " + to_string(value_);
        case Fold: return "Fold";
        default: return "ERROR" ;
      }
    }

    inline int GetValue() const {
      return value_;
    }

    inline int GetType() const {
      return type_;
    }

   private:
    int value_;
    int type_;
  };


/**
 * GenericPokerObservation is a class that represents Generic Poker observations,
 * which are identified by their id and contain an integer value
 * indicating if an action was successful(1) or not (0).
 */
  class GenericPokerObservation : public Observation {
   public:
    // constructor
    explicit GenericPokerObservation(int id, int type, int value);
    /**
     * id: 0 - check; 1 - call; 2 - fold; from 3 to 3+maxCardTypes - played cards;
     * then next BetsFirstRound.size() numbers - bets in first round
     * then next BetsSecondRound.size() numbers - bets in second round
     * then next RaisesFirstRound.size() numbers - raises in first round
     * then next RaisesSecondRound.size() numbers - raises in second round
     */

    // Returns description.
    inline string toString() const final {
      if (id == -1)
        return "NoOb";
      switch (type_) {
        case PlayCard: return "Card number is " + to_string(value_);
        case Check: return "Check";
        case Bet: return "Bet:"+ to_string(value_);
        case Call: return "Call";
        case Raise: return "Raise:"+ to_string(value_);
        case Fold: return "Fold";
        default: return "ERROR" ;
      }
    }
    inline int GetValue() const {
      return value_;
    }

    inline int GetType() const {
      return type_;
    }

   private:
    int value_;
    int type_;

  };

/**
 * GenericPokerState is a class that represents Generic Poker states,
 * which contains players' board - what they can see,
 * and who can play in the turn.
 */
  class GenericPokerState : public State {
   public:
    // Constructor
    GenericPokerState(const shared_ptr<Domain> &domain, int p1card, int p2card, optional<int> natureCard, double firstPlayerReward,
                      double pot, vector<int> players, int round, shared_ptr<GenericPokerAction> lastAction, int continuousRaiseCount);

    GenericPokerState(const shared_ptr<Domain> &domain, int p1card, int p2card, optional<int> natureCard,
                      unsigned int ante, vector<int> players);

    // Destructor
    ~GenericPokerState() override = default;

    // GetActions returns possible actions for a player in the state.
    vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;

    OutcomeDistribution
    performActions(const vector<pair<int, shared_ptr<Action>>> &actions) const override;


    inline vector<int> getPlayers() const final {
      return players_;
    }

    inline bool operator==(const State &rhs) const override {
      auto State = dynamic_cast<const GenericPokerState&>(rhs);

      return  player1Card_ == State.player1Card_&&
              player2Card_ == State.player2Card_ &&
              round_ == State.round_ &&
              pot == State.pot &&
              firstPlayerReward == State.firstPlayerReward &&
              domain == State.domain &&
              natureCard_ == State.natureCard_ &&
              players_ == State.players_ &&
              lastAction == State.lastAction;
    }

    inline size_t getHash() const override {
      size_t seed = 0;
      for (auto &i : players_) {
        boost::hash_combine(seed, i);
      }
      boost::hash_combine(seed, player1Card_);
      boost::hash_combine(seed, player2Card_);
      boost::hash_combine(seed, *natureCard_);
      boost::hash_combine(seed, round_);
      boost::hash_combine(seed, continuousRaiseCount_);
      boost::hash_combine(seed, pot);
      boost::hash_combine(seed, firstPlayerReward);
      boost::hash_combine(seed, domain);
      boost::hash_combine(seed, lastAction);
      return seed;
    }

    inline string toString() const override {
      return "Player 1 card: " + to_string(player1Card_) + "\nPlayer 2 card: " +
              to_string(player2Card_) + "\nNature card: " + to_string(natureCard_.value_or(-1)) +
              "\nPlayer on move: " + to_string(players_[0]) + "\nPot: " + to_string(pot) +
              "\nReward for first player: " + to_string(firstPlayerReward) +
              "\nLast action: " + lastAction->toString() + "\nRound: " + to_string(round_) +
              "Continuous raise count: " + to_string(continuousRaiseCount_) +  "\n";
    }

   protected:
    vector<int> players_;
    shared_ptr<GenericPokerAction> lastAction;
    optional<int> natureCard_;
    double pot;
    double firstPlayerReward;
    int player1Card_;
    int player2Card_;

    int round_;

    int continuousRaiseCount_;


  };


/**
 * GenericPokerDomain is a class that represents Generic Poker domain,
 * which contain static height and static width.
 */
  class GenericPokerDomain : public Domain {
   public:
    // constructor
    GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                                unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                                unsigned int maxDifferentRaises, unsigned int ante);

    GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes,
                       unsigned int maxRaisesInRow, unsigned int maxDifferentBets,
                       unsigned int maxDifferentRaises);

    GenericPokerDomain(unsigned int maxCardTypes, unsigned int maxCardsOfTypes);

    GenericPokerDomain();

    // destructor
    ~GenericPokerDomain() override = default;

    // GetInfo returns string containing domain information.
    string getInfo() const final;

    inline vector<int> getPlayers() const final {
      return {0,1};
    }

    vector<int> betsFirstRound;
    vector<int> raisesFirstRound;
    vector<int> betsSecondRound;
    vector<int> raisesSecondRound;
    const unsigned int maxCardTypes;  // cisla
    const unsigned int maxCardsOfEachType; // barvy
    const unsigned int maxRaisesInRow;
    int maxUtility;
    const unsigned int ante;
    const int TERMINAL_ROUND = 4;

  };


}
#endif //GTLIB2_GENERICPOKER_H
