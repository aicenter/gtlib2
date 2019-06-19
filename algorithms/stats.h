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

#include "base/efg.h"

namespace GTLib2::algorithms {

/**
 * Capture various domain statistics, for each player if possible.
 */
struct DomainStatistics {
    unsigned int max_EFGDepth = 0;
    unsigned int max_StateDepth = 0;

    uint64_t num_nodes = 0;
    uint64_t num_terminals = 0;
    uint64_t num_states = 0;

    vector<uint64_t> num_histories;
    vector<uint64_t> num_infosets;
    vector<uint64_t> num_sequences;

    friend std::ostream &
    operator<<(std::ostream &ss, DomainStatistics const &stats) {
        ss << endl;
        ss << "Depth of EFG:        \t" << stats.max_EFGDepth << endl;
        ss << "Depth of game state: \t" << stats.max_StateDepth << endl;
        ss << "EFG nodes:           \t" << stats.num_nodes << endl;
        ss << "Terminals:           \t" << stats.num_terminals << endl;
        ss << "States:              \t" << stats.num_states << endl;
        ss << "Histories:           \t" << stats.num_histories << endl;
        ss << "Infosets:            \t" << stats.num_infosets << endl;
        ss << "Sequences:           \t" << stats.num_sequences << endl;
        ss << endl;

        return ss;
    }

    friend bool operator==(const DomainStatistics &some, const DomainStatistics &other) {
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

/**
 * Count the number of nodes, infosets, public states
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
inline void printDomainStatistics(const Domain &domain) { printDomainStatistics(domain, cout); }

}  // namespace GTLib2

#endif  // ALGORITHMS_STATS_H_
