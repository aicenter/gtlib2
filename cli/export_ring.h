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

#ifndef GTLIB2_EXPORT_RING_H
#define GTLIB2_EXPORT_RING_H

#include "cli_helpers.h"
#include "base/tree.h"
#include "base/fogefg.h"
#include "global_args.h"

namespace GTLib2::CLI {

const auto cmap_parula = std::vector<std::string>{
    "#352a87", "#362d8d", "#363296", "#36359c", "#3639a6", "#353dac", "#3341b6", "#2f47bf",
    "#2c4ac6", "#2450d0", "#1d54d5", "#105bdd", "#0760e0", "#0364e1", "#0268e1", "#036ae1",
    "#056de0", "#076fdf", "#0a72de", "#0d75dc", "#0f77db", "#117ad9", "#127cd8", "#137fd7",
    "#1482d5", "#1484d4", "#1487d3", "#1389d3", "#118dd2", "#0f8fd2", "#0c93d2", "#0a97d1",
    "#0899d1", "#079ccf", "#069ece", "#06a1cc", "#06a4ca", "#06a5c8", "#06a7c5", "#07a9c3",
    "#08abc0", "#0bacbd", "#0daeba", "#12afb7", "#15b0b4", "#1ab2b1", "#1eb3ae", "#24b5aa",
    "#2bb6a6", "#2fb7a3", "#36b99f", "#3bba9c", "#43bb97", "#4bbc93", "#51bd90", "#5abd8b",
    "#60be88", "#69be84", "#6ebf81", "#77bf7e", "#7fbf7a", "#85bf78", "#8dbf75", "#92bf73",
    "#99bf70", "#a0be6d", "#a5be6b", "#acbe68", "#b0bd66", "#b7bd64", "#bdbc61", "#c2bc5f",
    "#c8bc5d", "#ccbb5b", "#d2bb59", "#d6ba57", "#dcba54", "#e2b951", "#e6b94f", "#ecb94c",
    "#f0b94a", "#f6ba46", "#fabc41", "#fdbe3e", "#fec13a", "#ffc437", "#fec833", "#fdca31",
    "#fcce2d", "#fad22a", "#f9d528", "#f7d925", "#f6db23", "#f5e020", "#f5e41d", "#f5e81a",
    "#f5ed17", "#f6f114", "#f8f710", "#f9fb0e"
};

void Command_ExportRing(args::Subparser &parser) {
    initializeParser(parser); // always include this line in command

    unique_ptr<Domain> d = constructDomain(args::get(args::domain));

    vector<double> utils;
    auto cb = [&](const shared_ptr<FOG2EFGNode> &node) {
        if (node->type_ == TerminalNode) {
            utils.push_back(node->getUtilities()[0]);
        }
    };
    treeWalk(*d, cb);


    cout <<
         R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>)" "\n"
         R"(<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">)" "\n"
         R"(<svg width="100%" height="100%" viewBox="0 0 42 42" class="donut" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">)" "\n"
         R"(<circle class="donut-hole" cx="21" cy="21" r="15.91549430918954" fill="#fff"></circle>)" "\n"
         R"(<circle class="donut-ring" cx="21" cy="21" r="15.91549430918954" fill="transparent" stroke="#d2d3d4" stroke-width="3"></circle>)" "\n";

    double umax = *std::max_element(utils.begin(), utils.end());
    double umin = *std::min_element(utils.begin(), utils.end());

    for (unsigned int i = 0; i < utils.size(); ++i) {
        const unsigned long
            coloridx = floor((utils[i] - umin) / (umax - umin) * (cmap_parula.size() - 1));
        const auto color = cmap_parula[coloridx];

        cout
            << R"(<circle class="donut-segment" cx="21" cy="21" r="15.91549430918954" fill="transparent" stroke-width="3" )";
        cout << "stroke=\"" << color << "\" ";
        cout << "stroke-dasharray=\"" << 100. / utils.size() << " " << 100. - (100. / utils.size())
             << "\" ";
        cout << "stroke-dashoffset=\"" << i * (100. / utils.size()) << "\" ";
        cout << "xlink:title=\"" << utils[i] << "\">";
        cout << "</circle>\n";
    }

    cout << "</svg>";

}

}
#endif // GTLIB2_EXPORT_RING_H
