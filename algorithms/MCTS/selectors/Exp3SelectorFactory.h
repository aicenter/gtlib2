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

#ifndef GTLIB2_EXP3SELECTORFACTORY_H
#define GTLIB2_EXP3SELECTORFACTORY_H

#include "algorithms/MCTS/ISMCTS_settings.h"
#include "algorithms/MCTS/selectors/selectorFactory.h"

namespace GTLib2::algorithms {

struct EXP3_ISMCTSSettings;

class Exp3SelectorFactory: public SelectorFactory {
 public:
    const EXP3_ISMCTSSettings &cfg_;
    explicit Exp3SelectorFactory(const EXP3_ISMCTSSettings &cfg);
    unique_ptr<Selector> createSelector(int actionsNumber) const override;
    unique_ptr<Selector> createSelector(vector<shared_ptr<Action>> actions) const override;
    std::mt19937 getRandom() const override;

    double normalizeValue(double value) const;
 private:
    std::mt19937 generator_;
};


struct EXP3_ISMCTSSettings: public ISMCTSSettings {
    // domain specific values
    const double minUtility_ = -1;
    const double maxUtility_ = 1;

    enum EXP3_Type { Exp3, Exp3L };
    EXP3_Type type = Exp3;
    double gamma = 0.05;
    bool storeExploration = false; //not used in Exp3L

    EXP3_ISMCTSSettings(const double minUtility, const double maxUtility)
        : minUtility_(minUtility), maxUtility_(maxUtility) {}

    unique_ptr<SelectorFactory> createFactory() const override {
        return make_unique<Exp3SelectorFactory>(*this);
    }

    //@formatter:off
    inline void update(const string &k, const string &v) override {
        if(k == "gamma")                            gamma             = std::stod(v); else
        if(k == "type" && v == "Exp3")              type              = Exp3;         else
        if(k == "type" && v == "Exp3L")             type              = Exp3L;        else
        if(k == "storeExploration" && v == "true")  storeExploration  = true;         else
        if(k == "storeExploration" && v == "false") storeExploration  = false;        else
        ISMCTSSettings::update(k,v);
    };
    inline string toString() const override {
        std::stringstream ss;
        ss << "; Exp3 selector" << endl;
        ss << "gamma     = " << gamma << endl;
        if(type == Exp3)  ss << "type      = Exp3"  << endl;
        if(type == Exp3L) ss << "type      = Exp3L" << endl;
        ss << "storeExploration = " << storeExploration << endl;
        ss << ISMCTSSettings::toString();
        return ss.str();
    }
    //@formatter:on
};

}

#endif //GTLIB2_EXP3SELECTORFACTORY_H
