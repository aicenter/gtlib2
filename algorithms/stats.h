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


#ifndef ALGORITHMS_STATS_H_
#define ALGORITHMS_STATS_H_

#include "base/base.h"
#include "base/efg.h"

namespace GTLib2 {
namespace algorithms {

#define PRINT_ARRAY(ostr, ARR) \
    ostr << "[";               \
    for (auto x : ARR) {       \
        ostr << x << ", ";     \
    }                          \
    ostr << "]\n";

/**
 * Capture various domain statistics, for each player if possible.
 */
struct DomainStatistics {
    int max_EFGDepth = 0;
    int max_StateDepth = 0;

    uint64_t num_nodes = 0;
    uint64_t num_terminals = 0;
    uint64_t num_states = 0;

    uint64_t num_histories[GAME_MAX_PLAYERS] = {0};
    uint64_t num_infosets[GAME_MAX_PLAYERS] = {0};
    uint64_t num_sequences[GAME_MAX_PLAYERS] = {0};

    friend std::ostream &
    operator<<(std::ostream &ss, DomainStatistics const &stats) {
        ss << std::endl;
        ss << "States:              \t" << stats.num_states << std::endl;
        ss << "Histories:           \t"; PRINT_ARRAY(ss, stats.num_histories);
        ss << "Terminals:           \t" << stats.num_terminals << std::endl;
        ss << "EFG nodes:           \t" << stats.num_nodes << std::endl;
        ss << "Infosets:            \t"; PRINT_ARRAY(ss, stats.num_infosets);
        ss << "Sequences:           \t"; PRINT_ARRAY(ss, stats.num_sequences);
        ss << "Depth of game state: \t" << stats.max_StateDepth << std::endl;
        ss << "Depth of EFG:        \t" << stats.max_EFGDepth << std::endl;
        ss << std::endl;

        return ss;
    }

    friend bool
    operator==(const DomainStatistics &some, const DomainStatistics &other) {
        if (!(some.max_EFGDepth == other.max_EFGDepth &&
            some.max_StateDepth == other.max_StateDepth &&
            some.num_nodes == other.num_nodes &&
            some.num_terminals == other.num_terminals &&
            some.num_states == other.num_states)) {
            return false;
        }

        if (!std::equal(std::begin(some.num_histories),
                        std::end(some.num_histories),
                        std::begin(other.num_histories)))
            return false;

        if (!std::equal(std::begin(some.num_sequences),
                        std::end(some.num_sequences),
                        std::begin(other.num_sequences)))
            return false;

        return std::equal(std::begin(some.num_infosets),
                          std::end(some.num_infosets),
                          std::begin(other.num_infosets));
    }
};

/** Count the number of nodes, infosets, public states
 * and other statistics defined in DomainStatistics
 * @see DomainStatistics
 */
void calculateDomainStatistics(const Domain &domain, DomainStatistics *stats);

/**
 * Print basic statistics about supplied domain to stream
 */
void printDomainStatistics(const Domain &domain, std::ostream &ostr);

/**
 * Print basic statistics about supplied domain
 */
inline void printDomainStatistics(const Domain &domain) {
    printDomainStatistics(domain, std::cout);
}

}  // namespace algorithms
}  // namespace GTLib2

#endif  // ALGORITHMS_STATS_H_
