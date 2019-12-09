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
#ifndef BASE_EFG_H_
#define BASE_EFG_H_

#include "base/base.h"
#include "base/tree.h"


namespace GTLib2 {


enum EFGNodeType {
    ChanceNode,
    PlayerNode,
    TerminalNode
};

/**
 * Add the ability to descendants of EFGNode to state what kind of node they are.
 * This is used in order to avoid typeid calls.
 */
enum EFGNodeSpecialization {
    NoSpecialization,
    GadgetSpecialization
};

class EFGChanceAction: public Action {
 public:
    explicit inline EFGChanceAction(ActionId id, double chanceProb)
        : Action(id), chanceProb_(chanceProb) {}
    inline string toString() const override {
        return to_string(id_) + ", p=" + to_string(chanceProb_);
    }
    bool operator==(const Action &that) const override {
        if (typeid(that) == typeid(*this)) {
            const auto rhsAction = static_cast<const EFGChanceAction *>(&that);
            return id_ == rhsAction->id_;
        }
        return false;
    }
    inline HashType getHash() const override { return id_; }

    double chanceProb_ = 1.0;
};


class PublicState: public Node<PublicState> {
 public:
    inline PublicState() : Node() {}
    inline PublicState(shared_ptr<PublicState const> parent, vector<ObservationId> history) :
        Node(move(parent), history) {}

    const vector<ActionId> &getHistory() const { return history_; };

    inline friend std::ostream & operator<<(std::ostream &ss, const PublicState &a) {
        ss << a.history_;
        return ss;
    }

//    const shared_ptr<Observation> incomingObservation_;
//    unsigned int countChildren() const override { return 0; };
//    const shared_ptr <PublicState> getChildAt(EdgeId index) const override { return nullptr; };
};

/**
 * EFGNode is a virtual class that represents node in an extensive form game (EFG).
 *
 * In Haskell lingo, this is a "type class", like "Eq a",
 * and FOG2EFG would be a "instance of this type class"
 *
 * This is done so that we do not need to specify template arguments to Node everywhere, and
 * we can have various implementation of EFGNode (for example GadgetNode)
 */
class EFGNode {
 protected:
    inline explicit EFGNode(const EFGNodeType type,
                            const Player currentPlayer,
                            vector<double> utilities) :
        type_(type),
        // use NO_PLAYER so we don't allocate more memory by optional
        currentPlayer_(type_ == PlayerNode ? currentPlayer : NO_PLAYER),
        utilities_(move(utilities)) {}

    inline explicit EFGNode() :
        EFGNode(ChanceNode, NO_PLAYER, vector<double>()) {}
    inline explicit EFGNode(Player currentPlayer) :
        EFGNode(PlayerNode, currentPlayer, vector<double>()) {}
    inline explicit EFGNode(vector<double> utilities) :
        EFGNode(TerminalNode, NO_PLAYER, move(utilities)) {}

    inline EFGNode(const EFGNode& other) :
        type_(other.type_),
        currentPlayer_(other.currentPlayer_),
        utilities_(other.utilities_) {}

 public:
    virtual EFGNodeSpecialization getSpecialization() const = 0;

    /**
     * Return parent node or nullptr if current node is a root node.
     */
    virtual shared_ptr<EFGNode const> getParent() const = 0;

    /**
     * Return the depth of this node in the EFG tree. Root node has depth of 0.
     */
    virtual unsigned int efgDepth() const = 0;

    /**
     * Returns number of available actions in the node.
     * Terminal nodes do not have any available actions.
     */
    virtual unsigned long countAvailableActions() const = 0;

    /**
     * Returns available actions in the node.
     * Terminal nodes do not have any available actions.
     */
    virtual vector<shared_ptr<Action>> availableActions() const = 0;

    /**
     * Perform the given action and returns the next EFG node.
     */
    virtual shared_ptr<EFGNode> performAction(const shared_ptr<Action> &action) const = 0;

    /**
     * In case the current node is a chance node, return probability for given action.
     */
    virtual double chanceProbForAction(const ActionId &action) const = 0;

    /**
     * In case the current node is a chance node, return probability for given action.
     */
    virtual double chanceProbForAction(const shared_ptr<Action> &action) const = 0;

    /**
     * In case the current node is a chance node,
     * return probability distribution over possible actions.
     *
     * They should be properly indexed, i.e. ProbDistribution[0] is for ActionId == 0
     */
    inline virtual ProbDistribution chanceProbs() const {
        assert(type_ == ChanceNode);
        const auto numActions = countAvailableActions();
        auto dist = ProbDistribution();
        dist.reserve(numActions);
        for (int i = 0; i < numActions; ++i) dist.push_back(chanceProbForAction(i));
        return dist;
    }

    /**
     * Describe the vector of (action,observation) of given player.
     * It is used to construct AOH infoset.
     */
    virtual vector<ActionObservationIds> getAOids(Player player) const = 0;

    /**
      * Describe the vector of (public_observation)
      * It is used to construct public state.
      */
    virtual vector<ObservationId> getPubObsIds() const = 0;

    virtual const vector<ActionId> &getHistory() const = 0;

    inline const ActionId getLastActionId() const {
        const auto &h = getHistory();
        assert(!h.empty());
        return h.at(h.size()-1);
    }

    virtual double getProbabilityOfActionSeq(Player player,
                                             const BehavioralStrategy &strat) const = 0; // todo: remove
    virtual shared_ptr<ActionSequence>
    getActionsSeqOfPlayer(Player player) const = 0; // todo: remove
    virtual HashType getHash() const = 0;

    virtual inline bool operator==(const EFGNode &rhs) const {
        if (getHash() != rhs.getHash()) return false;
        if (getHistory().size() != rhs.getHistory().size()) return false;
        return !memcmp(getHistory().data(), rhs.getHistory().data(), getHistory().size());
    };


    /**
     * Return the public state of the node based on public observation history.
     */
    inline shared_ptr<PublicState> getPublicState() const {
        if (getParent()) {
            const auto parentPub = getParent()->getPublicState();
            const auto pubHistory = getPubObsIds();
            if (parentPub->getHistory() == pubHistory) return parentPub;
            return make_shared<PublicState>(parentPub, pubHistory);
        }
        return make_shared<PublicState>();
    };

    /**
     * Gets the augmented information set of the node.
     *
     * Note that augmented information sets coincide with ordinary information sets
     * when the requested player is acting in this node.
     */
    inline shared_ptr<AOH> getAOHAugInfSet(Player player) const {
        const bool isActingPlayer = type_ == PlayerNode && getPlayer() == player;
        return make_shared<AOH>(player, isActingPlayer, getAOids(player));
    };

    /**
     * Gets the information set of the node represented as ActionObservationHistory set.
     */
    inline shared_ptr<AOH> getAOHInfSet() const {
        assert(type_ == PlayerNode);
        return getAOHAugInfSet(currentPlayer_);
    }

    /**
     * Get current player playing in this round.
     */
    inline Player getPlayer() const {
        assert(type_ == PlayerNode);
        return currentPlayer_;
    }

    /**
     * Return terminal utilities.
     */
    inline vector<double> getUtilities() const {
        assert(type_ == TerminalNode);
        return utilities_;
    }

    inline friend std::ostream & operator<<(std::ostream &ss, const EFGNode &n) {
        switch (n.type_) {
            case ChanceNode:
                ss << "C";
                break;
            case PlayerNode:
                ss << "P" << int(n.getPlayer());
                break;
            case TerminalNode:
                ss << "T" << n.getUtilities();
                break;
            default:
                unreachable("unrecognized option!");
        }
        ss << " " << n.getHistory();
        return ss;
    }

    const EFGNodeType type_;
 protected:
    const Player currentPlayer_;
    const vector<double> utilities_;
};

};  // namespace GTLib2

MAKE_EQ(GTLib2::EFGNode)
MAKE_HASHABLE(GTLib2::EFGNode)
MAKE_EQ(GTLib2::PublicState)
MAKE_HASHABLE(GTLib2::PublicState)

#endif  // BASE_EFG_H_

#pragma clang diagnostic pop
