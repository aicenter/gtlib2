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


#ifndef GTLIB2_GAMEPLAYINGALGORITHM_H
#define GTLIB2_GAMEPLAYINGALGORITHM_H

#include "base/base.h"
#include "base/efg.h"
#include "base/cache.h"
#include <random>
#include <utility>

namespace GTLib2 {

/**
 * Algorithm that is capable of playing games by being supplied
 * current infoset, domain and for which player to play.
 */
class GamePlayingAlgorithm {
 public:
    const Domain &domain_;
    const Player playingPlayer_;

    /**
     * Prepare for playing game on specified domain.
     * Algorithm is not allowed to run any computation in constructor.
     */
    GamePlayingAlgorithm(const Domain &domain, Player playingPlayer)
        : domain_(domain), playingPlayer_(playingPlayer) {};

    /**
     * Run one step of the algorithm and improve play distribution in current infoset.
     *
     * Infoset in which this algorithm should currently play is provided.
     * It will always be only infosets of the previously specified player.
     * If currentInfoset is empty, it means root iterations (preplay) should be done.
     *
     * @return whether algorithm decided to continue to play (true) or give up the game (false).
     */
    virtual bool runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) = 0;

    /**
     * Return probability distribution by which the next action should be selected.
     * They must sum up to 1.
     */
    virtual vector<double> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) = 0;
};

/**
 * Run iterations of give algorithm for a given time budget in microseconds.
 */
bool playForMicroseconds(unique_ptr<GamePlayingAlgorithm> &alg,
                         const optional<shared_ptr<AOH>> &currentInfoset,
                         long budgetUs);

/**
 * Random player gives up right away, and the rest of the match is played uniformly randomly.
 */
class RandomPlayer: public GamePlayingAlgorithm {
 public:
    inline RandomPlayer(const Domain &domain, Player playingPlayer)
        : GamePlayingAlgorithm(domain, playingPlayer) {}
    inline bool runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override { return false; };
    inline vector<double> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override {};
};

/**
 * Player that always chooses specified fixed action.
 * You can use modulo algebra to specify the action, even with negative integers.
 */
class FixedActionPlayer: public GamePlayingAlgorithm {
 public:
    explicit FixedActionPlayer(const Domain &domain, Player playingPlayer, int actionIdx);
    bool runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    vector<double> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;
 private:
    InfosetCache cache_;
    const int actionIdx_;
};

typedef std::function<std::unique_ptr<GamePlayingAlgorithm>(const Domain &, Player)>
    PreparedAlgorithm;

template<typename T, typename... Args>
PreparedAlgorithm createInitializer(Args... args) {
    return [=](const Domain &domain, Player pl) -> std::unique_ptr<GamePlayingAlgorithm> {
        return std::make_unique<T>(domain, pl, args ...);
    };
}

/**
 * Play match between given algorithms, for a given number of microseconds in the root (preplay)
 * and another time budget for each move. Return terminal utilities for each algorithm.
 *
 * You can prepare the algorithm by calling
 * @code
 * DomainInitilizer preparedAlg = createInitializer<MyAlg>( my_alg_params );
 * @endcode
 */
vector<double> playMatch(const Domain &domain,
                         vector<PreparedAlgorithm> algorithmInitializers,
                         vector<int> preplayBudgetMicrosec,
                         vector<int> moveBudgetMicrosec,
                         unsigned long matchSeed);
}  // namespace GTLib2


#endif //GTLIB2_GAMEPLAYINGALGORITHM_H
