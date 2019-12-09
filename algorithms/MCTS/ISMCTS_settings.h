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


#ifndef GTLIB2_ISMCTS_SETTINGS_H
#define GTLIB2_ISMCTS_SETTINGS_H

#include "base/algorithm.h"
#include "selectors/selectorFactory.h"

namespace GTLib2::algorithms {

struct ISMCTSSettings: public AlgConfig {
    /**
     * IMPORTANT NOTES
     *
     * useBelief DOES NOT AFFECT usual ISMCTS algorithm, only CPW_ISMCTS, as it requires IS -> nodes map.
     *
     * When useBelief = true, a node to be iterated is selected according to the probability distribution.
     *
     * Probability of the node counted as a multiplication of all probabilities,
     * that lead from the previous infoset to the current node.
     * If there are multiple ways from the old infoset to the node, they are summed.
     * In the end, probabilities for all nodes are normalized (see setCurrentInfoset and fillBelief at CPW_ISMCTS).
     */
    bool useBelief = false;

    int seed = 0;

    /**
     * Creates the factory that should make the selectors
     */
    virtual unique_ptr<SelectorFactory> createFactory() const = 0;

    //@formatter:off
    inline void update(const string &k, const string &v) override {
        if(k == "useBelief" && v == "true")   useBelief = true;  else
        if(k == "useBelief" && v == "false")  useBelief = false; else
        if(k == "seed") seed = std::stoi(v); else
        AlgConfig::update(k,v);
    };
    inline string toString() const override {
        std::stringstream ss;
        ss << "; ISMCTS" << endl;
        ss << "useBelief = " << useBelief  << endl;
        ss << "seed      = " << seed << endl;
        return ss.str();
    }
    //@formatter:on
};

}

#endif //GTLIB2_ISMCTS_SETTINGS_H
