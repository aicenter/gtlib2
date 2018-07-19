//
// Created by Jakub Rozlivek on 07.08.2017.
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
using std::make_unique;
using std::pair;
using std::tuple;


namespace GTLib2 {

/**
 * Action is an abstract class that represents actions,
 * which are identified by their id. Different actions have to have different ids, the same actions have to have
 * the same id.
 */
    class Action {
    public:
        explicit Action(int id);

        virtual ~Action() = default;

        virtual string toString() const;

        int getId() const;

        virtual bool operator==(const Action &that) const;

        virtual size_t getHash() const;

    protected:
        int id;
    };


    typedef vector<shared_ptr<Action>> ActionSequence;

    typedef unordered_map<shared_ptr<ActionSequence>, double> RealizationPlan;


/**
 * Observation is an abstract class that represents observations,
 * which are identified by their id. Different observations have to have different ids, the same observations have to have
 * the same id.
 */
    class Observation {
    public:
        explicit Observation(int id);

        virtual ~Observation() = default;

        // Returns description
        virtual string toString() const;

        // Returns observation id
        int getId() const;

        virtual bool operator==(const Observation &rhs) const;

        virtual size_t getHash() const;

    protected:
        int id;
    };


    class State;

/**
 * Outcome is a class that represents outcomes,
 * which contain rewards for each player, observations for each player and a new state.
 */
    class Outcome {
    public:

        Outcome(shared_ptr<State> s, unordered_map<int, shared_ptr<Observation>> observations,
                unordered_map<int, double> rewards);

        shared_ptr<State> state;
        unordered_map<int, shared_ptr<Observation>> observations;
        unordered_map<int, double> rewards;


        size_t getHash() const;

        bool operator==(const Outcome &rhs) const;
    };


    typedef pair<Outcome,double> distributionEntry;
    typedef vector<distributionEntry> OutcomeDistribution;




/**
 * InfSet is an abstract class that represent information sets,
 *  which are identified by their hash code. TODO: Maybe rather identified by some unique id?
 */
    class InformationSet {
    public:
        InformationSet() = default;

        virtual bool operator==(const InformationSet &rhs) const = 0;

        // GetHash returns hash code.
        virtual size_t getHash() const = 0;
    };


/**
 * AOH is a class that represents action-observation history,
 * which contains player id and vector of action-observation history,
 * hash code for faster comparing and has overloaded operator '=='.
 */
    class AOH : public InformationSet {
    public:

        AOH(int player, int initialObservation, const vector<pair<int, int>> &aoHistory);

        int getNumberOfActions() const;

        // GetHash returns hash code.
        size_t getHash() const final;

        // Overloaded for comparing two AOHs
        bool operator==(const InformationSet &rhs) const override;

        [[deprecated]]
        AOH(int player, const vector<int> &hist);

        int getPlayer() const;
        int getInitialObservationId() const;
        vector<pair<int, int>> getAOHistory() const;

    private:
        size_t computeHash() const;

        int player;
        int initialObservationId;
        vector<pair<int, int>> aoh; // Vector of pairs. First coordinate is action id, the second is observation id.
        size_t hashValue;
    };







/**
 * State is an abstract class that represent states
 */
    class State {
    public:
        State();

        virtual ~State() = default;

        // Returns possible actions for a player in the state.
        virtual vector<shared_ptr<Action>> getAvailableActionsFor(int player) const = 0;

        // Performs actions given by player->action map
        virtual OutcomeDistribution performActions(const unordered_map<int, shared_ptr<Action>> &actions) const = 0;

        // Gets players that can play in this state

        virtual vector<int> getPlayers() const = 0;

        // GetNumPlayers returns number of players who can play in this state.
        virtual int getNumberOfPlayers() const;

        virtual string toString() const;

        virtual bool operator==(const State &rhs) const;

        virtual size_t getHash() const;

        // ToString returns state description
        virtual string toString(int player) const;

        // AddString adds string s to a string in vector of strings.
        [[deprecated]]
        virtual void AddString(const string &s, int player);


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
            for (const auto &action : (*seq)) {
                boost::hash_combine(seed, action);
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


    template<>
    struct hash<shared_ptr<Observation>> {
        size_t operator()(const shared_ptr<Observation> &p) const {
            return p->getHash();
        }
    };

    template<>
    struct equal_to<shared_ptr<Observation>> {
        bool operator()(const shared_ptr<Observation> &a,
                        const shared_ptr<Observation> &b) const {
            return *a == *b;
        }
    };


    template<>
    struct hash<shared_ptr<State>> {
        size_t operator()(const shared_ptr<State> &p) const {
            return p->getHash();
        }
    };

    template<>
    struct equal_to<shared_ptr<State>> {
        bool operator()(const shared_ptr<State> &a,
                        const shared_ptr<State> &b) const {
            return *a == *b;
        }
    };

    template<>
    struct hash<Outcome> {
        size_t operator()(const Outcome &p) const {
            return p.getHash();
        }
    };

    template<>
    struct equal_to<Outcome> {
        bool operator()(const Outcome &a,
                        const Outcome &b) const {
            return a == b;
        }
    };
} // namespace std


namespace GTLib2 {


    typedef unordered_map<shared_ptr<InformationSet>, unordered_map<shared_ptr<Action>, double>> BehavioralStrategy;


    /**
 * Domain is an abstract class that represent domain,
 * contains a probability distribution over root states.
 */
    class Domain {
    public:
        // constructor
        Domain(int maxDepth, int numberOfPlayers);

        // destructor
        virtual ~Domain() = default;

        // Returns distributions of the root states.
        OutcomeDistribution getRootStatesDistribution() const;

        virtual vector<int> getPlayers() const = 0;

        // Returns number of players in the game.
        inline int getNumberOfPlayers() const {
            return numberOfPlayers;
        }

        // Returns default maximal depth used in algorithms.
        inline int getMaxDepth() const {
            return maxDepth;
        }

        // GetInfo returns string containing domain information.
        virtual string getInfo() const = 0;


    protected:
        int maxDepth;
        int numberOfPlayers;
        OutcomeDistribution rootStatesDistribution;

    };




}

#endif  // BASE_H_

#pragma clang diagnostic pop