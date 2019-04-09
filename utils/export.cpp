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

#include "export.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <algorithms/common.h>

#include "base/efg.h"
#include "base/hashing.h"
#include "algorithms/tree.h"

namespace GTLib2 {
namespace utils {

using std::setw;
using std::setfill;
using std::hex;
using std::ofstream;
using algorithms::treeWalkEFG;
using algorithms::createRootEFGNodes;


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

inline string getShape(Player player) {
    switch (player) {
        case 0:
            return "shape=triangle";
        case 1:
            return "shape=invtriangle";
    }
}

inline string getColor(Player player) {
    switch (player) {
        case 0:
            return "#FF0000";
        case 1:
            return "#00FF00";
    }
}



void exportGraphViz(const Domain &domain, ostream &fs) {
    // Print header
    fs << "digraph {" << std::endl;
    fs << "\trankdir=LR" << std::endl;
    fs << "\tgraph [fontname=courier]" << std::endl;
    fs << "\tnode  ["
          "fontname=courier, "
          "shape=box, "
          "style=\"filled\", "
          "fillcolor=white]"
       << std::endl;
    fs << "\tedge  [fontname=courier]" << std::endl;

// todo:
//    label = "The foo, the bar and the baz";
//    labelloc = "t"; // place the label at the top (b seems to be default)

    auto walkPrint = [&fs, &domain](shared_ptr<EFGNode> node) {
        if(node->isTerminal()) {
            // Print nodes
            fs << "\t\"" << node->getDescriptor() << "\" "
               << R"([fillcolor="#FFFFFF", label=")" << node->rewards_ <<"\"]\n";
            return;
        }

        for (auto &action : node->availableActions()) {
            auto children = node->performAction(action);

            // Print edges
            for (auto &[child, prob] : children) {
                fs << "\t\"" << node->getDescriptor() << "\""
                   << " -> "
                   << "\"" << child->getDescriptor() << "\""
                   << " [label=\"" << action->toString() << ", p=" << prob << "\"]\n";
            }

            // Print nodes
            string color = getColor(*node->getCurrentPlayer());
            string maybeShape = getShape(*node->getCurrentPlayer());

            fs << "\t\"" << node->getDescriptor() << "\" "
               << "[fillcolor=\"" << color << R"(", label="", )" << maybeShape << "]\n";
        }
    };

    treeWalkEFG(domain, walkPrint);

    fs << "}";
}

void exportGraphViz(const Domain &domain, const string &fileToSave) {
    ofstream fs(fileToSave);
    if (!fs.is_open()) {
        std::cerr << "Could not open " << fileToSave << " for writing.";
        return;
    }
    exportGraphViz(domain, fs);

    fs.close();
}


void exportGambit(const Domain &domain, ostream &fs) {
    // Print header
    const auto name = typeid(domain).name();
    fs << "EFG 2 R \"" << name << R"(" { "Pl0" "Pl1" })" << "\n";
    fs << "\"\"\n";

    int terminalIdx = 0, chanceIdx = 0, infosetIdx = 0;
    unordered_map<shared_ptr<AOH>, int> infoset2id;
    auto walkPrint = [&](shared_ptr<EFGNode> node) {
        auto nodeLabel = ""; //node->toString();
        for (int j = 0; j < node->getDistanceFromRoot(); ++j) {
            fs << " ";
        }

        if (node->isTerminal()) {
            fs << "t \"" << nodeLabel << "\" " << terminalIdx++ << " \"\" { ";
            fs << node->rewards_[0] << ", " << node->rewards_[1];
            fs << "}\n";
            return;
        }

        // inner node
        auto infoset = node->getAOHInfSet();
        int isId;
        if(infoset2id.find(infoset) == infoset2id.end()) {
            infoset2id.emplace(std::make_pair(infoset, ++infosetIdx));
            isId = infosetIdx;
        } else {
            isId = infoset2id.find(infoset)->second;
        }

        fs << "p \"" << nodeLabel << "\" "
           << (int(*node->getCurrentPlayer())+1) << " " << isId << " \"\" { ";
        for (const auto &action: node->availableActions()) {
            fs << "\"" << action->toString() << "\" ";
        }
        fs << "} 0\n";

        for (auto &action : node->availableActions()) {
            auto children = node->performAction(action);
            if(children.size() == 1) return;

            for (int j = 0; j < node->getDistanceFromRoot(); ++j) {
                fs << " ";
            }
            fs << "c \"" << nodeLabel << "\" " << chanceIdx++ << " \"\" { ";
            int i = 0;
            for (const auto &[childNode, chanceProb]: children) {
                fs << "\"" << i++ << "\" " << chanceProb << " ";
            }
            fs << "} 0\n";
        }
    };

    auto rootNodes = createRootEFGNodes(domain.getRootStatesDistribution());
    if(rootNodes.size() > 1) {
        fs << "c \"" << "" << "\" " << chanceIdx++ << " \"\" { ";
        int i = 0;
        for (const auto &[rootNode, chanceProb] : rootNodes) {
            fs << "\"" << i++ << "\" " << chanceProb << " ";
        }
        fs << "} 0\n";
    }

    treeWalkEFG(domain, walkPrint);
}

void exportGambit(const Domain &domain, const string &fileToSave) {
    ofstream fs(fileToSave);
    if (!fs.is_open()) {
        std::cerr << "Could not open " << fileToSave << " for writing.";
        return;
    }
    exportGambit(domain, fs);

    fs.close();
}

}
}
