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


#ifndef GTLIB2_CFR_REGRETS_H
#define GTLIB2_CFR_REGRETS_H

#include "utils/cli_helpers.h"
#include "algorithms/cfr.h"

namespace GTLib2::CLI {

void Command_CFRRegrets(args::Subparser &parser) {
    args::PositionalList<double> regrets(parser, "REGRETS",
                                         "initial regrets at the root", {0., 0.});
    initializeParser(parser); // always include this line in command

    unique_ptr<Domain> domain = constructDomain(args::get(args::domain));

    auto settings = CFRSettings();
    settings.cfrUpdating = algorithms::InfosetsUpdating;
    auto data = CFRData(*domain, settings.cfrUpdating);
    data.buildTree();

    auto rootNode = data.getRootNode();
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0]);
    auto childInfoset = childNode->getAOHInfSet();
    auto &rootData = data.infosetData.at(rootInfoset);
    auto &childData = data.infosetData.at(childInfoset);
    assert(rootData.regrets.size() == args::get(regrets).size());
    rootData.regrets = args::get(regrets);

    CFRAlgorithm cfr(*domain, Player(0), data, settings);

    cout << "reg0,reg1,sigma_curr0,sigma_avg0" << endl;
    for (int i = 0; i < 1000; ++i) {
        cfr.runIteration(Player(0));
        cfr.delayedApplyRegretUpdates();

        cfr.runIteration(Player(1));
        cfr.delayedApplyRegretUpdates();

        cout << rootData.regrets[0] << ","
             << rootData.regrets[1] << ","
             << algorithms::calcRMProbs(rootData.regrets)[0] << ","
             << algorithms::calcAvgProbs(rootData.avgStratAccumulator)[0] << endl;
    }
}

}

#endif //GTLIB2_CFR_REGRETS_H
