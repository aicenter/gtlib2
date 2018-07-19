//
// Created by Pavel Rytir on 1/20/18.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_EFG_OLD_H
#define PURSUIT_EFG_OLD_H

#include "base.h"
#include "efg.h"

namespace GTLib2 {



  class [[deprecated]] OldEFGNode {
   public:


    // GetParent returns pointer to parent EFGNode.
    [[deprecated]]
    inline OldEFGNode *GetParent() const {
        assert(("U r using deprecated method", false));
        return nullptr;
        //return parent.get();
    }


    [[deprecated]]
    OldEFGNode();

    // constructor
    [[deprecated]]
    OldEFGNode(int player, const shared_ptr<State> &state,
            const vector<double> &rewards, EFGNode *node);

    // constructor
    [[deprecated]]
    OldEFGNode(int player, const shared_ptr<State> &state,
            const vector<double> &rewards, EFGNode *node, vector<int> list);


    // GetAction returns a possible actions for the player in the node.
    [[deprecated]]
    vector<shared_ptr<Action>> GetAction();

    // OldPerformAction performs the player's action.
    [[deprecated]]
    unique_ptr<OldEFGNode> OldPerformAction(const shared_ptr<Action> &action2);

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


/**
 * ChanceNode is a class which encapsulates probability distribution,
 * making new EFGNodes from outcomes
 */

    class [[deprecated]] ChanceNode {
    public:
//  // constructor
        ChanceNode(OutcomeDistributionOld *prob,
                   const vector<shared_ptr<Action>> &actions,
                   const unique_ptr<EFGNode> &node);

        // GetALL returns a vector of all new EFGNodes.
        vector<unique_ptr<EFGNode>> GetAll();

    private:
        OutcomeDistributionOld *prob_;  // probability distribution over the new state
        const vector<shared_ptr<Action>> &actions_;
        const unique_ptr<EFGNode> &node_;  // a current node
    };


    extern unordered_map<size_t, vector<EFGNode>> mapa;


// Domain independent extensive form game treewalk algorithm
    [[deprecated]]
    void OldEFGTreewalk(const shared_ptr<Domain> &domain, EFGNode *node,
                        unsigned int depth, int players,
                        const vector<shared_ptr<Action>> &list);

    [[deprecated]]
    void OldEFGTreewalk(const shared_ptr<Domain> &domain, EFGNode *node,
                        unsigned int depth, int players,
                        const vector<shared_ptr<Action>> &list,
                        std::function<void(EFGNode *)> FunctionForState);

//// Start method for domain independent extensive form game treewalk algorithm
    [[deprecated]]
    void OldEFGTreewalkStart(const shared_ptr<Domain> &domain,
                             unsigned int depth = 0);

    [[deprecated]]
    void OldEFGTreewalkStart(const shared_ptr<Domain> &domain,
                             std::function<void(EFGNode *)> FunctionForState,
                             unsigned int depth = 0);

}

#endif //PURSUIT_EFG_OLD_H

#pragma clang diagnostic pop