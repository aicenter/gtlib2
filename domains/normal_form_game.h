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
#ifndef GTLIB2_DOMAINS_NORMAL_FORM_GAME_H_
#define GTLIB2_DOMAINS_NORMAL_FORM_GAME_H_

#include "base/base.h"

namespace GTLib2::domains {

struct NFGSettings {

    NFGSettings(vector <vector<double >> utilities, vector <uint32> dimensions) :
        utilities(utilities),
        dimensions(dimensions) {}

    NFGSettings(vector <vector<double >> utilities, vector <uint32> dimensions, int numPlayers) :
        utilities(utilities),
        dimensions(dimensions),
        numPlayers(numPlayers) {}

    NFGSettings(vector<vector<double>> twoPlayerZeroSumMatrix) :
        dimensions({(uint32)twoPlayerZeroSumMatrix.size(), (uint32)twoPlayerZeroSumMatrix[0].size()}),
        utilities(getUtilities(twoPlayerZeroSumMatrix)) {}

    vector <vector<double>> utilities = {};
    uint32 numPlayers = 2;
    vector <uint32> dimensions;

    vector <vector<double >> getUtilities(vector<vector<double>> twoPlayerZeroSumMatrix);
    vector<unsigned int> getIndexingOffsets();
};

class NFGAction : public Action {
 public:
    inline NFGAction() : Action() {}
    inline NFGAction(ActionId id) : Action(id) {}
    inline string toString() const override { return "Id of action: " + to_string(id_); };
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return id_; };

};

class NFGDomain : public Domain {
 public:
    explicit NFGDomain(NFGSettings settings);
    string getInfo() const override;
    vector <Player> getPlayers() const;
    const vector <uint32> dimensions_;
    const uint32 numPlayers_;
    const vector <vector<double>> utilities_;
    const vector<unsigned int> indexingOffsets_;

};

class NFGObservation : public Observation {
 public:
    inline NFGObservation() :
        Observation() {}
};

class NFGState : public State {
 public:
    inline NFGState(const Domain *domain, bool terminal, vector <uint32> playerActions) :
        State(domain, hashCombine(98612345434231, terminal, playerActions)),
        terminal_(terminal),
        playerActions_(playerActions) {}

    unsigned long countAvailableActionsFor(Player player) const override;
    vector <shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector <shared_ptr<Action>> &actions) const override;
    vector <Player> getPlayers() const override;
    bool isTerminal() const override;
    string toString() const override;
    bool operator==(const State &rhs) const override;

    const bool terminal_;
    vector <uint32> playerActions_;
};

} // namespace GTLib2

#endif //GTLIB2_DOMAINS_NORMAL_FORM_GAME_H_
