//
// Created by Pavel Rytir on 1/20/18.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_BASE_OLD_H
#define PURSUIT_BASE_OLD_H


#include "base.h"
#include <unordered_map>

using std::unordered_map;


namespace GTLib2 {

  [[deprecated]]
  static Action NoA(-1);  // No Action


  class [[deprecated]] OldOutcome {
    // Following obsolete

    [[deprecated]]
    OldOutcome(shared_ptr<State> s, vector<shared_ptr<Observation>> ob,
    vector<double> rew);

    // GetState returns a new state
    [[deprecated]]
    inline const shared_ptr<State> &GetState() const {
      return state;
    }

    // GetObs returns vector of observations
    [[deprecated]]
    inline const vector<shared_ptr<Observation>> &GetObs() {
      return ob_;
    }

    // GetReward returns vector of rewards for each player.
    [[deprecated]]
    inline const vector<double> &GetReward() const {
      return rew_;
    }

   private:

    [[deprecated]]
    vector<shared_ptr<Observation>> ob_;
    [[deprecated]]
    vector<double> rew_;
  };

  class [[deprecated]] OutcomeDistributionOld {
   public:
    // constructor

    OutcomeDistributionOld();

    explicit OutcomeDistributionOld(vector<pair<Outcome, double>> pairs);

    // GetOutcomes returns a vector of all outcomes.
    [[deprecated]]
    vector<Outcome> GetOutcomes();

    // GetProb returns vector of probabilities over outcomes.
    [[deprecated]]
    vector<double> GetProb();


    vector<pair<Outcome, double>> distribution;
  };


  class [[deprecated]] OldState {
    // Following methods are obsolete
    // OldPerformAction performs actions for all players who can play in the state.
    [[deprecated]]
    virtual OutcomeDistributionOld PerformAction(const vector<shared_ptr<Action>> &actions);


    // OldGetPlayers returns who can play in this state.
    [[deprecated]]
    virtual const vector<bool> &OldGetPlayers() const;

  };

  class [[deprecated]] OldDomain {
    [[deprecated]]
    inline shared_ptr<OutcomeDistributionOld> getRootStateDistributionPtr() const {
      return rootStatesDistributionPtr;
    }

   protected:
    [[deprecated]]
    shared_ptr<OutcomeDistributionOld> rootStatesDistributionPtr;
  };

  class [[deprecated]] Strategy {
    protected:
        unordered_map<shared_ptr<InformationSet>,
                vector<pair<double, shared_ptr<Action>>>> strategy;

    public:
        // constructor
        Strategy() {
            strategy = unordered_map<shared_ptr<InformationSet>,
                    vector<pair<double, shared_ptr<Action>>>>();
        }

        // inserts new element into unordered_map
        inline auto Add(const shared_ptr<InformationSet> &key,
                        const vector<pair<double, shared_ptr<Action>>> &value)
        -> decltype(strategy.emplace(key, value)) {
            strategy.emplace(key, value);
        }

        // returns element with  key equivalent to key
        inline auto Find(const shared_ptr<InformationSet> &key) const
        -> decltype(strategy.find(key)) {
            return strategy.find(key);
        }

        // returns end of unordered_map it is used with Find method.
        inline auto End() const -> decltype(strategy.end()) const {
            return strategy.end();
        }

        // returns all strategies
        inline virtual const unordered_map<shared_ptr<InformationSet>,
                vector<pair<double, shared_ptr<Action>>>> &GetStrategies() {
            return strategy;
        }
    };


    class [[deprecated]] PureStrategy : public Strategy {
    public:
        // constructor
        PureStrategy() : Strategy() {}

        // inserts new element into unordered_map
        inline auto Add(const shared_ptr<InformationSet> &key,
                        const shared_ptr<Action> &value)
        -> decltype(strategy.emplace(key, vector<pair<double, shared_ptr<Action>>>
                {std::make_pair(1, value)})) {
            strategy.emplace(key, vector<pair<double, shared_ptr<Action>>>
                    {std::make_pair(1, value)});
        }
    };

          // Start function to calculate an expected value for a strategy profile
        [[deprecated]]
        virtual double CalculateUtility(const vector<PureStrategy> &pure_strategies);

        // Calculate an expected value for a strategy profile
        [[deprecated]]
        virtual double ComputeUtility(State *state, unsigned int depth,
                                      unsigned int players,
                                      const vector<PureStrategy> &pure_strategies,
                                      const vector<vector<int>> &aoh);

    [[deprecated]]
    double OldBestResponse(int player, shared_ptr<vector<double>> strategies, int rows,
                           int cols, vector<double> utilities);

// Domain independent treewalk algorithm
    [[deprecated]]
    void OldTreewalk(shared_ptr<Domain> domain, State *state,
                     unsigned int depth, int players,
                     std::function<void(State *)> FunctionForState);

    [[deprecated]]
    void OldTreewalk(shared_ptr<Domain> domain, State *state,
                     unsigned int depth, int players);

// Start method for domain independent treewalk algorithm
    [[deprecated]]
    void OldTreewalkStart(const shared_ptr<Domain> &domain, unsigned int depth = 0);


    // Domain independent treewalk algorithm
  [[deprecated]]
  void Pursuit(const shared_ptr<Domain> &domain, State *state,
               unsigned int depth, int players);

// Start method for domain independent treewalk algorithm
  [[deprecated]]
  void PursuitStart(const shared_ptr<Domain> &domain, unsigned int depth = 0);

}

#endif //PURSUIT_BASE_OLD_H

#pragma clang diagnostic pop