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

#include "base/cache.h"

namespace GTLib2 {

/**
 * Specify the control flow of the playing algorithm.
 * If
 * - ContinueImproving: runPlayIteration will be called again in current round if there is time left
 * - StopImproving: do not call another runPlayIteration in this round
 * - GiveUp: do not call runPlayIteration anymore in the match and play randomly for the rest of the game
 */
enum PlayControl {
    ContinueImproving,
    StopImproving,
    GiveUp,
};

struct AlgConfig {
    inline virtual void update(const string &key, const string &value) {
        LOG_WARN("The key/value '" << key << "': '" << value << "' cannot be set!")
    };
    inline void updateAll(const unordered_map<string, string> &params) {
        for (const auto &[k, v] : params) update(k, v);
    };
    inline virtual string toString() const { return ""; }
    inline friend std::ostream &operator<<(std::ostream &ss, const AlgConfig &a) {
        ss << a.toString();
        return ss;
    }
};


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

    virtual ~GamePlayingAlgorithm() = default;

    /**
     * Run one step of the algorithm and improve play distribution in current infoset.
     *
     * Infoset in which this algorithm should currently play is provided.
     * It will always be only infosets of the previously specified playing player.
     * If currentInfoset is nullopt, it means root iterations (preplay) should be done.
     *
     * @return how the game flow should operatore, see PlayControl
     */
    virtual PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) = 0;

    /**
     * Return probability distribution by which the next action should be selected.
     * They must sum up to 1. If the distribution is null, it means algorithm failed to find
     * any valid distribution and the consequence is that it gives up the game.
     */
    virtual optional<ProbDistribution>
    getPlayDistribution(const shared_ptr<AOH> &currentInfoset) = 0;
};


enum BudgetType { BudgetTime, BudgetIterations };

#define PLAY_FROM_ROOT nullopt

/**
 * Run iterations of given algorithm for a given budget value and type.
 *
 * See playForIterations / playForMicroseconds
 *
 * @return whether algorithm decided to continue (true) or give up (false)
 */
bool playForBudget(GamePlayingAlgorithm &alg,
                   const optional<shared_ptr<AOH>> &currentInfoset,
                   unsigned int budgetValue, BudgetType type);

/**
 * Run iterations of given algorithm for a given number of iterations.
 * @return whether algorithm decided to continue (true) or give up (false)
 */
bool playForIterations(GamePlayingAlgorithm &alg,
                       const optional<shared_ptr<AOH>> &currentInfoset,
                       unsigned int budgetIters);
/**
 * Run iterations of given algorithm for a given time budget in microseconds.
 * @return whether algorithm decided to continue (true) or give up (false)
 */
bool playForMilliseconds(GamePlayingAlgorithm &alg,
                         const optional<shared_ptr<AOH>> &currentInfoset,
                         unsigned int budgetMs);

/**
 * Random player gives up right away, and the rest of the match is played uniformly randomly.
 */
class RandomPlayer: public GamePlayingAlgorithm {
 public:
    inline RandomPlayer(const Domain &domain, Player playingPlayer)
        : GamePlayingAlgorithm(domain, playingPlayer) {}
    inline PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset)
    override { return StopImproving; };
    inline optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset)
    override { return nullopt; };
};

/**
 * Player that always chooses fixed action specified by it's index (action id).
 * You can use modulo algebra to specify the action, even with negative integers.
 */
class FixedActionPlayer: public GamePlayingAlgorithm {
 public:
    explicit FixedActionPlayer(const Domain &domain, Player playingPlayer, int actionIdx);
    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;
 private:
    InfosetCache cache_;
    const int actionIdx_;
};

typedef function<unique_ptr<GamePlayingAlgorithm>(const Domain &, Player)>
    PreparedAlgorithm;

template<typename T, typename... Args>
PreparedAlgorithm createInitializer(Args &... args) {
    return [&](const Domain &domain, Player pl) mutable -> unique_ptr<GamePlayingAlgorithm> {
        return make_unique<T>(domain, pl, args...);
    };
}

struct AlgorithmWithData {
    virtual PreparedAlgorithm prepare() = 0;
    virtual AlgConfig &config() = 0;
};

/**
 * Play match between given algorithms, for a given preplay budget and another budget for each move.
 * Return terminal utilities for each algorithm.
 *
 * You can prepare the algorithm by calling
 * @code
 * DomainInitilizer preparedAlg = createInitializer<MyAlg>( my_alg_params );
 * @endcode
 */
vector<double> playMatch(const Domain &domain,
                         vector<PreparedAlgorithm> algorithmInitializers,
                         vector<unsigned int> preplayBudget,
                         vector<unsigned int> moveBudget,
                         BudgetType simulationType,
                         unsigned long matchSeed);

}  // namespace GTLib2


#endif //GTLIB2_GAMEPLAYINGALGORITHM_H
