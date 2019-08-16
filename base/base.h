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
#include "external/cereal/cereal.hpp"
#include "utils/utils.h"
#include "utils/logging.h"

namespace GTLib2 {

class Action;
class Observation;
class Outcome;
class State;
class Domain;
class InformationSet;
class AOH;
class Domain;

typedef uint8_t Player;
constexpr Player NO_PLAYER = 0xff;
inline Player opponent(Player pl) {
    assert(pl == 0 || pl == 1);
    return Player(1-pl);
}

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
struct PlayerAction {
    const Player player;
    const shared_ptr<Action> action;
};

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
 * Each domain can implement it's own Action subclass.
 */
class Action {
 public:
    inline explicit Action(ActionId id) : id_(id) {}
    explicit Action() : Action(NO_ACTION) {};
    virtual ~Action() = default;

    inline virtual string toString() const {
        if (id_ == NO_ACTION) return "NoA";
        return to_string(id_);
    }

    inline ActionId getId() const { return id_; };
    inline virtual HashType getHash() const { return id_; };
    inline virtual bool operator==(const Action &that) const { return id_ == that.id_; };
    inline friend std::ostream & operator<<(std::ostream &ss, const Action &a) {
        ss << a.toString();
        return ss;
    }

 protected:
    const ActionId id_ = NO_ACTION;
};


/**
 * Special value of observation id, indicating no observation has been made.
 */
constexpr ObservationId NO_OBSERVATION = 0xFFFFFFFF;

/**
 * Special value of observation id, indicating that player made a move (in simultaneos round)
 * 0x0xFFFFFF00 means player 0, 0xFFFFFF01 player 1, etc.
 *
 * Use function observationPlayerMoved for calculating the values.
 */
constexpr ObservationId OBSERVATION_PLAYER_MOVED = 0xFFFFFF00;

/**
 * Observation is an abstract class that represents observations, which are identified by their id.
 *
 * Different observations must have different ids, the same observations must have the same id.
 *
 * It's up to each domain to guarantee consistency of observation ids.
 */
class Observation {
 public:
    inline explicit Observation(ObservationId id) : id_(id) {}
    explicit Observation() : Observation(NO_OBSERVATION) {};
    virtual ~Observation() = default;

    inline virtual string toString() const {
        if (id_ == NO_OBSERVATION) return "NoOb";
        return to_string(id_);
    }

    inline ObservationId getId() const { return id_; };
    inline virtual const HashType getHash() const { return id_; };
    inline virtual bool operator==(const Observation &that) const { return id_ == that.id_; };
    inline friend std::ostream & operator<<(std::ostream &ss, const Observation &o) {
        ss << o.toString();
        return ss;
    }

 protected:
    // we do not set it const, as computation of it can be non-trivial
    // todo: maybe we should and provide helper function for computation?
    ObservationId id_ = NO_OBSERVATION;
};

inline ObservationId observationPlayerMoved(Player pl) {
    return OBSERVATION_PLAYER_MOVED+pl;
}


/**
 * Action observation puts together what observation was made with an action.
 */
struct ActionObservationIds {
    ActionId action;
    ObservationId observation;

    bool operator==(const ActionObservationIds &rhs) const;
    bool operator!=(const ActionObservationIds &rhs) const;
    const HashType getHash() const { return hashCombine(132456456, action, observation); }
    inline friend std::ostream & operator<<(std::ostream &ss, const ActionObservationIds &ids) {
        ss << (ids.action == NO_ACTION ? "NoA" : to_string(ids.action)) << " ";
        ss << (ids.observation == NO_OBSERVATION ? "NoOb" : to_string(ids.observation));
        return ss;
    }
};

static_assert(sizeof(ActionObservationIds) == 8, "Should fit within size_t (64 bit)");
static_assert(sizeof(ActionObservationIds) == sizeof(HashType), "Should have the same size");

const ActionObservationIds NO_ACTION_OBSERVATION{
    .action = NO_ACTION,
    .observation = NO_OBSERVATION
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
struct Outcome {
 public:
    Outcome(shared_ptr<State> _state,
            vector<shared_ptr<Observation>> privateObservations,
            shared_ptr<Observation> publicObservation,
            vector<double> rewards);

    const shared_ptr<State> state;
    const vector<shared_ptr<Observation>> privateObservations;
    const shared_ptr<Observation> publicObservation;
    const vector<double> rewards;
    const HashType hash;

    HashType getHash() const { return hash; };
    bool operator==(const Outcome &rhs) const;
};

/**
 * The probability of given outcome.
 */
struct OutcomeEntry {
    Outcome outcome;
    double prob;

    inline OutcomeEntry(Outcome outcome_) : outcome(move(outcome_)), prob(1.0) {}
    inline OutcomeEntry(Outcome outcome_, double prob_) : outcome(move(outcome_)), prob(prob_) {
        assert(prob > 0); // domain should not produce outcomes that have zero probability!
    }
};

/**
 * Probabilities of outcomes.
 */
typedef vector<OutcomeEntry> OutcomeDistribution;


/**
 * Abstract class that represents information sets.
 *
 * For the actual use in the code, refer to AOH class.
 */
class InformationSet {
 public:
    InformationSet() = default;
    virtual bool operator==(const InformationSet &rhs) const = 0;
    inline bool operator!=(const InformationSet &rhs) const { return !(rhs == *this); };
    virtual HashType getHash() const = 0;
    virtual string toString() const = 0;
    friend inline std::ostream &operator<<(std::ostream &ss, InformationSet &infoset) {
        ss << infoset.toString();
        return ss;
    }
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
    AOH(Player player, const vector<ActionObservationIds> &aoHistory);

    inline HashType getHash() const final { return hash_; }
    bool operator==(const InformationSet &rhs) const override;

    inline Player getPlayer() const { return player_; }
    inline ObservationId getInitialObservationId() const { return aoh_.front().observation; }
    inline vector<ActionObservationIds> getAOids() const { return aoh_; }
    string toString() const override;

 private:
    const Player player_;
    const vector<ActionObservationIds> aoh_;
    const HashType hash_;
};

struct InfosetAction {
    const shared_ptr<InformationSet> infoset;
    const shared_ptr<Action> action;
    InfosetAction(shared_ptr<InformationSet> infoset, shared_ptr<Action> action)
        : infoset(move(infoset)), action(move(action)) {}
    inline HashType getHash() const { return infoset->getHash() + action->getHash(); }
    inline bool operator==(const InfosetAction &rhs) const {
        return getHash() == rhs.getHash() && *infoset == *rhs.infoset && *action == *rhs.action;
    };
};

/**
 * List of actions that had to be taken at information sets to get to current EFG node.
 */
class ActionSequence {
 public:
    explicit ActionSequence(vector<InfosetAction> sequence)
        : sequence_(move(sequence)), hash_(hashCombine(2315468453135153, sequence_)) {}
    bool operator==(const ActionSequence &rhs) const;
    inline HashType getHash() const { return hash_; };
    const vector<InfosetAction> sequence_;
    const HashType hash_;
};

/**
 * Realization plan is a probability distribution over action sequences.
 */
typedef unordered_map<shared_ptr<ActionSequence>, double> RealizationPlan;

/**
 * State is an abstract class that represents domain states.
 *
 * Domain is modeled using a (possibly cyclic) graph whose nodes represent state of the domain.
 */
class State {
 public:
    explicit State(const Domain *domain, HashType hash);

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
     * Performs actions given by vector of  <player, action>. If actions for some players
     * are missing, they will be padded by NO_ACTION.
     */
    OutcomeDistribution performPartialActions(const vector<PlayerAction> &plActions) const;

    /**
     * Performs actions by all the players, indexed from 0.
     * Players that do not have an action will receive special NO_ACTION.
     */
    virtual OutcomeDistribution performActions(const vector<shared_ptr<Action>> &actions) const = 0;

    /**
     * Returns players that can play in this state
     */
    virtual vector<Player> getPlayers() const = 0;

    /**
     * Check if given player is making a move in this state
     */
    bool isPlayerMakingMove(Player pl) const;

    /**
     * Returns whether there is no more transition to any other state
     */
    virtual bool isTerminal() const = 0;


    /**
     * Returns state description
     */
    virtual string toString() const;

    HashType getHash() const { return hash_; };
    virtual bool operator==(const State &rhs) const = 0;
    inline const Domain *getDomain() const { return domain_; }

 protected:
    const Domain *domain_;
    const HashType hash_;
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
    Domain(unsigned int maxStateDepth, unsigned int numberOfPlayers, bool isZeroSum_,
           shared_ptr<Action> noAction, shared_ptr<Observation> noObservation);

    virtual ~Domain() = default;

    /**
     * Returns the roots of the trees of the domain forest
     * with the probability of each tree.
     */
    const OutcomeDistribution &getRootStatesDistribution() const;

    /**
     * Returns number of players in the game.
     */
    inline unsigned int getNumberOfPlayers() const { return numberOfPlayers_; }

    /**
     * Returns default maximal depth used in algorithms.
     *
     * Note that the "depth 0" corresponds to the root outcomes, so state after the root outcome
     * has a state depth of 1 (there has been already one transition in the graph)
     */
    inline unsigned int getMaxStateDepth() const { return maxStateDepth_; }

    /**
     * Are all the rewards in outcomes zero-sum?
     */
    inline bool isZeroSum() const { return isZeroSum_; };

    inline double getMaxUtility() const { return maxUtility_; }
    inline double getMinUtility() const { return -maxUtility_; }

    /**
     * Returns string containing detailed domain information.
     */
    virtual string getInfo() const = 0;

    inline const shared_ptr<Action> &getNoAction() const { return noAction_; }
    inline const shared_ptr<Observation> &getNoObservation() const { return noObservation_; }

 protected:
    OutcomeDistribution rootStatesDistribution_;
    unsigned int maxStateDepth_;
    const unsigned int numberOfPlayers_;
    const bool isZeroSum_;
    double maxUtility_;

    const shared_ptr<Action> noAction_;
    const shared_ptr<Observation> noObservation_;
};
}  // namespace GTLib2

MAKE_EQ(GTLib2::InformationSet)
MAKE_EQ(GTLib2::AOH)
MAKE_EQ(GTLib2::Action)
MAKE_EQ(GTLib2::Observation)
MAKE_EQ(GTLib2::State)
MAKE_EQ(GTLib2::Outcome)
MAKE_EQ(GTLib2::ActionSequence)

MAKE_HASHABLE(GTLib2::InformationSet)
MAKE_HASHABLE(GTLib2::AOH)
MAKE_HASHABLE(GTLib2::Action)
MAKE_HASHABLE(GTLib2::Observation)
MAKE_HASHABLE(GTLib2::State)
MAKE_HASHABLE(GTLib2::Outcome)
MAKE_HASHABLE(GTLib2::ActionSequence)

namespace std { // NOLINT(cert-dcl58-cpp)

template<typename T>
std::ostream &operator<<(std::ostream &ss, const vector<T> &arr) {
    ss << "[";
    for (int i = 0; i < arr.size(); ++i) {
        if (i == 0) ss << arr.at(i);
        else ss << ", " << arr.at(i);
    }
    ss << "]";
    return ss;
}

template<typename K, typename V>
std::ostream &operator<<(std::ostream &ss, const unordered_map<K, V> &map) {
    bool addNewLine = map.size() > 4;
    ss << "{";
    if(addNewLine) ss << endl;
    bool first=true;
    for (const auto &[k,v] : map) {
        if(!first && !addNewLine) ss << ", ";
        ss << k << ": " << v;
        first=false;
        if(addNewLine) ss << endl;
    }
    ss << "}";
    return ss;
}

template<typename K, typename V>
std::ostream &operator<<(std::ostream &ss, const unordered_map<shared_ptr<K>, V> &map) {
    bool addNewLine = map.size() > 4;
    ss << "{";
    if(addNewLine) ss << endl;
    bool first=true;
    for (const auto &[k,v] : map) {
        if(!first && !addNewLine) ss << ", ";
        ss << *k << ": " << v;
        first=false;
        if(addNewLine) ss << endl;
    }
    ss << "}";
    return ss;
}

}  // namespace std


#endif  // BASE_BASE_H_

#pragma clang diagnostic pop
