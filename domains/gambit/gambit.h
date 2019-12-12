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


#ifndef DOMAINS_GAMBIT_H_
#define DOMAINS_GAMBIT_H_

#include "base/base.h"

// Gambit parser: load from file or from string stream.

namespace GTLib2::domains::gambit {

class GambitState;
class GambitDomain;
unique_ptr<GambitDomain> loadFromFile(const string &file);

struct Node {
    char node_type;
    int player;
    unsigned long infoset_idx;
    unsigned long pubstate_idx;
    int num_actions;
    std::vector<double> utils;
    std::vector<double> probs;
    std::string description;
    std::vector<std::unique_ptr<Node>> children;

    bool operator==(const Node &rhs) const {
        return node_type == rhs.node_type
            && player == rhs.player
            && infoset_idx == rhs.infoset_idx
            && pubstate_idx == rhs.pubstate_idx
            && num_actions == rhs.num_actions
            && utils == rhs.utils
            && probs == rhs.probs
            && description == rhs.description;
//            && children == rhs.children;
    }
    friend std::ostream &operator<<(std::ostream &os, const Node &node) {
        os << "node_type: " << node.node_type
           << " player: " << node.player
           << " infoset_idx: " << node.infoset_idx
           << " pubstate_idx: " << node.pubstate_idx
           << " num_actions: " << node.num_actions
           << " utils: " << node.utils
           << " probs: " << node.probs
           << " description: " << node.description;
        return os;
    }
};

class GambitDomain: public Domain {
 public:
    explicit GambitDomain(std::istream &in);

    string getInfo() const override { return "Gambit domain"; };
    vector<Player> getPlayers() const { return {0, 1}; }

 protected:
    friend GambitState;
    OutcomeDistribution createOutcomes(Node *next) const;
    vector<shared_ptr<Observation>> createPrivateObs(Node *next) const;
    shared_ptr<Observation> createPublicObs(Node *next) const;

 private:
    std::unique_ptr<Node> parseSubtree(std::istream &in, const std::string &line, int &line_num);
    std::unique_ptr<Node> root_; // game tree
};


class GambitState: public State {
 public:
    explicit GambitState(const Domain *domain, Node *n)
        : State(domain, hashCombine(0, long(n))), // I dont have time to write more thorough hash
          n_(n) {}

    inline unsigned long countAvailableActionsFor(Player player) const override {
        if (player != n_->player) return 0;
        return n_->children.size();
    }
    inline vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override {
        auto actions = vector<shared_ptr<Action>>();
        actions.reserve(countAvailableActionsFor(player));
        for (int i = 0; i < countAvailableActionsFor(player); ++i) {
            actions.push_back(make_shared<Action>(i));
        }
        return actions;
    };
    OutcomeDistribution performActions(const vector<shared_ptr<Action>> &actions) const override;
    inline vector<Player> getPlayers() const override {
        if (n_->node_type != 'p') return {};
        return {Player(n_->player)};
    }
    inline bool isTerminal() const override { return n_->node_type == 't'; }
    inline string toString() const override { return n_->description; }
    inline bool operator==(const State &rhs) const override {
        auto gsState = dynamic_cast<const GambitState &>(rhs);
        return n_ == gsState.n_;
    };

 private:
    Node *n_;
};

std::unique_ptr<Node> parseNodeLine(const std::string &line, const int &line_num);

}  // namespace GTLib2

#endif  // DOMAINS_GAMBIT_H_
