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

        // constructor for the first EFGNode
        EFGNode();

        // Constructor for the same round node
        EFGNode(shared_ptr<EFGNode const> lastNode, const shared_ptr<Action> &lastAction,
                int lastPlayer, vector<int> remainingPlayers);

        // Constructor for the new round node
        EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> lastNode,
                const unordered_map<int, shared_ptr<Observation>> &observations,
                const unordered_map<int, double> &rewards,
                const unordered_map<int, shared_ptr<Action>> &lastRoundActions);

        // Constructor for the new round node
        EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> lastNode,
                const unordered_map<int, shared_ptr<Observation>> &observations,
                const unordered_map<int, double> &rewards,
                const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                const unordered_map<int, shared_ptr<Observation>> &initialObservations);


        // Returns the sequence of actions performed by the player since the root.
        ActionSequence getActionHistoryOfPlayer(int player) const;

        // Returns available actions for the current player
        vector<shared_ptr<Action>> availableActions() const;

        // Perform the given action and returns the next node or nodes in case of stochastic games together with the probabilities.
        unordered_map<shared_ptr<EFGNode>, double> pavelPerformAction(shared_ptr<Action> action) const;

        // Gets the information set of the node represented as ActionObservationHistory set.
        shared_ptr<AOH> pavelgetAOHInfSet() const;

        // Check if the node is in the given information set.
        bool containedInInformationSet(const shared_ptr<AOH> &infSet) const;

        virtual string description() const { return "No Description"; }; //TODO: Change to abstract method

        // Gets the parent efg node.
        shared_ptr<EFGNode const> getParent() const;

        // Returns the game state of that is represented by EFG node. Note that in simultaneous games one state corresponds to
        // mutliple efg nodes.
        shared_ptr<State> getState() const;



        double getProbabilityOfSeqOfPlayer(int player, const BehavioralStrategy &strat) const;

        optional<int> getCurrentPlayer() const;

        unordered_map<int, double> rewards;

    private:

        vector<std::tuple<int, int>> pavelGetAOH(int player) const;

        vector<int> remainingPlayersInTheRound;
        unordered_map<int, shared_ptr<Action>> performedActionsInThisRound;
        optional<int> currentPlayer = nullopt;
        shared_ptr<State> state;
        unordered_map<int, shared_ptr<Observation>> initialObservations;
        unordered_map<int, shared_ptr<Observation>> observations;

        unordered_map<int, shared_ptr<Action>> lastRoundActions;
        shared_ptr<EFGNode const> parent;

        //---------------------------------------------

    public:


        // GetParent returns pointer to parent EFGNode.
        [[deprecated]]
        inline EFGNode *GetParent() const {
            return nullptr;
            //return parent.get();
        }

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

        // PerformAction performs the player's action.
        [[deprecated]]
        unique_ptr<EFGNode> PerformAction(const shared_ptr<Action> &action2);

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


        // GetAOH returns action-observation histories of all players.
        [[deprecated]]
        vector<int> GetAOH(int player) const;

        // GetIS returns the player's information set.
        [[deprecated]]
        inline shared_ptr<InformationSet> GetIS() {
            if (infset_ == nullptr)
                infset_ = make_shared<AOH>(player_, GetAOH(player_));
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


// The rest of the file is obsolete

#endif  // EFG_H_

#pragma clang diagnostic pop