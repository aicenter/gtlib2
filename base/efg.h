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

/**
 * EFGNode is a class that represents node in an extensive form game,
 * which contains action-observation history, state,
 * rewards (utility) and Information set.
 */
    class EFGNode : public std::enable_shared_from_this<EFGNode const> {
    public:

        // Constructor for the same round node
        EFGNode(shared_ptr<EFGNode const> parent, const unordered_map<int, shared_ptr<Action>> &performedActions,
                const int lastPlayer);

        // Constructor for the new round node
        EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                const unordered_map<int, shared_ptr<Observation>> &observations,
                const unordered_map<int, double> &rewards,
                const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                double natureProbability);

        // Constructor for the new round node
        EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                const unordered_map<int, shared_ptr<Observation>> &observations,
                const unordered_map<int, double> &rewards,
                const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                double natureProbability,
                const unordered_map<int, shared_ptr<Observation>> &initialObservations);


        // Returns the sequence of actions performed by the player since the root.
        ActionSequence getActionsSeqOfPlayer(int player) const;

        double getProbabilityOfActionsSeqOfPlayer(int player, const BehavioralStrategy &strat) const;

        // Returns available actions for the current player
        vector<shared_ptr<Action>> availableActions() const;

        // Perform the given action and returns the next node or nodes in case of stochastic games together with the probabilities.
        unordered_map<shared_ptr<EFGNode>, double> performAction(shared_ptr<Action> action) const;

        // Gets the information set of the node represented as ActionObservationHistory set.
        shared_ptr<AOH> getAOHInfSet() const;

        // Check if the node is in the given information set.
        bool isContainedInInformationSet(const shared_ptr<AOH> &infSet) const;

        virtual string description() const { return "No Description"; }; //TODO: Change to abstract method

        // Gets the parent efg node.
        shared_ptr<EFGNode const> getParent() const;

        // Returns the game state of that is represented by EFG node. Note that in simultaneous games one state corresponds to
        // mutliple efg nodes.
        shared_ptr<State> getState() const;


        size_t getHash() const;

        bool operator==(const EFGNode &rhs) const;

        int getDistanceFromRoot() const;




        optional<int> getCurrentPlayer() const;

        unordered_map<int, double> rewards;

        double natureProbability;

    private:

        vector<std::tuple<int, int>> getAOH(int player) const;

        vector<int> remainingPlayersInTheRound;
        unordered_map<int, shared_ptr<Action>> performedActionsInThisRound;
        optional<int> currentPlayer = nullopt;
        shared_ptr<State> state;
        unordered_map<int, shared_ptr<Observation>> initialObservations;
        unordered_map<int, shared_ptr<Observation>> observations;

        unordered_map<int, shared_ptr<Action>> previousRoundActions;
        shared_ptr<EFGNode const> parent;

        //----------------DEPRECATED_BELLOW-----------------------------

    public:


        // GetParent returns pointer to parent EFGNode.
        [[deprecated]]
        inline EFGNode *GetParent() const {
            return nullptr;
            //return parent.get();
        }


        [[deprecated]]
        EFGNode();

        // constructor
        [[deprecated]]
        EFGNode(int player, const shared_ptr<State> &state,
                const vector<double> &rewards, EFGNode *node);

        // constructor
        [[deprecated]]
        EFGNode(int player, const shared_ptr<State> &state,
                const vector<double> &rewards, EFGNode *node, vector<int> list);


        // GetAction returns a possible actions for the player in the node.
        [[deprecated]]
        vector<shared_ptr<Action>> GetAction();

        // OldPerformAction performs the player's action.
        [[deprecated]]
        unique_ptr<EFGNode> OldPerformAction(const shared_ptr<Action> &action2);

        // GetNumPlayers returns player on the turn (player in this node).
        [[deprecated]]
        inline int GetPlayer() const {
            return player_;
        }

        // GetRewards returns rewards for all players in this node.
        [[deprecated]]
        inline const vector<double> &GetRewards() const {
            return rewards_;
        }


        // OldGetAOH returns action-observation histories of all players.
        [[deprecated]]
        vector<int> OldGetAOH(int player) const;

        // GetIS returns the player's information set.
        [[deprecated]]
        inline shared_ptr<InformationSet> GetIS() {
            if (infset_ == nullptr)
                infset_ = make_shared<AOH>(player_, OldGetAOH(player_));
            return infset_;
        }


        // GetLast returns vector of actions' and observations' id from last state.
        [[deprecated]]
        inline const vector<int> &GetLast() const {
            return last_;
        }

        [[deprecated]]
        void PushLast(int action, int obs) {
            last_.push_back(action);
            last_.push_back(obs);
        }


    private:
        [[deprecated]]
        vector<double> rewards_;

        [[deprecated]]
        int player_;

        [[deprecated]]
        shared_ptr<AOH> infset_;

        [[deprecated]]
        vector<int> last_;  // last turn
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