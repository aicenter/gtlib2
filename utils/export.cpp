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
#include "base/cache.h"
#include "base/tree.h"
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
            unreachable("unrecognized option!");
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
            unreachable("unrecognized option!");
    }
}

void exportGraphViz(const PublicStateCache &cache, std::ostream &fs) {
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

    std::function<void(shared_ptr<PublicState>)> walkPrint = [&](shared_ptr<PublicState> node) {
        string color = "#FFFFFF";
        string shape = "circle";

        string label = node->getDepth() > 0 ? to_string(node->getHistory().back()) : "∅";

        if (psIsTerminal(cache, node)) {
            // Print nodes
            fs << "\t\"" << node->toString() << "\" "
               << "[fillcolor=\"" << color << "\""
               << ",label=\"" << label << "\""
               << ",shape=\"" << shape << "\"]\n";
            return;
        }

        // Print node
        fs << "\t\"" << node->toString() << "\" "
           << "[fillcolor=\"" << color << "\""
           << ",label=\"" << label << "\""
           << ",shape=\"" << shape << "\"]\n";

        for (unsigned int i = 0; i < cntPsChildren(cache, node); ++i) {
            auto child = expandPs(cache, node, i);

            // Print edges
            fs << "\t\"" << node->toString() << "\""
               << " -> "
               << "\"" << child->toString() << "\""
               << " [label=\"" << i << "\"]\n";

            walkPrint(child);
        }
    };
    walkPrint(cache.getRootPublicState());

    fs << "}\n";
}

void exportGraphViz(const PublicStateCache &cache, const string &fileToSave) {
    ofstream fs(fileToSave);
    if (!fs.is_open()) {
        LOG_ERROR("Could not open " << fileToSave << " for writing.")
        return;
    }
    exportGraphViz(cache, fs);

    fs.close();
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

    auto walkPrint = [&fs](shared_ptr<FOG2EFGNode> node) {
        const string color = getColor(node);
        const string shape = getShape(node);
        const auto tooltip = (node->type_ == ChanceNode && !node->parent_)
                             ? ""
                             : node->getState()->toString();

        if (node->type_ == TerminalNode) {
            // Print nodes
            fs << "\t\"" << node->toString() << "\" "
               << "[fillcolor=\"" << color << "\""
               << ",label=\"" << node->getUtilities() << "\""
               << ",tooltip=\"" << Escaped{tooltip} << "\""
               << ",shape=\"" << shape << "\"]\n";
            return;
        }

        // Print node
        fs << "\t\"" << node->toString() << "\" "
           << "[fillcolor=\"" << color << "\""
           << ",label=\"" << node->toString() << "\""
           << ",tooltip=\"" << Escaped{tooltip} << "\""
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
    treeWalk(domain, walkPrint);

    fs << "}\n";
}

void exportGraphViz(const Domain &domain, const string &fileToSave) {
    ofstream fs(fileToSave);
    if (!fs.is_open()) {
        LOG_ERROR("Could not open " << fileToSave << " for writing.")
        return;
    }
    exportGraphViz(domain, fs);

    fs.close();
}

void exportGambit(const shared_ptr<EFGNode> &rootNode, std::ostream &fs) {
    // Print header
    fs << "EFG 2 R \"" << R"(" { "Pl0" "Pl1" })" << "\n";

    int terminalIdx = 0, chanceIdx = 0, infosetIdx = 0, pubStateIdx = 0;
    unordered_map<shared_ptr<AOH>, int> infoset2id;
    unordered_map<shared_ptr<PublicState>, int> pubstate2id;

    auto walkPrint = [&](shared_ptr<EFGNode> node) {
// Useful for debugging, but in normal view it clutters the graph a lot:
//        std::stringstream ss;
//        ss << "Pl0: " << node->getAOids(Player(0)) << " - ";
//        ss << "Pl1: " << node->getAOids(Player(1));
//        string nodeLabel = ss.str();

//        string nodeLabel = node->toString();
        string nodeLabel = "";
        for (unsigned int j = 0; j < node->efgDepth(); ++j) fs << " ";

        switch (node->type_) {
            case ChanceNode: {
                for (unsigned int j = 0; j < node->efgDepth(); ++j) fs << " ";
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

                // May be undefined for some nodes (like in gadget)
                int psId = -1;
                if (node->getSpecialization() == NoSpecialization) {
                    auto pubstate = node->getPublicState();
                    if (pubstate2id.find(pubstate) == pubstate2id.end()) {
                        pubstate2id.emplace(make_pair(pubstate, ++pubStateIdx));
                        psId = pubStateIdx;
                    } else {
                        psId = pubstate2id.find(pubstate)->second;
                    }
                }

                auto actions = node->availableActions();
                fs << "p \"" << nodeLabel << "\" "
                   << (int(node->getPlayer()) + 1) << " " << isId << " \"" << psId << "\" { ";
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
                unreachable("unrecognized option!");
        }
    };

    treeWalk<EFGNode>(rootNode, walkPrint);
}

void exportGambit(const Domain &domain, std::ostream &fs) {
    exportGambit(createRootEFGNode(domain), fs);
}

void exportGambit(const Domain &domain, const string &fileToSave) {
    ofstream fs(fileToSave);
    if (!fs.is_open()) {
        LOG_ERROR("Could not open " << fileToSave << " for writing.")
        return;
    }
    exportGambit(domain, fs);

    fs.close();
}

void exportGambit(const shared_ptr<EFGNode> &rootNode, const string &fileToSave) {
    ofstream fs(fileToSave);
    if (!fs.is_open()) {
        LOG_ERROR("Could not open " << fileToSave << " for writing.")
        return;
    }
    exportGambit(rootNode, fs);

    fs.close();
}

}
