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


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef BASE_BASE_H_
#define BASE_BASE_H_

#include "base/includes.h"
#include "base/hashing.h"
#include "utils/utils.h"

namespace GTLib2 {

// Maximum number of players we consider at any game
#define GAME_MAX_PLAYERS 2

class Action;
class Observation;
class Outcome;
class State;
class Domain;
class InformationSet;
class AOH;
class Domain;

typedef uint8_t Player;

/**
 * For a given State/EFGNode/InformationSet, IDs should be indexed
 * from 0 to N-1, where N is the number of available actions.
 */
typedef uint32_t ActionId;

/**
 * Unlike ActionId, the values of ObservationId *do not* need to be indexed from 0 to N-1.
 */
typedef uint32_t ObservationId;

/**
 * Specify action that given player played.
 */
// todo: refactor pair into a more readable struct (avoid using .first/.second)
typedef pair<Player, shared_ptr<Action>> PlayerAction;

/**
 * The probability of given outcome.
 */
// todo: refactor pair into a more readable struct (avoid using .first/.second)
typedef pair<Outcome, double> distributionEntry;

/**
 * Probabilities of outcomes.
 */
typedef vector<distributionEntry> OutcomeDistribution;

/**
 * Specify weight d[i] for each event i in vector, such that p[i] := d[i] / sum_j d[j]
 * I.e. it is "probability" distribution that does not sum up to 1.
 */
typedef vector<double> Distribution;

/**
 * Specify probability for each event in vector.
 * This must sum up to 1.
 */
typedef vector<double> ProbDistribution;

/**
 * Specify probability for actions in map.
 * The probabilities of listed actions must sum up to 1.
 * There might be some actions missing -- it is assumed they have probability 0.
 */
typedef unordered_map<shared_ptr<Action>, double> ActionProbDistribution;

/**
 * Behavioral strategy at given infoset: with what probability given action should be played?
 *
 * This should be **per player only**.
 */
typedef unordered_map<shared_ptr<InformationSet>, ActionProbDistribution> BehavioralStrategy;

/**
 * Specify behavioral strategy for each player.
 */
typedef vector<BehavioralStrategy> StrategyProfile;

/**
 * List of actions that had to be taken at information sets to get to current EFG node.
 */
// todo: refactor pair into a more readable struct (avoid using .first/.second)
typedef vector<pair<shared_ptr<InformationSet>, shared_ptr<Action>>> ActionSequence;

/**
 * Realization plan is a probability distribution over action sequences.
 */
typedef unordered_map<shared_ptr<ActionSequence>, double> RealizationPlan;

/**
 * Action observation puts together what observation was made with an action.
 */
// todo: refactor pair into a more readable struct (avoid using .first/.second)
typedef pair<ActionId, ObservationId> ActionObservation;



/**
 * Special value of action id, indicating no action has been taken.
 *
 * It is useful for example in phantom games.
 */
constexpr ActionId NO_ACTION = 0xFFFFFFFF;


/**
 * Action is an abstract class that represents actions, which are identified by their id.
 *
 * Different actions must have different ids, the same actions must have the same id.
 *
 * Each domain must implement it's own Action subclass.
 */
class Action {
 public:
    explicit Action(ActionId id);
    explicit Action() : Action(NO_ACTION) {};

    virtual ~Action() = default;

    virtual string toString() const;

    ActionId getId() const;

    virtual bool operator==(const Action &that) const;

    virtual size_t getHash() const;

 protected:
    ActionId id_;
};


/**
 * Special value of observation id, indicating no observation has been made.
 */
constexpr ObservationId NO_OBSERVATION = 0xFFFFFFFF;

/**
 * Observation is an abstract class that represents observations, which are identified by their id.
 *
 * Different observations must have different ids, the same observations must have the same id.
 *
 * It's up to each domain to guarantee consistency of observation ids.
 */
class Observation {
 public:
    explicit Observation(ObservationId id);
    explicit Observation() : Observation(NO_OBSERVATION) {};

    virtual ~Observation() = default;

    virtual string toString() const;

    ObservationId getId() const;

    virtual bool operator==(const Observation &rhs) const;

    virtual size_t getHash() const;

 protected:
    ObservationId id_;
};


/**
 * Outcome is a class that represents outcomes, or edges of the domain graph.
 *
 * They contain:
 * - a new state,
 * - observations for each player,
 * - public observation for all players,
 * - rewards for each player.
*/
class Outcome {
 public:
    Outcome(shared_ptr<State> s,
            vector<shared_ptr<Observation>> observations,
            shared_ptr<Observation> publicObservation,
            vector<double> rewards);

    shared_ptr<State> state_;
    vector<shared_ptr<Observation>> privateObservations_;
    shared_ptr<Observation> publicObservation_;
    vector<double> rewards_;

    size_t getHash() const;

    bool operator==(const Outcome &rhs) const;
};

/**
 * Abstract class that represents information sets.
 *
 * For the actual use in the code, refer to AOH class.
 */
class InformationSet {
 public:
    InformationSet() = default;

    virtual bool operator==(const InformationSet &rhs) const = 0;

    virtual size_t getHash() const = 0;

    virtual string toString() const = 0;
};

/**
 * AOH is a class that represents action-observation history,
 * a variant of how infosets can be implemented if observations
 * are part of the domain description.
 *
 * It contains
 * - player id, and
 * - vector of action-observation history.
 *
 * Implements hash code for faster comparing and has special overloaded operator '=='.
 */
class AOH: public InformationSet {
 public:
    AOH(Player player, const vector<ActionObservation> &aoHistory);

    inline unsigned long getSize() const {
        return aoh_.size();
    }

    inline size_t getHash() const final {
        return hashValue_;
    }

    // Overloaded for comparing two AOHs
    bool operator==(const InformationSet &rhs) const override;

    inline Player getPlayer() const {
        return player_;
    }
    inline ObservationId getInitialObservationId() const {
        return aoh_.front().second;
    }
    inline vector<ActionObservation> getAOHistory() const {
        return aoh_;
    }

    string toString() const override;

 private:
    size_t computeHash() const;
    vector<ActionObservation> aoh_;
    size_t hashValue_;
    Player player_;
};

/**
 * State is an abstract class that represents domain states.
 *
 * Domain is modeled using a (possibly cyclic) graph whose nodes represent state of the domain.
 *
 *
 *
 */
class State {
 public:
    explicit State(Domain *domain);

    virtual ~State() = default;

    /**
     * Returns number of possible actions for a player in the state.
     */
    virtual unsigned long countAvailableActionsFor(Player player) const = 0;

    /**
     * Returns possible actions for a player in the state.
     */
    virtual vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const = 0;

    /**
     * Performs actions given by vector of  <player, action> and return
     * new outcome distribution.
     */
    virtual OutcomeDistribution performActions(const vector<PlayerAction> &actions) const = 0;

    /**
     * Returns players that can play in this state
     */
    virtual vector<Player> getPlayers() const = 0;

    /**
     * Returns number of players who can play in this state.
     */
    virtual int getNumberOfPlayers() const;

    /**
     * Returns state description
     */
    virtual string toString() const;

    virtual bool operator==(const State &rhs) const = 0;

    virtual size_t getHash() const = 0;

    inline Domain *getDomain() const {
        return domain_;
    }

 protected:
    Domain *domain_;
};

/**
 * Domain is an abstract class that each domain must override.
 *
 * It contains a probability distribution over root states,
 * as starting point for other algorithms.
 *
 * Domain is modeled as a (possibly cyclic) graph whose nodes represent state of the game.
 * The edges or transitions from state to state happen only when the actual domain state
 * has changed -- players might play actions, which do not change the state!
 *
 * Additionally, chance is encoded by *stochastic* transitions, i.e. the outcome
 * of player's actions (in general) is not deterministic.
 */
class Domain {
 public:
    Domain(unsigned int maxDepth, unsigned int numberOfPlayers);

    virtual ~Domain() = default;

    /**
     * Returns the roots of the trees of the domain forest
     * with the probability of each tree.
     */
    const OutcomeDistribution &getRootStatesDistribution() const;

    virtual vector<Player> getPlayers() const = 0;

    /**
     * Returns number of players in the game.
     */
    inline unsigned int getNumberOfPlayers() const {
        return numberOfPlayers_;
    }

    /**
     * Returns default maximal depth used in algorithms.
     */
    inline unsigned int getMaxDepth() const {
        return maxDepth_;
    }

    inline double getMaxUtility() const {
        return maxUtility_;
    }

    inline double getMinUtility() const {
        return -maxUtility_;
    }

    /**
     * Returns string containing detailed domain information.
     */
    virtual string getInfo() const = 0;

 protected:
    OutcomeDistribution rootStatesDistribution_;
    unsigned int maxDepth_;
    unsigned int numberOfPlayers_;
    double maxUtility_;
};
}  // namespace GTLib2

namespace std { // NOLINT(cert-dcl58-cpp)

template<>
struct hash<shared_ptr<GTLib2::InformationSet>> {
    size_t operator()(shared_ptr<GTLib2::InformationSet> const &p) const {
        return p->getHash();
    }
};

template<>
struct equal_to<shared_ptr<GTLib2::InformationSet>> {
    bool operator()(shared_ptr<GTLib2::InformationSet> const &a,
                    shared_ptr<GTLib2::InformationSet> const &b) const {
        return *a == *b;
    }
};

template<>
struct hash<GTLib2::InformationSet *> {
    size_t operator()(GTLib2::InformationSet const *p) const {
        return p->getHash();
    }
};

template<>
struct equal_to<GTLib2::InformationSet *> {
    bool operator()(GTLib2::InformationSet *a,
                    GTLib2::InformationSet *b) const {
        return *a == *b;
    }
};

template<>
struct hash<shared_ptr<GTLib2::AOH>> {
    size_t operator()(shared_ptr<GTLib2::AOH> const &p) const {
        return p->getHash();
    }
};

template<>
struct equal_to<shared_ptr<GTLib2::AOH>> {
    bool operator()(shared_ptr<GTLib2::AOH> const &a,
                    shared_ptr<GTLib2::AOH> const &b) const {
        return *a == *b;
    }
};

template<>
struct hash<shared_ptr<GTLib2::Action>> {
    size_t operator()(const shared_ptr<GTLib2::Action> &p) const {
        return p->getHash();
    }
};

template<>
struct equal_to<shared_ptr<GTLib2::Action>> {
    bool operator()(const shared_ptr<GTLib2::Action> &a,
                    const shared_ptr<GTLib2::Action> &b) const {
        return *a == *b;
    }
};

template<>
struct hash<shared_ptr<GTLib2::ActionSequence>> {
    size_t operator()(const shared_ptr<GTLib2::ActionSequence> &seq) const {
        size_t seed = 0;
        for (const auto &action : (*seq)) {
            boost::hash_combine(seed, action.first);
            boost::hash_combine(seed, action.second);
        }
        return seed;
    }
};

template<>
struct equal_to<shared_ptr<GTLib2::ActionSequence>> {
    bool operator()(const shared_ptr<GTLib2::ActionSequence> &a,
                    const shared_ptr<GTLib2::ActionSequence> &b) const {
        hash<shared_ptr<GTLib2::ActionSequence>> hasher;
        if (hasher(a) != hasher(b) || a->size() != b->size()) {
            return false;
        }
        for (int i = 0; i <= a->size(); i++) {
            if ((*a)[i] != (*b)[i]) {
                return false;
            }
        }
        return true;
    }
};

template<>
struct hash<GTLib2::ActionSequence> {
    size_t operator()(const GTLib2::ActionSequence &seq) const {
        size_t seed = 0;
        hash<shared_ptr<GTLib2::InformationSet>> hasher;
        hash<shared_ptr<GTLib2::Action>> hasher2;
        for (const auto &action : seq) {
            boost::hash_combine(seed, hasher(action.first));
            boost::hash_combine(seed, hasher2(action.second));
        }
        return seed;
    }
};

template<>
struct equal_to<GTLib2::ActionSequence> {
    bool operator()(const GTLib2::ActionSequence &a,
                    const GTLib2::ActionSequence &b) const {
        hash<GTLib2::ActionSequence> hasher;
        if (hasher(a) != hasher(b) || a.size() != b.size()) {
            return false;
        }
        for (int i = 0; i < a.size(); i++) {
            if (!a[i].first->operator==(*b[i].first.get()) ||
                !a[i].second->operator==(*b[i].second.get())) {
                return false;
            }
        }
        return true;
    }
};

template<>
struct hash<shared_ptr<GTLib2::Observation>> {
    size_t operator()(const shared_ptr<GTLib2::Observation> &p) const {
        return p->getHash();
    }
};

template<>
struct equal_to<shared_ptr<GTLib2::Observation>> {
    bool operator()(const shared_ptr<GTLib2::Observation> &a,
                    const shared_ptr<GTLib2::Observation> &b) const {
        return *a == *b;
    }
};

template<>
struct hash<shared_ptr<GTLib2::State>> {
    size_t operator()(const shared_ptr<GTLib2::State> &p) const {
        return p->getHash();
    }
};

template<>
struct equal_to<shared_ptr<GTLib2::State>> {
    bool operator()(const shared_ptr<GTLib2::State> &a,
                    const shared_ptr<GTLib2::State> &b) const {
        return *a == *b;
    }
};

template<>
struct hash<GTLib2::Outcome> {
    size_t operator()(const GTLib2::Outcome &p) const {
        return p.getHash();
    }
};

template<>
struct equal_to<GTLib2::Outcome> {
    bool operator()(const GTLib2::Outcome &a,
                    const GTLib2::Outcome &b) const {
        return a == b;
    }
};

template<
    typename T,
    typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
>
std::ostream &
operator<<(std::ostream &ss, vector<T> arr) {
    ss << "[";
    for (int i = 0; i < arr.size(); ++i) {
        if(i == 0) ss << arr[i];
        else ss << ", " << arr[i];
    }
    ss << "]";
    return ss;
}

}  // namespace std


#endif  // BASE_BASE_H_

#pragma clang diagnostic pop
