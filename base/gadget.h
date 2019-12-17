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

    You should have received a copy of the GNU Lesser General Public
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BASE_GADGET_H_
#define BASE_GADGET_H_

#include "base/efg.h"

#pragma GCC diagnostic push
// there are bunch of methods in gadget which do not have implementations, and should not be called
#pragma GCC diagnostic ignored "-Wreturn-type"

namespace GTLib2 {


struct PublicStateSummary {
    const shared_ptr<PublicState> publicState;
    const vector<shared_ptr<EFGNode>> topmostHistories;
    const vector<array<double, 3>> topmostHistoriesReachProbs;
    const vector<double> expectedValues; // for player 0 for each topmost history

    PublicStateSummary(shared_ptr<PublicState> _publicState,
                       vector<shared_ptr<EFGNode>> _topmostHistories,
                       vector<array<double, 3>> _topmostHistoriesReachProbs,
                       vector<double> _expectedValues)
        : publicState(move(_publicState)),
          topmostHistories(move(_topmostHistories)),
          topmostHistoriesReachProbs(move(_topmostHistoriesReachProbs)),
          expectedValues(move(_expectedValues)) {
#ifndef NDEBUG
        for (const auto &h : topmostHistories) {
            assert(h->getPubObsIds() == publicState->getHistory());
        }
#endif
    }
};

class GadgetRootNode;

enum GadgetVariant {
    SAFE_RESOLVING,
    UNSAFE_RESOLVING,
    MAX_MARGIN // todo: paper Solving Endgames in Large Imperfect-Information Games such as Poker
};

class GadgetGame {
 public:
    // Information needed to construct the gadget game
    const PublicStateSummary summary_;
    // Player for which we want to recover strategy
    const Player resolvingPlayer_;
    // Player after chance node, he chooses follow / terminate
    const Player viewingPlayer_;
    // Specific infoset we are interested in (can be nullptr if there's none)
    const shared_ptr<AOH> targetAOH_;

    const GadgetVariant variant_;

    // Defined as $ sum_{h \in \hat{S}} pi_{resolvingPlayer,c}(h) $
    const double pubStateReach_;
    // CFV values to put into "Terminate" nodes (for player 0)
    const vector<double> cfvValues_;


 public:
    GadgetGame(PublicStateSummary summary,
               Player resolvingPlayer,
               shared_ptr<AOH> targetAOH,
               GadgetVariant variant) :
        summary_(move(summary)),
        resolvingPlayer_(resolvingPlayer),
        viewingPlayer_(opponent(resolvingPlayer)),
        targetAOH_(move(targetAOH)),
        variant_(variant),
        pubStateReach_(computePubStateReach()),
        cfvValues_(computeTerminateCFVValues()) {}

    inline const shared_ptr<GadgetRootNode> getRootNode() {
        return make_shared<GadgetRootNode>(*this);
    }

    double chanceProbForAction(const ActionId &action) const;

 private:
    double computePubStateReach();
    vector<double> computeTerminateCFVValues();
};

class GadgetRootNode: public EFGNode,
                      public std::enable_shared_from_this<GadgetRootNode const> {
    const vector<ActionId> history_ = {};
 public:

    const GadgetGame &gadget_;
    inline explicit GadgetRootNode(const GadgetGame &game) : gadget_(game) {};

    inline EFGNodeSpecialization getSpecialization() const override { return GadgetSpecialization; }

    inline unsigned long countAvailableActions() const override {
        return gadget_.summary_.topmostHistories.size();
    }
    inline vector<shared_ptr<Action>> availableActions() const override {
        vector<shared_ptr<Action>> actions;
        for (unsigned int i = 0; i < countAvailableActions(); ++i) {
            actions.emplace_back(make_shared<EFGChanceAction>(i, chanceProbForAction(i)));
        }
        return actions;
    }

    shared_ptr<EFGNode> performAction(const shared_ptr<Action> &action) const override;
    double chanceProbForAction(const ActionId &action) const override;

    inline double chanceProbForAction(const shared_ptr<Action> &action) const override {
        return chanceProbForAction(action->getId());
    }
    inline shared_ptr<const EFGNode> getParent() const override { return nullptr; }
    inline unsigned int efgDepth() const override { return 0; }

    inline vector<ActionObservationIds> getAOids(Player ) const override {
        unreachable("do not try to build ao out of gadget root");
    }
    inline vector<ObservationId> getPubObsIds() const override {
        unreachable("do not try to build public state out of gadget");
    }
    inline const vector<ActionId> &getHistory() const override { return history_; }
    inline double getProbabilityOfActionSeq(Player , const BehavioralStrategy &)
    const override { // todo
        return 0;
    }
    inline shared_ptr<ActionSequence> getActionsSeqOfPlayer(Player ) const override {
        return shared_ptr<ActionSequence>(); // todo:
    }
    inline HashType getHash() const override { return 0; }
    inline bool operator==(const EFGNode &rhs) const override {
        return rhs.getSpecialization() == getSpecialization()
            && rhs.type_ == type_
            && EFGNode::operator==(rhs);
    }
};

constexpr ActionId GADGET_FOLLOW = 0;
constexpr ActionId GADGET_TERMINATE = 1;

class GadgetAction: public Action {
 public:
    inline explicit GadgetAction(ActionId id) : Action(id) {}
    string toString() const override {
        if (id_ == 0) return "Follow";
        if (id_ == 1) return "Terminate";
        unreachable("no such action");
    }
};

class GadgetInnerNode: public EFGNode,
                       public std::enable_shared_from_this<GadgetInnerNode const> {
 public:

    const GadgetGame &game_;
    const shared_ptr<GadgetRootNode const> &parent_;
    const shared_ptr<EFGNode> underlyingNode_;
    const vector<ActionObservationIds> viewingPlAOIds_;
    const vector<ActionId> history_;

    explicit GadgetInnerNode(const GadgetGame &game,
                             const shared_ptr<GadgetRootNode const> &parent,
                             shared_ptr<EFGNode> underlyingNode,
                             vector<ActionObservationIds> viewingPlAOids,
                             const EdgeId transitionId) :
        EFGNode(opponent(game.resolvingPlayer_)),
        game_(game), parent_(parent), underlyingNode_(move(underlyingNode)),
        viewingPlAOIds_(move(viewingPlAOids)), history_({transitionId}) {}

    shared_ptr<EFGNode> performAction(const shared_ptr<Action> &action) const override;

    inline EFGNodeSpecialization getSpecialization() const override { return GadgetSpecialization; }

    unsigned long countAvailableActions() const override { return 2; }

    inline vector<shared_ptr<Action>> availableActions() const override {
        return vector<shared_ptr<Action>>{
            make_shared<GadgetAction>(GADGET_FOLLOW),
            make_shared<GadgetAction>(GADGET_TERMINATE)
        };
    }

    inline double chanceProbForAction(const ActionId &) const override {
        unreachable("not a chance node!");
    }

    inline double chanceProbForAction(const shared_ptr<Action> &) const override {
        unreachable("not a chance node!");
    }
    inline ProbDistribution chanceProbs() const override {
        unreachable("not a chance node!");
    }
    inline shared_ptr<const EFGNode> getParent() const override { return parent_; }
    inline unsigned int efgDepth() const override { return 1; }
    inline vector<ActionObservationIds> getAOids(Player ) const override {
        return viewingPlAOIds_;
    }
    inline vector<ObservationId> getPubObsIds() const override {
        unreachable("do not try to build public state out of gadget");
    }
    inline const vector<ActionId> &getHistory() const override { return history_; }
    inline double getProbabilityOfActionSeq(Player , const BehavioralStrategy &)
    const override { // todo
        return 0;
    }
    inline shared_ptr<ActionSequence> getActionsSeqOfPlayer(Player ) const override {
        return shared_ptr<ActionSequence>(); // todo:
    }
    inline HashType getHash() const override { return history_[0]; }
    inline bool operator==(const EFGNode &rhs) const override {
        return rhs.getSpecialization() == getSpecialization()
            && rhs.type_ == type_
            && EFGNode::operator==(rhs);
    }
};

class GadgetTerminalNode: public EFGNode {
    const shared_ptr<GadgetInnerNode const> &parent_;
    const vector<ActionId> history_;

 public:
    explicit GadgetTerminalNode(const shared_ptr<GadgetInnerNode const> &parent,
                                vector<double> utilities) :
        EFGNode(move(utilities)), parent_(parent),
        history_(extend(parent_->history_, GADGET_TERMINATE)) {}

    inline EFGNodeSpecialization getSpecialization() const override { return GadgetSpecialization; }

    inline unsigned long countAvailableActions() const override { return 0; }

    inline vector<shared_ptr<Action>> availableActions() const override {
        unreachable("terminal node!");
    }

    inline shared_ptr<EFGNode> performAction(const shared_ptr<Action> &) const override {
        unreachable("terminal node!");
    }

    inline double chanceProbForAction(const ActionId &) const override {
        unreachable("terminal node!");
    }

    inline double chanceProbForAction(const shared_ptr<Action> &) const override {
        unreachable("terminal node!");
    }

    inline ProbDistribution chanceProbs() const override {
        unreachable("terminal node!");
    }
    inline shared_ptr<const EFGNode> getParent() const override { return parent_; }
    inline unsigned int efgDepth() const override { return 2; }

    inline vector<ActionObservationIds> getAOids(Player ) const override {
        unreachable("terminal node!");
    }
    inline vector<ObservationId> getPubObsIds() const override {
        unreachable("do not try to build public state out of gadget");
    }
    inline const vector<ActionId> &getHistory() const override { return history_; }
    inline double getProbabilityOfActionSeq(Player , const BehavioralStrategy &)
    const override { // todo
        return 0;
    }
    inline shared_ptr<ActionSequence> getActionsSeqOfPlayer(Player ) const override {
        return shared_ptr<ActionSequence>(); // todo:
    }
    inline HashType getHash() const override { return history_[0]; }
    inline bool operator==(const EFGNode &rhs) const override {
        return rhs.getSpecialization() == getSpecialization()
            && rhs.type_ == type_
            && EFGNode::operator==(rhs);
    }
};

#pragma GCC diagnostic pop

}

#endif  // BASE_EFG_H_
