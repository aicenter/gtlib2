//
// Created by rozliv on 07.08.2017.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef BASE_H_
#define BASE_H_


#include <functional>
#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <ctime>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "../utils/utils.h"
#include <tuple>
#include <boost/functional/hash.hpp>
//#include "efg.h"

using std::string;
using std::cout;
using std::vector;
using std::move;
using std::unique_ptr;
using std::shared_ptr;
using std::to_string;
using std::unordered_map;
using std::unordered_set;
using std::make_shared;
using std::pair;
using std::tuple;



namespace GTLib2 {

/**
 * Action is an abstract class that represents actions,
 * which are identified by their id.
 */
    class Action {
    public:
        explicit Action(int id);

        virtual ~Action() = default;

        virtual string toString() const;

        int getId() const;

        virtual bool operator==(Action &that);

        virtual size_t getHash() const;

    protected:
        int id;
    };


    typedef vector<shared_ptr<Action>> ActionSequence;

    typedef unordered_map<shared_ptr<ActionSequence>, double> RealizationPlan;


/**
 * Observation is an abstract class that represents observations,
 * which are identified by their id.
 */
    class Observation {
    public:
        explicit Observation(int id);

        virtual ~Observation() = default;

        // Returns description
        virtual string toString() const;

        // Returns observation id
        int getId() const;

    protected:
        int id_;
    };


    class State;

/**
 * Outcome is a class that represents outcomes,
 * which contain rewards, observations and a new state.
 */
    class Outcome {
    public:

        Outcome(shared_ptr<State> s, unordered_map<int, shared_ptr<Observation>> observations,
                unordered_map<int, double> rewards);

        shared_ptr<State> state;
        unordered_map<int, shared_ptr<Observation>> observations;
        unordered_map<int, double> rewards;




        // Following obsolete

        [[deprecated]]
        Outcome(shared_ptr<State> s, vector<shared_ptr<Observation>> ob,
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

/**
 * ProbDistribution is a class that represent
 * pairs of outcomes and their probability.
 */
    class ProbDistribution {
    public:
        // constructor

        ProbDistribution();

        explicit ProbDistribution(vector<pair<Outcome, double>> pairs);

        // GetOutcomes returns a vector of all outcomes.
        [[deprecated]]
        vector<Outcome> GetOutcomes();

        // GetProb returns vector of probabilities over outcomes.
        [[deprecated]]
        vector<double> GetProb();


        vector<pair<Outcome, double>> distribution;
    };


/**
 * InfSet is an abstract class that represent information sets,
 *  which are identified by their hash code.
 */
    class InformationSet {
    public:
        InformationSet() = default;

        virtual bool operator==(InformationSet &other) = 0;

        // GetHash returns hash code.
        virtual size_t getHash() = 0;
    };


/**
 * AOH is a class that represents action-observation history,
 * which contains player id and vector of action-observation history,
 * hash code for faster comparing and has overloaded operator '=='.
 */
    class AOH : public InformationSet {
    public:

        AOH(int player, int initialObservation, const vector<tuple<int, int>> &aoHistory);

        // GetHash returns hash code.
        size_t getHash() final;

        // Overloaded for comparing two AOHs
        bool operator==(InformationSet &other2) override;

        [[deprecated]]
        AOH(int player, const vector<int> &hist);

    private:
        int player_;
        int initialObservationId;
        vector<tuple<int, int>> aoh; // Vector of pairs. First coordinate is action id, the second is observation id.
        // vector<int> aohistory_; // TODO: remove
        size_t seed_ = 0;
    };


}

namespace std {
    using namespace GTLib2;
    template<>
    struct hash<shared_ptr<InformationSet>> {
        size_t operator()(shared_ptr<InformationSet> const &p) const {
            return p->getHash();
        }
    };

    template<>
    struct equal_to<shared_ptr<InformationSet>> {
        bool operator()(shared_ptr<InformationSet> const &a,
                        shared_ptr<InformationSet> const &b) const {
            return *a == *b;
        }
    };

    template<>
    struct hash<shared_ptr<AOH>> {
        size_t operator()(shared_ptr<AOH> const &p) const {
            return p->getHash();
        }
    };

    template<>
    struct equal_to<shared_ptr<AOH>> {
        bool operator()(shared_ptr<AOH> const &a,
                        shared_ptr<AOH> const &b) const {
            return *a == *b;
        }
    };

    template<>
    struct hash<shared_ptr<ActionSequence>> {
        size_t operator()(const shared_ptr<ActionSequence> &seq) const {
            size_t seed = 0;
            for (auto it = (*seq).begin(); it != (*seq).end(); ++it) {
                boost::hash_combine(seed, *it);
            }
            return seed;
        }
    };

    template<>
    struct equal_to<shared_ptr<ActionSequence>> {
        bool operator()(const shared_ptr<ActionSequence> &a,
                        const shared_ptr<ActionSequence> &b) const {
            hash<shared_ptr<ActionSequence>> hasher;
            if (hasher(a) != hasher(b) || (*a).size() != (*b).size()) {
                return false;
            }
            for (int i = 0; i <= (*a).size(); i++) {
                if ((*a)[i] != (*b)[i]) {
                    return false;
                }
            }
            return true;
        }
    };

    template<>
    struct hash<shared_ptr<Action>> {
        size_t operator()(const shared_ptr<Action> &p) const {
            return p->getHash();
        }
    };

    template<>
    struct equal_to<shared_ptr<Action>> {
        bool operator()(const shared_ptr<Action> &a,
                        const shared_ptr<Action> &b) const {
            return *a == *b;
        }
    };

}  // namespace std

namespace GTLib2 {

    typedef unordered_map<shared_ptr<InformationSet>, unordered_map<shared_ptr<Action>, double>> BehavioralStrategy;

/**
 * Strategy is a class that represents a player's (behavioral) strategy.
 * Contains unordered_map with shared pointer to InfSet used as key and
 * probability distribution over actions as values. Prob. distribution is
 * represented by vector of pairs of double (0-1) and shared pointer to Action.
 */


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


/**
 * PureStrategy is a class that represents a player's pure strategy.
 * Contains unordered_map with shared pointer to InfSet used as key and
 * probability distribution over actions as values. Prob. distribution is
 * represented by a pair of double (1) and shared pointer to Action.
 */
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


/**
 * State is an abstract class that represent states
 */
    class State {
    public:
        // constructor
        State();

        // destructor
        virtual ~State() = default;

        // Returns possible actions for a player in the state.
        virtual vector<shared_ptr<Action>> getAvailableActionsFor(int player) const = 0;

        // Performs actions given by player->action map
        virtual ProbDistribution performActions(const unordered_map<int, shared_ptr<Action>> &actions) const = 0;

// Gets players that can play as an unordered_set of int

        virtual unordered_set<int> getPlayersSet() const;

        // GetNumPlayers returns number of players who can play in this state.
        virtual int getNumberOfPlayers() const = 0;

        // ToString returns state description
        virtual string toString(int player) = 0;

// Following methods are obsolete
        // GetActions returns possible actions for a player in the state.
        [[deprecated]]
        virtual void GetActions(vector<shared_ptr<Action>> &list, int player) const;

        // PerformAction performs actions for all players who can play in the state.
        [[deprecated]]
        virtual ProbDistribution PerformAction(const vector<shared_ptr<Action>> &actions);


        // GetPlayers returns who can play in this state.
        [[deprecated]]
        virtual const vector<bool> &GetPlayers() const;


        // AddString adds string s to a string in vector of strings.
        [[deprecated]]
        virtual void AddString(const string &s, int player);


    };


    typedef shared_ptr<Action> ActionPtr;

/**
 * Domain is an abstract class that represent domain,
 * contains a probability distribution over root states.
 */
    class Domain {
    public:
        // constructor
        Domain(int maxDepth,int numberOfPlayers);

        // destructor
        virtual ~Domain() = default;

        // Returns distributions of the root states.
        inline shared_ptr<ProbDistribution> getRootStateDistributionPtr() const {
            return rootStatesDistributionPtr;
        }


        virtual vector<int> getPlayers() const;

        // Returns number of players in the game.
        inline int getNumberOfPlayers() const {
            return numberOfPlayers;
        }

        // Returns default maximal depth used in algorithms.
        inline int getMaxDepth() const {
            return maxDepth;
        }

        // GetInfo returns string containing domain information.
        virtual string GetInfo() = 0;

        // Start function to calculate an expected value for a strategy profile
        [[deprecated]]
        virtual double CalculateUtility(const vector<PureStrategy> &pure_strategies);

        // Calculate an expected value for a strategy profile
        [[deprecated]]
        virtual double ComputeUtility(State *state, unsigned int depth,
                                      unsigned int players,
                                      const vector<PureStrategy> &pure_strategies,
                                      const vector<vector<int>> &aoh);


    protected:
        int maxDepth;
        int numberOfPlayers;
        ProbDistribution rootStatesDistribution;
        shared_ptr<ProbDistribution> rootStatesDistributionPtr; // TODO: remove shared_ptr
    };

// The rest of the file is obsolete - does not work.


    [[deprecated]]
    static Action NoA(-1);  // No Action
    [[deprecated]]
    static Observation NoOb(-1);  // No Observation

}




#endif  // BASE_H_

#pragma clang diagnostic pop