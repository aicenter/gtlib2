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
#ifndef DOMAINS_GAMBIT_H_
#define DOMAINS_GAMBIT_H_

#include "base/base.h"

namespace GTLib2::domains {

struct Node {
    char node_type;
    int player;
    int infoset_idx;
    int num_actions;
    std::vector<double> utils;
    std::vector<double> probs;
    std::string description;
    std::vector<std::unique_ptr<Node>> children;
    std::vector<std::string> actionLabels;
};

class GambitState;

class GambitDomain: public Domain {
 public:
    explicit GambitDomain(string file);

    string getInfo() const override { return "Gambit - from file: " + file_; };
    vector<Player> getPlayers() const { return {0, 1}; }

 protected:
    friend GambitState;
    OutcomeDistribution createOutcomes(Node* next) const;

 private:
    std::unique_ptr<Node> ParseNodeLine(std::ifstream &in, const std::string &line, int &line_num);

    std::string file_;  // Path to the gambit file

    // cached fields
    double min_utility_;
    double max_utility_;
    int max_game_length_ = 10000;

    // game tree
    std::unique_ptr<Node> root_;

    vector<shared_ptr<Observation>> createPrivateObs(Node *next) const;
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

}  // namespace GTLib2

#endif  // DOMAINS_GOOFSPIEL_H_

#pragma clang diagnostic pop
