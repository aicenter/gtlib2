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

#include "gambit.h"

#include <fstream>

namespace GTLib2::domains::gambit {

unique_ptr<GambitDomain> loadFromFile(const string &file) {
    // open file for reading
    std::ifstream in(file);
    if (in.fail()) {
        cerr << "Cannot open " + file + " for reading" << endl;
        exit(1);
    }
    auto d = make_unique<GambitDomain>(in);
    in.close();

    return d;
}

GambitDomain::GambitDomain(std::istream &in)
    : Domain(1000, 2, true, make_shared<Action>(), make_shared<Observation>()) {

    // read header
    std::string header;
    std::getline(in, header);
    if (in.fail()) {
        cerr << "Cannot read line 1 in input stream" << endl;
        exit(1);
    }
    // now all the nodes should be listed
    std::string line;
    std::getline(in, line);
    int line_num = 1;
    root_ = move(parseSubtree(in, line, line_num));

    rootStatesDistribution_ = createOutcomes(root_.get());
}

std::unique_ptr<Node>
GambitDomain::parseSubtree(std::istream &in, const std::string &line, int &line_num) {
    line_num++;
    auto node = parseNodeLine(line, line_num);

    // create the node
    for (int i = 0; i < node->num_actions; ++i) {
        std::string next_line;
        std::getline(in, next_line);
        if (in.fail()) {
            cerr << "Cannot read line " + std::to_string(line_num + 1) << "\n";
            exit(1);
        }
        if (in.eof()) {
            cerr << "Premature end of file on line " + std::to_string(line_num + 1) << "\n";
            exit(1);
        }
        node->children.emplace_back(move(parseSubtree(in, next_line, line_num)));
    }

    // update game stats
    if (node->node_type == 't')
        maxUtility_ = std::max(maxUtility_,
                               *max_element(node->utils.begin(),
                                            node->utils.end()));

    return node;
}

std::unique_ptr<Node> parseNodeLine(const std::string &line, const int &line_num) {
    // A node line is something like:
    // c "∅ " 0 "" { "0" 0.333 "1" 0.333 "2" 0.333 } 0
    // p "∅ 0 1" 1 1 "" { "0" "1" } 0
    // t "∅ 0 1 1 0" 3 "" { 1.0, -1.0 } 0
    constexpr int OPEN = 0;
    constexpr int NODE_TYPE = 1;
    constexpr int NODE_LABEL_OPEN = 2;
    constexpr int NODE_LABEL_CLOSE = 3;
    constexpr int INFOSET_IDX = 4;
    constexpr int PUBSTATE_LABEL = 5;
    constexpr int PUBSTATE_LABEL_CLOSE = 6;
    constexpr int OUTCOMES_OPEN = 7;
    int state = OPEN;
    unsigned int num_action_quotes = 0;
    std::string buf;

    // what we want to parse out:
    char node_type = 'c'; // invalid node type
    int player = 2;
    unsigned long infoset_idx = 0;
    unsigned long pubstate_idx = 0;
    int infoset_len = 0;
    int pubstate_len = 0;
    unsigned int num_actions = 0;
    std::vector<double> utils;
    std::vector<double> probs;

    for (char c : line) {
        if (c == ' ') continue;

        switch (state) {
            case OPEN:
                if (c == '#') { // comment
                    return nullptr;
                }

                if (!(c == 'c' || c == 'p' || c == 't')) {
                    cerr << "Node type can be either c/p/t "
                            "at line " + std::to_string(line_num) + ". Line is: " + line << endl;
                    exit(1);
                }
                node_type = c;
                state = NODE_TYPE;
                break;
            case NODE_TYPE:
                assert(c == '"');
                state = NODE_LABEL_OPEN;
                break;
            case NODE_LABEL_OPEN:
                if (c == '"') state = NODE_LABEL_CLOSE;
                break;
            case NODE_LABEL_CLOSE:
                if (node_type == 'c' || node_type == 't') {
                    if (c == '{') state = OUTCOMES_OPEN;
                } else if (node_type == 'p') {
                    // must be player id now, they start with 1, we want it to be 0-indexed
                    // only one digit
                    player = c - '1';
                    state = INFOSET_IDX;
                }
                break;

            case INFOSET_IDX:
                assert(node_type == 'p');
                // now infoset id comes, can be multiple digits
                if (c != '"') {
                    infoset_idx = infoset_idx * 10 + (c - '0');
                    infoset_len++;
                    if (infoset_len > 20) { // 18446744073709551615
                        cerr << "Infoset identifier is too lengthy for unsigned long: "
                                "at line " + std::to_string(line_num) + ". Line is: " + line;
                        exit(1);
                    }
                } else state = PUBSTATE_LABEL;
                break;

            case PUBSTATE_LABEL:
                if (c != '"') {
                    pubstate_idx = pubstate_idx * 10 + (c - '0');
                    pubstate_len++;
                    if (pubstate_len > 20) { // 18446744073709551615
                        cerr << "Pubstate identifier is too lengthy for unsigned long: "
                                "at line " + std::to_string(line_num) + ". Line is: " + line;
                        exit(1);
                    }
                } else state = PUBSTATE_LABEL_CLOSE;
                break;

            case PUBSTATE_LABEL_CLOSE:
                if (c == '{') state = OUTCOMES_OPEN;
                break;

            case OUTCOMES_OPEN:
                if (node_type == 'c' || node_type == 'p') {
                    // count the number of occurences of " -- div/2 is number of actions
                    if (c == '"') {
                        num_action_quotes++;
                    }
                }

                if (node_type == 'c') {
                    if (num_action_quotes % 2 == 0 && c != '"') { // not a node label, i.e. a prob
                        buf += c; // collect the prob into a buffer
                    } else if (!buf.empty()) {
                        probs.push_back(std::stod(buf));
                        buf = "";
                    }
                } else if (node_type == 't') {
                    if (c != ',') { // until we get another util
                        buf += c; // collect the prob into a buffer
                    } else if (!buf.empty()) {
                        utils.push_back(std::stod(buf));
                        buf = "";
                    }
                }
                break;
        }
    }

    if (!buf.empty()) {
        if (node_type == 'c') probs.push_back(std::stod(buf));
        else if (node_type == 't') utils.push_back(std::stod(buf));
    }

    if (num_action_quotes % 2 != 0) {
        cerr << "Mismatched number of quotes in actions "
                "at line " + std::to_string(line_num) + ". Line is: " + line << endl;
        exit(1);
    }

    num_actions = num_action_quotes / 2;
    if (node_type == 'c' && probs.size() != num_actions) {
        cerr << "Mismatched number of probabilities and actions "
                "at line " + std::to_string(line_num) + ". Line is: " + line << endl;
        exit(1);
    }

    // properly normalize probs (there can be an error due to string serialization)
    double psum = 0.;
    for (double p: probs) psum += p;
    for (double &p: probs) p /= psum;

    // update player for different node types
    if (node_type == 'c') player = 2;
    if (node_type == 't') player = 3;

    std::unique_ptr<Node> node = std::make_unique<Node>();
    node->node_type = node_type;
    node->player = player;
    node->infoset_idx = infoset_idx;
    node->pubstate_idx = pubstate_idx;
    node->num_actions = num_actions;
    node->utils = utils;
    node->probs = probs;
    node->description = ""; // todo:

    return node;
}


OutcomeDistribution GambitDomain::createOutcomes(Node *next) const {
    if (next->node_type == 'c') { // assumes no repeated chance nodes
        // I dont have time for special cases
        auto outcomes = OutcomeDistribution();
        for (unsigned int i = 0; i < next->probs.size(); ++i) {
            outcomes.emplace_back(OutcomeEntry(Outcome(
                make_shared<GambitState>(this, next->children[i].get()),
                createPrivateObs(next->children[i].get()),
                createPublicObs(next->children[i].get()),
                {0, 0}
            ), next->probs[i]));
        }
        return outcomes;
    }

    auto utils = vector<double>(2);
    if (next->node_type == 't') {
        utils = next->utils;
    }

    return {OutcomeEntry(Outcome(
        make_shared<GambitState>(this, next),
        createPrivateObs(next),
        createPublicObs(next),
        utils
    ))};
}

vector<shared_ptr<Observation>> GambitDomain::createPrivateObs(Node *next) const {
    auto obs = vector<shared_ptr<Observation>>{noObservation_, noObservation_};
    if (next->node_type == 't') {
        return obs;
    }
    obs[next->player] = make_shared<Observation>(next->infoset_idx);
    return obs;
}

shared_ptr<Observation> GambitDomain::createPublicObs(Node *next) const {
    if (next->node_type == 't') {
        return noObservation_;
    }
    return make_shared<Observation>(next->pubstate_idx);
}

OutcomeDistribution GambitState::performActions(const vector<shared_ptr<Action>> &actions) const {
    int actionId = actions.at(n_->player)->getId();
    Node *nextNode = n_->children.at(actionId).get();
    const auto g = static_cast<const GambitDomain *>(domain_);
    return g->createOutcomes(nextNode);
}

}
