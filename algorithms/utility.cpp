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

#include "algorithms/utility.h"

#include "algorithms/strategy.h"

namespace GTLib2::algorithms {

vector<double> computeUtilitiesTwoPlayerGame(const Domain &domain,
                                             const StrategyProfile &stratProfile) {

    function<vector<double>(shared_ptr<EFGNode>)> calculate = [&](shared_ptr<EFGNode> node) {
        switch (node->type_) {
            case ChanceNode: {
                vector<double> utils = {0.0, 0.0};
                const auto chanceProbs = node->chanceProbs();
                for (const auto &action : node->availableActions()) {
                    const auto newNode = node->performAction(action);
                    const auto childUtils = calculate(newNode);
                    utils[0] += chanceProbs[action->getId()] * childUtils[0];
                    utils[1] += chanceProbs[action->getId()] * childUtils[1];
                }
                return utils;
            }

            case PlayerNode: {
                auto utils = vector<double>{0.0, 0.0};
                for (const auto &action : node->availableActions()) {
                    const auto infoset = node->getAOHInfSet();
                    auto dist = stratProfile[node->getPlayer()].at(infoset);
                    double prob = getActionProb(dist, action);
                    if (prob <= 0) continue;

                    const auto childNode = node->performAction(action);
                    const auto childUtils = calculate(childNode);
                    utils[0] += prob * childUtils[0];
                    utils[1] += prob * childUtils[1];
                }
                return utils;
            }

            case TerminalNode:
                return node->getUtilities();

            default:
                unreachable("unrecognized option!");
        }
    };

    return calculate(createRootEFGNode(domain));
}

unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>>
createInfosetsAndActions(const Domain &domain, const Player player) {
    unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>> infosetsActions;

    function<void(shared_ptr<EFGNode>)> extract = [&](shared_ptr<EFGNode> node) {
        if (node->type_ != PlayerNode) return;
        if (node->getPlayer() != player) return;

        const auto infoset = node->getAOHInfSet();
        if (infosetsActions.find(infoset) != infosetsActions.end()) return;

        infosetsActions[infoset] = node->availableActions();
    };

    treeWalk(domain, extract);
    return infosetsActions;
}

vector<BehavioralStrategy> generateAllPureStrategies(
    const unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>> &infosetsActions) {

    vector<BehavioralStrategy> pureStrats;
    const vector<pair<shared_ptr<AOH>, vector<shared_ptr<Action>>>> infosetsActionsPairs(
        infosetsActions.begin(), infosetsActions.end());

    function<void(BehavioralStrategy, int)> generate = [&](BehavioralStrategy strat, int setIndex) {
        if (setIndex >= infosetsActionsPairs.size()) {
            pureStrats.push_back(strat);
            return;
        }

        const auto infosetWithActions = infosetsActionsPairs[setIndex];
        const auto actions = infosetWithActions.second;
        for (const auto &action : actions) {
            const ActionProbDistribution dist = {{action, 1.0}};
            const auto infoset = infosetWithActions.first;
            strat[infoset] = dist;

            generate(strat, setIndex + 1);
        }
    };

    auto s = BehavioralStrategy();
    generate(s, 0);
    return pureStrats;
}

UtilityMatrix constructUtilityMatrixFor(
    const Domain &domain, const Player player,
    const array<vector<BehavioralStrategy>, 2> &playersPureStrats) {

    const auto rows = playersPureStrats[0].size();
    const auto cols = playersPureStrats[1].size();
    vector<double> matrix(rows * cols, 0.0);

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            auto utils = computeUtilitiesTwoPlayerGame(
                domain, {playersPureStrats[0][row], playersPureStrats[1][col]});
            matrix[cols * row + col] = utils[player];
        }
    }

    return UtilityMatrix{.u = matrix, .rows = rows, .cols = cols};
}

}  // namespace GTLib2
#pragma clang diagnostic pop
