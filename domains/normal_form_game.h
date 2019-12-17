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


#ifndef GTLIB2_DOMAINS_NORMAL_FORM_GAME_H_
#define GTLIB2_DOMAINS_NORMAL_FORM_GAME_H_

#include "base/base.h"

namespace GTLib2::domains {

struct NFGSettings {
    NFGSettings(vector <vector<double>> utilities_, vector <uint32> dimensions_,
                int numPlayers_, vector <vector<string>> actionNames_) :
        utilities(move(utilities_)),
        dimensions(move(dimensions_)),
        numPlayers(numPlayers_),
        actionNames(move(actionNames_)) {
#ifndef NDEBUG
        // check that each utility has value for each player
        for (const auto &playerUtils : utilities) assert(playerUtils.size() == numPlayers);
#endif
    }

    NFGSettings(vector <vector<double>> _utilities, vector <uint32> _dimensions) :
        NFGSettings(move(_utilities), move(_dimensions), 2, {}) {}

    explicit NFGSettings(vector <vector<double>> twoPlayerZeroSumMatrix) :
        NFGSettings(getUtilities(twoPlayerZeroSumMatrix), {
            (uint32) twoPlayerZeroSumMatrix.size(),
            (uint32) twoPlayerZeroSumMatrix[0].size()
        }, 2, {}) {}

    /**
     * Utilities in the game: inner vector represents utilities for each of the players,
     * outer vector is the indexing of n-dimensional tensor (based on how many players play)
     * by serializing it into a single array.
     */
    const vector <vector<double>> utilities;
    const vector <uint32> dimensions;
    const uint32 numPlayers;
    const vector <vector<string>> actionNames;

    static vector <vector<double>> getUtilities(vector <vector<double>> twoPlayerZeroSumMatrix);
    vector<unsigned int> getIndexingOffsets()  const;
    vector <vector<string>> getActionNames()  const;
    bool isZeroSum() const;
};

class NFGAction: public Action {
 public:
    inline NFGAction() : Action() {}
    inline NFGAction(ActionId id, string actionName) : Action(id), actionName_(move(actionName)) {}
    inline string toString() const override { return actionName_; };
    const string actionName_;
};

class NFGDomain: public Domain {
 public:
    explicit NFGDomain(NFGSettings settings);
    inline string getInfo() const override {
        return to_string(numPlayers_) + " players normal form game";
    }
    inline vector <Player> getPlayers() const {
        vector<Player> ps(numPlayers_);
        std::iota(ps.begin(), ps.end(), 0);
        return ps;
    }
    inline string getActionName(ActionId i, Player pl) const {
        return actionNames_.at(pl).at(i);
    }
    const vector <uint32> dimensions_;
    const uint32 numPlayers_;
    const vector <vector<double>> utilities_;
    const vector<unsigned int> indexingOffsets_;
    const vector <vector<string>> actionNames_;
};


class NFGState: public State {
 public:
    inline NFGState(const Domain *domain, vector <uint32> playerActions) :
        State(domain, hashCombine(98612345434231, playerActions)),
        playedActions_(move(playerActions)) {}

    unsigned long countAvailableActionsFor(Player player) const override;
    vector <shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector <shared_ptr<Action>> &actions) const override;
    inline vector <Player> getPlayers() const override {
        const auto nfgDomain = static_cast<const NFGDomain *>(domain_);
        return nfgDomain->getPlayers();
    }
    inline bool isTerminal() const override { return !playedActions_.empty(); }
    string toString() const override;
    bool operator==(const State &rhs) const override;

    const vector <uint32> playedActions_;
};

} // namespace GTLib2

#endif //GTLIB2_DOMAINS_NORMAL_FORM_GAME_H_
