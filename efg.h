//
// Created by rozliv on 14.08.2017.
//

#ifndef EFG_H_
#define EFG_H_

#include <unordered_map>
#include "pursuit.h"

using std::unordered_map;

/**
 * EFGNode is a class that represents node in an extensive form game,
 * which contains action-observation history, state,
 * rewards (utility) and Information set.
 */
class EFGNode {
 public:
  // constructor
  EFGNode(int player, const shared_ptr<State>& state,
          const vector<double>& rewards, EFGNode* node);

  // constructor
  EFGNode(int player, const shared_ptr<State>& state,
          const vector<double>& rewards, EFGNode* node, vector<int> list);

  // constructor for a first EFGNode
  EFGNode();

  // GetAction returns a possible actions for the player in the node.
  virtual vector<shared_ptr<Action>>  GetAction();

  // PerformAction performs the player's action.
  unique_ptr<EFGNode> PerformAction(const shared_ptr<Action>& action2);

  // GetNumPlayers returns player on the turn (player in this node).
  inline int GetPlayer() const {
    return player_;
  }

  // GetRewards returns rewards for all players in this node.
  inline const vector<double>& GetRewards() const {
    return rewards_;
  }

  // GetState returns a game state
  inline const shared_ptr<State>& GetState() const {
    return state_;
  }

  // GetAOH returns action-observation histories of all players.
  vector<int> GetAOH(int player) const;

  // AddAOH sets action-observation histories of all players.

  // GetHash returns the player's information set.
  inline shared_ptr<InfSet> GetIS() {
    if (infset_ == nullptr)
      infset_ = make_shared<AOH>(player_, GetAOH(player_));
    return infset_;
  }

  // GetParent returns pointer to parent EFGNode.
  inline EFGNode* GetParent() {
    return node_;
  }

  // GetLast returns vector of actions' and observations' id from last state.
  inline const vector<int>& GetLast() const {
    return last_;
  }
  void PushLast(int action, int obs) {
    last_.push_back(action);
    last_.push_back(obs);
  }

 private:
  int player_;
  shared_ptr<State> state_;
  vector<double> rewards_;
  shared_ptr<AOH> infset_;
  EFGNode* node_;
  vector<int> last_;  // last turn
};

/**
 * ChanceNode is a class which encapsulates probability distribution,
 * making new EFGNodes from outcomes
 */
class ChanceNode {
 public:
//  // constructor
  ChanceNode(ProbDistribution* prob,
             const vector<shared_ptr<Action>>& actions,
             const unique_ptr<EFGNode>& node);

  // GetALL returns a vector of all new EFGNodes.
  vector<unique_ptr<EFGNode>> GetAll();

 private:
  ProbDistribution* prob_;  // probability distribution over the new state
  const vector<shared_ptr<Action>>& actions_;
  const unique_ptr<EFGNode>& node_;  // a current node
};


extern unordered_map<size_t, vector<EFGNode>> mapa;

// Domain independent extensive form game treewalk algorithm
void EFGTreewalk(const shared_ptr<Domain>& domain, EFGNode *node,
                 unsigned int depth, int players,
                 const vector<shared_ptr<Action>>& list);

void EFGTreewalk(const shared_ptr<Domain>& domain, EFGNode *node,
                 unsigned int depth, int players,
                 const vector<shared_ptr<Action>>& list,
                 std::function<void(EFGNode*)> FunctionForState);

//// Start method for domain independent extensive form game treewalk algorithm
void EFGTreewalkStart(const shared_ptr<Domain>& domain,
                      unsigned int depth = 0);

void EFGTreewalkStart(const shared_ptr<Domain>& domain,
                      std::function<void(EFGNode*)> FunctionForState,
                      unsigned int depth = 0);

#endif  // EFG_H_
