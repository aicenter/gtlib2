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


#include "gadget.h"

namespace GTLib2 {

double GadgetGame::computePubStateReach() {
    const auto &reachProbs = summary_.topmostHistoriesReachProbs;
    double pubStateReach = 0.0;
    switch (variant_) {
        case SAFE_RESOLVING:
            for (const array<double, 3> h : reachProbs)
                pubStateReach += h[resolvingPlayer_] * h[2];
            break;
        case UNSAFE_RESOLVING:
            for (const array<double, 3> h : reachProbs)
                pubStateReach += h[0] * h[1] * h[2];
            break;
        case MAX_MARGIN:
            assert(false);
        default:
            assert(false);
    }
    return pubStateReach;
}

vector<double> GadgetGame::computeTerminateCFVValues() {
    const auto numHistories = summary_.topmostHistories.size();
    auto augInfosetse = vector<shared_ptr<AOH>>();
    augInfosetse.reserve(numHistories);
    auto infosetValues = unordered_map<shared_ptr<AOH>, double>{}; // for player 0
    auto infosetReaches = unordered_map<shared_ptr<AOH>, double>{};
    const auto &reachProbs = summary_.topmostHistoriesReachProbs;

    for (int i = 0; i < numHistories; ++i) {
        const auto &h = summary_.topmostHistories.at(i);
        const shared_ptr<AOH> aoh = augInfosetse.emplace_back(h->getAOHAugInfSet(viewingPlayer_));
        const auto addReach = reachProbs[i][resolvingPlayer_] * reachProbs[i][2];
        const auto addValue = addReach * summary_.expectedValues[i];

        infosetValues[aoh] += addValue;
        infosetReaches[aoh] += addReach;
    }

    auto cfvValues = vector<double>();
    cfvValues.reserve(numHistories);
    for (int i = 0; i < numHistories; ++i) {
        // the pubStateReach_ term must be added by the resolver when accessing terminal node!
        cfvValues.emplace_back(
            infosetValues.at(augInfosetse.at(i)) / infosetReaches.at(augInfosetse.at(i)));
    }

    return cfvValues; // again, defined for player 0
}
double GadgetGame::chanceProbForAction(const ActionId &action) const {
    const auto reach = summary_.topmostHistoriesReachProbs.at(action);

    switch (variant_) {
        case SAFE_RESOLVING:
            return reach[resolvingPlayer_] * reach[2] / pubStateReach_;
        case UNSAFE_RESOLVING:
            return reach[0] * reach[1] * reach[2] / pubStateReach_;
        case MAX_MARGIN: // todo:
            assert(false);
        default:
            unreachable("unrecognized option!");
    }
}

double GadgetRootNode::chanceProbForAction(const ActionId &action) const {
    return gadget_.chanceProbForAction(action);
}

shared_ptr<EFGNode> GadgetRootNode::performAction(const shared_ptr<Action> &action) const {
    const auto underlyingNode = gadget_.summary_.topmostHistories.at(action->getId());

    switch (gadget_.variant_) {
        case SAFE_RESOLVING:
            return make_shared<GadgetInnerNode>(
                gadget_, shared_from_this(), underlyingNode,
                underlyingNode->getAOids(gadget_.viewingPlayer_), action->getId());
        case UNSAFE_RESOLVING:
            return underlyingNode;
        case MAX_MARGIN:
            assert(false); // todo:
        default:
            unreachable("unrecognized option!");
    }
}

shared_ptr<EFGNode> GadgetInnerNode::performAction(const shared_ptr<Action> &action) const {
    switch (action->getId()) {
        case GADGET_FOLLOW:
            return underlyingNode_;
        case GADGET_TERMINATE: {
            double v = game_.cfvValues_.at(getLastActionId());
            return make_shared<GadgetTerminalNode>(shared_from_this(), vector<double>{v, -v});
        }
        default:
            unreachable("unrecognized option!");
    }
}

}