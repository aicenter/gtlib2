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

#include "utils/export.h"

#include <iomanip>

#include "base/fogefg.h"
#include "algorithms/common.h"

namespace GTLib2::utils {

using std::setw;
using std::setfill;
using std::hex;
using std::ofstream;


struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    friend std::ostream &
    operator<<(std::ostream &ss, Color const &c) {
        ss << setw(2) << setfill('0') << hex << (int) c.r;
        ss << setw(2) << setfill('0') << hex << (int) c.g;
        ss << setw(2) << setfill('0') << hex << (int) c.b;
        return ss;
    }
};

inline Color operator*(double s, const Color &x) {
    return {uint8_t(s * x.r), uint8_t(s * x.g), uint8_t(s * x.b)};
}
inline Color operator+(const Color &x, const Color &y) {
    return {uint8_t(x.r + y.r), uint8_t(x.g + y.g), uint8_t(x.b + y.b)};
}

inline Color getColor(double alpha, const Color &c0, const Color &c1) {
    return (1 - alpha) * c0 + alpha * c1;
}

inline Color getColor(double v, double vmin, double vmax) {
    // clamp value within range
    v = v <= vmin ? vmin
                  : v >= vmax ? vmax
                              : v;

    const double alpha = (vmax <= vmin)
                         ? 0.5                   // avoid divide by zero
                         : (v - vmin) / (vmax - vmin);

    static const Color green{0, 255, 0};
    static const Color white{255, 255, 255};
    static const Color red{255, 0, 0};

    if (alpha < 0.5) {
        return getColor(alpha * 2, green, white);
    } else {
        return getColor(alpha * 2 - 1, white, red);
    }
}

inline string getShape(const shared_ptr<EFGNode> &node) {
    switch (node->type_) {
        case ChanceNode:
            return "circle";
        case PlayerNode:
            return array<string, 2>{"triangle", "invtriangle"}[node->getPlayer()];
        case TerminalNode:
            return "square";
        default:
            assert(false); // unrecognized option!
    }
}

inline string getColor(const shared_ptr<EFGNode> &node) {
    switch (node->type_) {
        case ChanceNode:
            return "#FFFFFF";
        case PlayerNode:
            return array<string, 2>{"#FF0000", "#00FF00"}[node->getPlayer()];
        case TerminalNode:
            return "#888888";
        default:
            assert(false); // unrecognized option!
    }
}


void exportGraphViz(const Domain &domain, std::ostream &fs) {
    // Print header
    fs << "digraph {" << endl;
    fs << "\trankdir=LR" << endl;
    fs << "\tgraph [fontname=courier]" << endl;
    fs << "\tnode  ["
          "fontname=courier, "
          "shape=box, "
          "style=\"filled\", "
          "fillcolor=white]"
       << endl;
    fs << "\tedge  [fontname=courier]" << endl;

// todo:
//    label = "The foo, the bar and the baz";
//    labelloc = "t"; // place the label at the top (b seems to be default)

    auto walkPrint = [&fs, &domain](shared_ptr<FOG2EFGNode> node) {
        string color = getColor(node);
        string shape = getShape(node);

        if (node->type_ == TerminalNode) {
            // Print nodes
            fs << "\t\"" << node->toString() << "\" "
               << "[fillcolor=\"" << color << "\""
               << ",label=\"" << node->getUtilities() << "\""
               << ",shape=\"" << shape << "\"]\n";
            return;
        }

        // Print node
        fs << "\t\"" << node->toString() << "\" "
           << "[fillcolor=\"" << color << "\""
           << ",label=\"" << node->toString() << "\""
           << ",shape=\"" << shape << "\"]\n";

        for (auto &action : node->availableActions()) {
            auto child = dynamic_pointer_cast<FOG2EFGNode>(node->performAction(action));

            // Print edges
            fs << "\t\"" << node->toString() << "\""
               << " -> "
               << "\"" << child->toString() << "\""
               << " [label=\"" << action->toString() << "\"]\n";
        }
    };
    const auto rootNode = dynamic_pointer_cast<FOG2EFGNode>(createRootEFGNode(domain));
    treeWalk<FOG2EFGNode>(rootNode, walkPrint);

    fs << "}\n";
}

void exportGraphViz(const Domain &domain, const string &fileToSave) {
    ofstream fs(fileToSave);
    if (!fs.is_open()) {
        cerr << "Could not open " << fileToSave << " for writing.";
        return;
    }
    exportGraphViz(domain, fs);

    fs.close();
}


void exportGambit(const Domain &domain, std::ostream &fs) {
    // Print header
    const auto name = typeid(domain).name();
    fs << "EFG 2 R \"" << name << R"(" { "Pl0" "Pl1" })" << "\n";
    fs << "\"\"\n";

    int terminalIdx = 0, chanceIdx = 0, infosetIdx = 0;
    unordered_map<shared_ptr<AOH>, int> infoset2id;

    auto walkPrint = [&](shared_ptr<EFGNode> node) {
// Useful for debugging, but in normal view it clutters the graph a lot:
//        std::stringstream ss;
//        ss << "Pl0: " << node->getAOids(Player(0)) << " - ";
//        ss << "Pl1: " << node->getAOids(Player(1));
//        string nodeLabel = ss.str();

//        string nodeLabel = node->toString();
        string nodeLabel = "";
        for (int j = 0; j < node->efgDepth(); ++j) fs << " ";

        switch (node->type_) {
            case ChanceNode: {
                for (int j = 0; j < node->efgDepth(); ++j) fs << " ";
                fs << "c \"" << nodeLabel << "\" " << chanceIdx++ << " \"\" { ";
                int i = 0;
                for (const auto &chanceProb : node->chanceProbs()) {
                    fs << "\"" << i++ << "\" " << chanceProb << " ";
                }
                fs << "} 0\n";
                return;
            }

            case PlayerNode: {
                auto infoset = node->getAOHInfSet();
                int isId;
                if (infoset2id.find(infoset) == infoset2id.end()) {
                    infoset2id.emplace(make_pair(infoset, ++infosetIdx));
                    isId = infosetIdx;
                } else {
                    isId = infoset2id.find(infoset)->second;
                }

                auto actions = node->availableActions();
                fs << "p \"" << nodeLabel << "\" "
                   << (int(node->getPlayer()) + 1) << " " << isId << " \"\" { ";
                for (const auto &action: actions) {
                    fs << "\"" << action->toString() << "\" ";
                }
                fs << "} 0\n";
                return;
            }

            case TerminalNode: {
                fs << "t \"" << nodeLabel << "\" " << terminalIdx++ << " \"\" { ";
                fs << node->getUtilities()[0] << ", " << node->getUtilities()[1];
                fs << "}\n";
                return;
            }

            default:
                assert(false); // unrecognized option!
        }
    };

    treeWalk(domain, walkPrint);
}

void exportGambit(const Domain &domain, const string &fileToSave) {
    ofstream fs(fileToSave);
    if (!fs.is_open()) {
        cerr << "Could not open " << fileToSave << " for writing.";
        return;
    }
    exportGambit(domain, fs);

    fs.close();
}

}
