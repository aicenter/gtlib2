//
// Created by rozliv on 14.08.2017.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef EFG_H_
#define EFG_H_

#include <unordered_map>
#include <experimental/optional>
#include "base.h"

using std::unordered_map;
using std::experimental::nullopt;
using std::experimental::optional;



namespace GTLib2 {

    class EFGNode;

    typedef pair<shared_ptr<EFGNode>,double> EFGDistEntry;
    typedef vector<EFGDistEntry> EFGNodesDistribution;

/**
 * EFGNode is a class that represents node in an extensive form game,
 * which contains action-observation history, state,
 * rewards (utility) and Information set.
 */
    class EFGNode final : public std::enable_shared_from_this<EFGNode const> {
    public:

        // Constructor for the same round node
        EFGNode(shared_ptr<EFGNode const> parent, const unordered_map<int, shared_ptr<Action>> &performedActions,
                int lastPlayer, shared_ptr<Action> incomingAction);

        // Constructor for the new round node
        EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                const unordered_map<int, shared_ptr<Observation>> &observations,
                const unordered_map<int, double> &rewards,
                const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                double natureProbability, shared_ptr<Action> incomingAction);

        // Constructor for the new round node
        EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                const unordered_map<int, shared_ptr<Observation>> &observations,
                const unordered_map<int, double> &rewards,
                const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                double natureProbability, shared_ptr<Action> incomingAction,
                const unordered_map<int, shared_ptr<Observation>> &initialObservations);


        // Returns the sequence of actions performed by the player since the root.
        ActionSequence getActionsSeqOfPlayer(int player) const;

        double getProbabilityOfActionsSeqOfPlayer(int player, const BehavioralStrategy &strat) const;

        // Returns available actions for the current player
        vector<shared_ptr<Action>> availableActions() const;

        // Perform the given action and returns the next node or nodes in case of stochastic games together with the probabilities.
        EFGNodesDistribution performAction(shared_ptr<Action> action) const;

        // Gets the information set of the node represented as ActionObservationHistory set.
        shared_ptr<AOH> getAOHInfSet() const;

        // Check if the node is in the given information set.
        bool isContainedInInformationSet(const shared_ptr<AOH> &infSet) const;

        // Gets the parent efg node.
        shared_ptr<EFGNode const> getParent() const;

        // Gets action that was performed at parent node and the result led to this node.
        shared_ptr<Action> getIncomingAction() const;

        // Returns the game state of that is represented by EFG node. Note that in simultaneous games one state corresponds to
        // mutliple efg nodes.
        shared_ptr<State> getState() const;

        string toString() const; // TODO: toString()

        size_t getHash() const;

        bool operator==(const EFGNode &rhs) const;

        int getDistanceFromRoot() const;

        int getLastObservationIdOfCurrentPlayer() const;


        optional<int> getCurrentPlayer() const;

        unordered_map<int, double> rewards;

        double natureProbability;

    private:

        vector<std::pair<int, int>> getAOH(int player) const;

        unordered_set<int> remainingPlayersInTheRound;
        unordered_map<int, shared_ptr<Action>> performedActionsInThisRound;
        unordered_map<int, shared_ptr<Action>> previousRoundActions;
        optional<int> currentPlayer = nullopt;
        shared_ptr<State> state;
        unordered_map<int, shared_ptr<Observation>> initialObservations;
        unordered_map<int, shared_ptr<Observation>> observations;


        shared_ptr<EFGNode const> parent;
        shared_ptr<Action> incomingAction; // Action performed in the parent node.
    };

}

namespace std {
    template<>
    struct hash<shared_ptr<EFGNode>> {
        size_t operator()(const shared_ptr<EFGNode> &p) const {
            return p->getHash();
        }
    };

    template<>
    struct equal_to<shared_ptr<EFGNode>> {
        bool operator()(const shared_ptr<EFGNode> &a,
                        const shared_ptr<EFGNode> &b) const {
            return *a == *b;
        }
    };
}


#endif  // EFG_H_

#pragma clang diagnostic pop