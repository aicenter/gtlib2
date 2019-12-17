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

#include "algorithms/common.h"
#include "base/fogefg.h"



namespace GTLib2::algorithms {

BehavioralStrategy mixedToBehavioralStrategy(const Domain &domain,
                                             const vector<BehavioralStrategy> &pureStrats,
                                             const ProbDistribution &distribution,
                                             Player player) {
    BehavioralStrategy behavStrat;

    auto updateBehavStrategy = [&behavStrat, &player](shared_ptr<EFGNode> node) {
        if (node->type_ == TerminalNode) return;
        if (node->getPlayer() != player) return;

        const auto infoSet = node->getAOHInfSet();
        if (behavStrat.find(infoSet) == behavStrat.end()) {
            behavStrat[infoSet] = ActionProbDistribution();
        }
    };

    treeWalk(domain, updateBehavStrategy);

    for (int i = 0; i < pureStrats.size(); ++i) {
        for (const auto &pureStrat : pureStrats[i]) {
            const auto infoset = pureStrat.first;
            const auto actions = pureStrat.second;

            for (auto &action : actions) {
                const shared_ptr<Action> actionPtr = action.first;
                assert(action.second == 1.0);
                if (behavStrat[infoset].find(actionPtr) == behavStrat[infoset].end()) {
                    behavStrat[infoset][actionPtr] = 0;
                }
                behavStrat[infoset][actionPtr] += distribution[i];
            }
        }
    }

    return behavStrat;
}

bool isAOCompatible(const vector<ActionObservationIds> &aoTarget,
                    const vector<ActionObservationIds> &aoCmp) {
    auto sizeTarget = aoTarget.size();
    auto sizeCmp = aoCmp.size();
    if (min(sizeTarget, sizeCmp) == 0) return true;

    static_assert(sizeof(ActionObservationIds) == 8);
    static_assert(sizeof(ObservationId) == 4);
    static_assert(NO_OBSERVATION > OBSERVATION_PLAYER_MOVE);

    const ObservationId obsCmp = aoCmp[aoCmp.size() - 1].observation;
    const ObservationId obsTgt = aoTarget[aoTarget.size() - 1].observation;

    size_t cmpBytes;
    if ((obsCmp >= OBSERVATION_PLAYER_MOVE && (obsCmp < NO_OBSERVATION))
        || (obsTgt >= OBSERVATION_PLAYER_MOVE && (obsTgt < NO_OBSERVATION))) {
        // make sure that player move observation has precedence over no observation,
        // unless it's both no observation
        cmpBytes = (min(sizeTarget, sizeCmp) - 1) * sizeof(ActionObservationIds);
    } else if (obsCmp == NO_OBSERVATION || obsTgt == NO_OBSERVATION) {
        cmpBytes = min(sizeTarget, sizeCmp) * sizeof(ActionObservationIds) - sizeof(ObservationId);
    } else {
        cmpBytes = min(sizeTarget, sizeCmp) * sizeof(ActionObservationIds);
    }

    return !memcmp(aoTarget.data(), aoCmp.data(), cmpBytes);
}

vector<shared_ptr<EFGNode>> getAllNodesInInfoset(const shared_ptr<AOH> &infoset,
                                                 const Domain &domain) {

    vector<shared_ptr<EFGNode>> nodes;
    const auto aoTarget = infoset->getAOids();
    const Player player = infoset->getPlayer();

    function<void(shared_ptr<EFGNode>)> traverse = [&](shared_ptr<EFGNode> node) {
        switch (node->type_) {
            case ChanceNode:
            case PlayerNode: {
                const auto aoNode = node->getAOids(player);
                assert(aoTarget.size() >= aoNode.size()); // we should not get below the infoset

                if (node->type_ == PlayerNode
                    && node->getPlayer() == player
                    && aoTarget.size() == aoNode.size()
                    && isAOCompatible(aoTarget, aoNode)) {
                    nodes.push_back(node);
                    return; // do not go below infoset
                }

                if (isAOCompatible(aoTarget, aoNode)) {
                    for (const auto &action : node->availableActions()) {
                        traverse(node->performAction(action));
                    }
                }
                return;
            }
            case TerminalNode:
                return;
            default:
                unreachable("unrecognized option!");
        }
    };

    const auto rootNode = createRootEFGNode(domain);
    assert(isAOCompatible(aoTarget, rootNode->getAOids(player)));
    traverse(rootNode);

    return nodes;
}
}  // namespace GTLib2

