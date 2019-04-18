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
#ifndef GTLIB2_OSHIZUMO_H
#define GTLIB2_OSHIZUMO_H

#include "base/base.h"

/*
 * Oshi Zumo domain implementation. For game rules go to: http://mlanctot.info/files/papers/aij-2psimmove.pdf, p.43
 */
namespace GTLib2{
namespace domains{

class OshiZumoAction : public Action {
public:
    explicit OshiZumoAction(ActionId id, int bid);
    bool operator==(const Action &that) const override;
    size_t getHash() const override;
    string toString() const;

    inline int getBid() const {
        return bid_;
    }

private:
    int bid_;
};

class OshiZumoDomain : public Domain {
public:
    /*
     * @param startingCoins number of coins each player has at beginning of a game
     * @param startingLoc starting position of wrestler, if startingLoc = 3, then there is (2*3 + 1) = 7 locations, wrestler is in the middle
     * @param minBid minimum allowed bid by player per round
     */
    OshiZumoDomain(int startingCoins, int startingLoc, int minBid);
    /*
     * @param startingCoins number of coins each player has at beginning of a game
     * @param startingLoc starting position of wrestler, if startingLoc = 3, then there is (2*3 + 1) = 7 locations, wrestler is in the middle
     * @param minBid minimum allowed bid by player per round
     * @param optimalEndGame allow to simulate end optimal game, if one of the players can only bid 0
     */
    OshiZumoDomain(int startingCoins, int startingLoc, int minBod, bool optimalEndGame);
    string getInfo() const override;
    inline vector<Player> getPlayers() const final{
        return {0, 1};
    }

    inline const int getStartingLocation() const {
        return startingLocation_;
    }

    inline const int getMinBid() const {
        return minBid_;
    }

    inline const bool isOptimalEndGame() const {
        return optimalEndGame_;
    }


private:
    const int startingCoins_;
    const int startingLocation_;
    const int minBid_;
    const bool optimalEndGame_;
};

class IIOshiZumoDomain : public Domain {
public:
    /*
     * @param startingCoins number of coins each player has at beginning of a game
     * @param startingLoc starting position of wrestler, if startingLoc = 3, then there is (2*3 + 1) = 7 locations, wrestler is in the middle
     * @param minBid minimum allowed bid by player per round
     */
    IIOshiZumoDomain(int startingCoins, int startingLoc, int minBid);
    /*
     * @param startingCoins number of coins each player has at beginning of a game
     * @param startingLoc starting position of wrestler, if startingLoc = 3, then there is (2*3 + 1) = 7 locations, wrestler is in the middle
     * @param minBid minimum allowed bid by player per round
     * @param optimalEndGame allow to simulate end optimal game, if one of the players can only bid 0
     */
    IIOshiZumoDomain(int startingCoins, int startingLoc, int minBod, bool optimalEndGame);
    string getInfo() const override;
    inline vector<Player> getPlayers() const final{
        return {0, 1};
    }

    inline const int getStartingLocation() const {
        return startingLocation_;
    }

    inline const int getMinBid() const {
        return minBid_;
    }

    inline const bool isOptimalEndGame() const {
        return optimalEndGame_;
    }


private:
    const int startingCoins_;
    const int startingLocation_;
    const int minBid_;
    const bool optimalEndGame_;
};


class OshiZumoState : public State {
public:
    OshiZumoState(Domain *domain, int wrestlerPosition, int startingCoins);
    OshiZumoState(Domain *domain, int wrestlerPosition, vector<int> coinsPerPlayer);
    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    unsigned long countAvailableActionsFor(Player player) const override;
    OutcomeDistribution
    performActions(const vector<PlayerAction> &actions) const override;
    vector<Player> getPlayers() const override;
    bool isGameEnd() const;
    string toString() const override;
    bool operator==(const State &rhs) const override;
    size_t getHash() const override;

    inline int getWrestlerLocation() const {
        return wrestlerLocation_;
    }

    inline const vector<int> &getCoins() const {
        return coins_;
    }

protected:
    int wrestlerLocation_;
    vector<int> coins_;

};

class IIOshiZumoState : public OshiZumoState{
public:
    IIOshiZumoState(Domain *domain, int wrestlerPosition, int startingCoins);
    IIOshiZumoState(Domain *domain, int wrestlerPosition, vector<int> coinsPerPlayer);
    OutcomeDistribution performActions(const vector<PlayerAction> &actions) const final;
};

class OshiZumoObservation : public Observation {
public:
    explicit OshiZumoObservation(int opponentBid);

    inline int getOpponentBid() const {
        return opponentBid_;
    }

private:
    int opponentBid_;

};

class IIOshiZumoObservation : public Observation {
public:
    explicit IIOshiZumoObservation(int roundResult);

    inline int getActionResult() const {
        return roundResult_;
    }

private:
    int roundResult_;

};
} // namespace domains
} // namespace GTLib2


#endif //GTLIB2_OSHIZUMO_H
